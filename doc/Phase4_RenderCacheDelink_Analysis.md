# Phase 4 Render Cache Delink: Expected Gain vs Observed Delay

## Scope
This note explains:
- what improvement was expected from `DecodeRenderCache` in `TableModel` and `SearchTableModel`
- why delay is observed vs master for:
  - search word flow
  - filter apply flow (CFI)
  - file open flow (CFI during open)

## Expected Improvement (Design Intent)
The design intent of `DecodeRenderCache` was to reduce repeated work in repaint/scroll paths:
- Keep preformatted display values per row so repeated `data(Qt::DisplayRole)` calls avoid re-formatting time, payload, APID/CTID text, arguments.
- Reuse decoded message objects via decode cache so plugin decode is not repeated for the same message.
- Improve perceived smoothness during scrolling, table expose/repaint, and repeated viewport redraws.

Expected win area: repeated repaint of already-visible rows.

Expected non-win area: first-time render of a row, index/filter computation itself.

## Why Delay Is Seen vs Master

### 1) Search word flow appears slower
Search itself (message matching) is not the main regression; the visible delay is mostly in post-search UI rendering.

Primary causes:
- On first paint of a search result row, the cache builds a full row entry (`displayValues`) instead of only the requested cell.
- If many columns are enabled (`showArguments`), one first-hit request can trigger formatting for all columns of that row.
- `SearchTableModel::data()` performs decode-manager fetch for multiple roles (Display/Foreground/Background), adding repeated fetch + object copy overhead per row paint cycle.

Impact:
- Search completion feels delayed when result table is shown/updated, even if matcher runtime is unchanged.

Code snippets causing this slowdown:

1) Full-row precompute on first hit in search model (formats all columns, not only requested cell)

```cpp
// src/searchtablemodel.cpp
SearchTableModel::DecodeRenderCacheEntry SearchTableModel::buildDecodeRenderCacheEntry(unsigned long messageIndex, QDltMsg &msg) const
{
  DecodeRenderCacheEntry entry;
  entry.messageIndex = messageIndex;
  entry.generation = m_renderCacheGeneration;

  const int currentColumnCount = columnCount();
  entry.displayValues.reserve(currentColumnCount);
  for (int column = 0; column < currentColumnCount; ++column)
  {
    entry.displayValues.push_back(buildDisplayValue(column, messageIndex, msg));
  }

  return entry;
}
```

Why this is slow:
- On a cache miss, this loop computes every visible column value for the row, even if the view requested only one column.
- Runtime grows with `columnCount()`; enabling argument columns (`showArguments`) makes first-hit cost significantly higher.

What this code does:
- Creates one cache entry per visible row.
- Stores generation and message identity for validity checks.
- Fills `displayValues` eagerly for all columns by repeatedly calling `buildDisplayValue(...)`.

2) Cache miss path in DisplayRole always materializes entire row entry

```cpp
// src/searchtablemodel.cpp (inside SearchTableModel::data)
if (role == Qt::DisplayRole)
{
  unsigned long messageIndex = m_searchResultList.at(index.row());
  if(!qfile->getMsg(messageIndex, msg)) { /* ... */ }

  if(QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool())
    pluginManager->decodeMsg(msg,!QDltOptManager::getInstance()->issilentMode());

  DecodeRenderCacheEntry entry;
  const bool hasRenderCacheEntry = m_decodeRenderCache.exists(index.row());
  if (hasRenderCacheEntry)
  {
    entry = m_decodeRenderCache.get(index.row());
  }

  const bool isRenderCacheValid = hasRenderCacheEntry
                  && entry.messageIndex == messageIndex
                  && entry.generation == m_renderCacheGeneration
                  && entry.displayValues.size() == columnCount();

  if (!isRenderCacheValid)
  {
    entry = buildDecodeRenderCacheEntry(messageIndex, msg);
    m_decodeRenderCache.put(index.row(), entry);
  }

  return entry.displayValues.at(index.column());
}
```

Why this is slow:
- `qfile->getMsg(...)` + `pluginManager->decodeMsg(...)` happen before cache validation completes, so cold requests pay decode + format costs immediately.
- If cache is invalid/missing, it triggers full-row precompute via `buildDecodeRenderCacheEntry(...)`.

What this code does:
- Handles display text retrieval for one cell.
- Loads and decodes the message for the row.
- Checks if row cache is valid for `(row -> messageIndex, generation, column count)`.
- Rebuilds and stores full row display cache when invalid.
- Returns only `entry.displayValues.at(index.column())` to the view.

3) Additional per-role message fetches in same paint cycle (Foreground/Background)

```cpp
// src/searchtablemodel.cpp (inside SearchTableModel::data)
if ( role == Qt::ForegroundRole )
{
  if(qfile->getMsg(m_searchResultList.at(index.row()), msg))
  {
    return QVariant(QBrush(DltUiUtils::optimalTextColor(getMsgBackgroundColor(msg))));
  }
  /* ... */
}

if ( role == Qt::BackgroundRole )
{
  if(qfile->getMsg(m_searchResultList.at(index.row()), msg))
  {
    return QVariant(QBrush(getMsgBackgroundColor(msg)));
  }
  /* ... */
}
```

Why this is slow:
- Qt can ask `DisplayRole`, `ForegroundRole`, and `BackgroundRole` for the same row during one paint cycle.
- Foreground/Background paths call `qfile->getMsg(...)` again, adding repeated per-row fetch overhead.

What this code does:
- For text color, reloads message and computes contrast color from computed background.
- For background color, reloads message and computes color classification again.
- If message cannot be loaded, falls back to theme-dependent defaults.

### 2) Filter apply (CFI) feels slower
CFI indexing/filtering cost itself is mostly unchanged, but the completion path now includes heavier cold-cache repaint.

Primary causes:
- After filter apply, model state changes invalidate render caches (generation bump).
- First viewport paint after CFI is cold and reconstructs row display caches.
- Row cache generation is correct functionally, but first paint cost is paid synchronously on UI thread.

Impact:
- User perceives CFI as slower because “filter complete + first visible UI refresh” takes longer than master.

Code snippets causing this slowdown:

1) Generation bump invalidates prior render-cache entries after model transition

```cpp
// src/tablemodel.cpp
void TableModel::modelChanged()
{
  /* ... index update logic ... */
  lastSearchIndex = -1;
  ++m_renderCacheGeneration;
  emit(layoutChanged());
}
```

Why this is slow:
- Incrementing `m_renderCacheGeneration` makes existing cache entries stale by design.
- Immediately after model change, the next viewport paint has low cache-hit rate and triggers expensive rebuilds.

What this code does:
- Marks cache epoch as changed whenever data ordering/content may have changed.
- Forces correctness by preventing stale row text from being reused.
- Triggers view relayout/repaint through `layoutChanged()` or model reset flow.

```cpp
// src/searchtablemodel.cpp
void SearchTableModel::modelChanged()
{
  /* ... index update logic ... */
  ++m_renderCacheGeneration;
  emit(layoutChanged());
}

void SearchTableModel::clear_SearchResults()
{
  beginResetModel();
  m_searchResultList.clear();
  ++m_renderCacheGeneration;
  endResetModel();
}
```

2) After invalidation, first visible DisplayRole requests rebuild full row caches synchronously

```cpp
// src/tablemodel.cpp (inside TableModel::data)
const bool isRenderCacheValid = hasRenderCacheEntry
                && entry.filterPosIndex == filterposindex
                && entry.generation == m_renderCacheGeneration
                && entry.displayValues.size() == columnCount();

if (!isRenderCacheValid)
{
  entry = buildDecodeRenderCacheEntry(filterposindex, msg);
  m_decodeRenderCache.put(index.row(), entry);
}
```

Why this is slow:
- This rebuild runs on UI-thread during `data(...)` access, so repaint latency directly increases.
- Multiple visible rows can hit this path back-to-back right after CFI.

What this code does:
- Verifies cache entry coherence using row identity, generation, and column count.
- If stale/missing, reconstructs row display cache and inserts into LRU cache.

### 3) File open (CFI during open) feels slower
Open-file path includes index + filter + initial drawing of tables.

Primary causes:
- Cold startup of decode/render caches during the initial visible table paint.
- Two-stage view updates in open flow (index completion and filter completion) can trigger multiple cold-ish paint bursts.
- Preformatting all columns per row amplifies first-paint cost on wide tables.

Impact:
- Time-to-interactive after file open can regress even if backend indexing speed is unchanged.

Code snippets causing this slowdown:

1) Full-row render materialization on cache miss in main table path

```cpp
// src/tablemodel.cpp
TableModel::DecodeRenderCacheEntry TableModel::buildDecodeRenderCacheEntry(long int filterPosIndex, std::optional<QDltMsg> &msg) const
{
  DecodeRenderCacheEntry entry;
  entry.filterPosIndex = filterPosIndex;
  entry.generation = m_renderCacheGeneration;

  const int currentColumnCount = columnCount();
  entry.displayValues.reserve(currentColumnCount);
  for (int column = 0; column < currentColumnCount; ++column)
  {
    entry.displayValues.push_back(buildDisplayValue(column, filterPosIndex, msg));
  }

  return entry;
}
```

Why this is slow:
- Cost is front-loaded to first access of each row because all columns are formatted immediately.
- Heavy columns (payload transforms, APID/CTID description traversal, argument stringification) are paid even when not needed yet.

What this code does:
- Builds a row-level `DecodeRenderCacheEntry` for the main table.
- Tags it with current generation.
- Pre-fills `displayValues` for every visible column.

2) Decode and display formatting are in cold startup paint path

```cpp
// src/tablemodel.cpp (inside TableModel::data)
long int filterposindex = qfile->getMsgFilterPos(index.row());

std::optional<QDltMsg> msg;
QDltMsg omsg;
if (bool success = qfile->getMsg(filterposindex, omsg); success)
{
  msg = std::make_optional(omsg);
  if (QDltSettingsManager::getInstance()->value("startup/pluginsEnabled", true).toBool())
  {
    pluginManager->decodeMsg(*msg, !QDltOptManager::getInstance()->issilentMode());
  }
}

if (role == Qt::DisplayRole)
{
  /* cache validation ... */
  if (!isRenderCacheValid)
  {
    entry = buildDecodeRenderCacheEntry(filterposindex, msg);
    m_decodeRenderCache.put(index.row(), entry);
  }
  return entry.displayValues.at(index.column());
}
```

Why this is slow:
- During initial paint after open, message load + decode + cache-miss row materialization happen in the critical path.
- Startup can trigger many first-row requests quickly, multiplying cold-path work before UI feels responsive.

What this code does:
- Maps view row to filtered message index.
- Loads raw message, optionally decodes with plugins.
- For display requests, validates render cache and rebuilds row cache when needed.
- Returns formatted text for requested column from cached row vector.

## Core Regression Pattern
The current cache strategy optimizes repeated paints but increases first-hit work:
- Master: compute only what is asked now (cell/role scoped).
- Current: compute/store broader row payload on first access.

Related structure showing full-row cache storage:

```cpp
// src/tablemodel.h
struct DecodeRenderCacheEntry
{
  long int filterPosIndex;
  unsigned long long generation;
  QVector<QVariant> displayValues;
};
mutable QDltLruCache<int, DecodeRenderCacheEntry> m_decodeRenderCache{512};
```

Why this is slow in first-hit scenarios:
- The cache size helps repeated paints, but it does not reduce the initial cost of constructing each entry.
- Since each entry stores a full `QVector<QVariant>` for the row, miss cost scales with visible columns.

What this structure does:
- Keeps row-scoped preformatted display values with LRU eviction.
- Uses `generation` to invalidate stale data safely after model changes.

Expensive per-column formatting work that gets multiplied by full-row precompute:

```cpp
// src/tablemodel.cpp (inside TableModel::buildDisplayValue)
case FieldNames::AppId:
  switch(project->settings->showApIdDesc)
  {
  case 1:
    for(int num = 0; num < project->ecu->topLevelItemCount (); num++)
    {
      EcuItem *ecuitem = (EcuItem*)project->ecu->topLevelItem(num);
      if(!ecuitem) continue;
      for(int numapp = 0; numapp < ecuitem->childCount(); numapp++)
      {
        ApplicationItem * appitem = (ApplicationItem *) ecuitem->child(numapp);
        if(appitem && appitem->id == msg->getApid() && !appitem->description.isEmpty())
        {
          return appitem->description;
        }
      }
    }
    return QString("Apid: %1 (No description)").arg(msg->getApid());
  }

case FieldNames::Payload:
  visu_data = msg->toStringPayload().simplified().remove(QChar::Null);
  if(qfile) qfile->applyRegExString(*msg,visu_data);
  if(visu_data.size()>1000)
  {
    visu_data = visu_data.mid(0,1000);
  }
  return visu_data;
```

Why this is slow:
- APID description path can walk ECU/application trees for each evaluation.
- Payload path performs string normalization and optional regex replacements.
- When called for many columns during eager row build, these per-column costs accumulate.

What this code does:
- For `AppId`, optionally resolves and returns human-readable descriptions.
- For `Payload`, transforms payload text, applies filter regex replacements, and truncates for rendering.

```cpp
// src/searchtablemodel.h
struct DecodeRenderCacheEntry
{
  unsigned long messageIndex;
  unsigned long long generation;
  QVector<QVariant> displayValues;
};
mutable QDltLruCache<int, DecodeRenderCacheEntry> m_decodeRenderCache{512};
```

Why this also contributes in search view:
- Search model uses the same eager full-row value strategy, so first show of result rows carries the same front-loaded cost.

What this structure does:
- Stores search-row display values keyed by table row and validated by `messageIndex + generation`.

If workload is dominated by first-paint after model transitions (search result refresh, filter apply, file open), regression can appear.

## Secondary Contributors
- Multi-role fetch overhead: Foreground/Background calls can re-fetch decoded message in addition to Display path.
- Object copy overhead: optional/message copies in hot paint path.
- High column count (`showArguments`) increases first-hit row materialization cost.

## Part-wise Rectification of Slow Code Paths

This section maps each slow part directly to a concrete fix strategy.

### 1) Rectify full-row precompute on cache miss (SearchTableModel and TableModel)

Current slow part:
- `buildDecodeRenderCacheEntry(...)` loops across all columns and fills `displayValues` eagerly.

How to rectify:
- Convert row cache from eager-fill to per-column lazy-fill.
- Keep one row cache entry, but add per-column validity flags.
- Compute only requested column inside `data(index, Qt::DisplayRole)` when that column is still invalid.

Suggested cache shape:

```cpp
struct DecodeRenderCacheEntry
{
  long int filterPosIndex;                // or messageIndex for search model
  unsigned long long generation;
  QVector<QVariant> displayValues;
  QVector<quint8>   columnReady;          // 0:not ready, 1:ready
};
```

Suggested lazy logic:

```cpp
if (!isRenderCacheValid)
{
  entry.displayValues.resize(columnCount());
  entry.columnReady.fill(0, columnCount());
  entry.generation = m_renderCacheGeneration;
  entry.filterPosIndex = filterposindex;  // or messageIndex
}

const int c = index.column();
if (!entry.columnReady.at(c))
{
  entry.displayValues[c] = buildDisplayValue(c, filterposindex, msg);
  entry.columnReady[c] = 1;
}

m_decodeRenderCache.put(index.row(), entry);
return entry.displayValues.at(c);
```

Why this fixes it:
- First-hit cost becomes O(1 column) instead of O(all visible columns).
- Wide tables with arguments no longer pay full formatting cost upfront.

### 2) Rectify repeated message fetch/decode across roles

Current slow part:
- `DisplayRole`, `ForegroundRole`, and `BackgroundRole` can separately trigger message load/decode work for same row during one paint burst.

How to rectify:
- Add a small row-scoped decoded message cache keyed by `(row, generation)`.
- Reuse already loaded/decoded message for all role evaluations.
- Avoid repeated `qfile->getMsg(...)` and decode in Foreground/Background when DisplayRole already decoded it.

Suggested approach:

```cpp
struct DecodedRowEntry
{
  unsigned long long generation;
  bool hasMsg;
  QDltMsg msg;
};

mutable QDltLruCache<int, DecodedRowEntry> m_decodedRowCache{512};
```

Role path usage rule:
- Resolve message once per row-generation.
- Use same cached `msg` object for Display/Foreground/Background.

Why this fixes it:
- Removes redundant disk/cache lookups and redundant decode-manager calls in hot repaint cycles.

### 3) Rectify cold-start spikes after modelChanged / clear_SearchResults

Current slow part:
- Generation bump is correct but causes near-zero hit rate immediately after model changes.

How to rectify (without compromising correctness):
- Keep generation invalidation as-is.
- Combine with lazy per-column materialization so cold phase does minimal work per cell.
- Optionally prewarm only critical visible columns (`Index`, `Time`, `Payload`) for visible rows after layout change using a low-priority queued task.

Safe prewarm rule:
- Prewarm must be bounded to current viewport rows only.
- Stop prewarm if user scrolls or model changes again.

Why this fixes it:
- Maintains correctness while spreading cold-start work over time and keeping UI responsive.

### 4) Rectify expensive AppId/CtId and payload formatting paths

Current slow part:
- AppId/CtId description lookup traverses ECU/App/Context trees repeatedly.
- Payload formatting applies string cleanup and regex replacement repeatedly.

How to rectify:
- Build lookup maps once when metadata or settings change:
  - `QHash<QString, QString> apidDescMap`
  - `QHash<QPair<QString, QString>, QString> ctidDescMap`
- Replace nested traversal in `buildDisplayValue(...)` with O(1) hash lookups.
- For payload regex replacement, compile regex objects once per filter-set change and reuse compiled expressions.

Why this fixes it:
- Converts repeated deep traversal and repeated regex construction work into cached O(1) lookups and reused compiled regex.

### 5) Rectify avoidable copies in hot path

Current slow part:
- Local message copies and optional wrapping can add overhead in tight UI loops.

How to rectify:
- Prefer move-aware or in-place storage in decoded-row cache.
- Keep temporary allocations outside tight loops where possible.
- Return prebuilt `QVariant` references from cache vector when valid.

Why this fixes it:
- Reduces allocator and copy pressure during bursty paint operations.

### 6) Rectify by measuring the right boundaries (verification plan)

Add timers/counters around:
- Search matcher runtime only.
- CFI apply runtime only.
- First-paint-after-layoutChanged runtime.
- Cache hit/miss counts for:
  - row render cache
  - per-column readiness
  - decoded-row cache
  - per-role call counts

Acceptance criteria:
- First paint after search/CFI/open reduces significantly versus current branch.
- No correctness regressions in displayed text/colors.
- Scroll performance remains equal or better than current branch.

## Recommendations to Recover Performance

### A) Switch from full-row precompute to per-column lazy row cache
Keep row cache container, but compute a column only when requested.
- Store a `QVector<QVariant>` plus a parallel valid-bit vector.
- On `data(row,col)`, compute only missing `col`.
- Preserves repaint benefit while reducing first-hit overwork.

### B) Reuse decoded message within one `data()` call across roles
Avoid repeated decode fetch/copy for Display/Foreground/Background evaluation in the same row request.

### C) Keep cache invalidation but reduce cold-start penalty
Generation invalidation is correct for correctness; do not remove it.
- Instead reduce first-paint work per request (A+B).

### D) Add timing instrumentation around these boundaries
Measure separately:
- matcher runtime
- CFI runtime
- first-paint-after-modelChanged runtime
- per-role `data()` call counts and cache hit/miss rates

Without this separation, UI first-paint cost is misattributed to search/CFI backend runtime.

## Expected Outcome After Applying A+B
- Scroll/repaint remains better than master for stable viewport.
- Search result reveal delay decreases.
- Perceived delay after filter apply decreases.
- Time-to-interactive after file open improves while keeping render-cache correctness.

## Conclusion
The implementation delivers the intended repaint optimization but currently shifts too much work into first-hit rendering after model transitions. This is why search/CFI/open feel slower vs master. The fix is not to remove caching, but to make cache fill more granular (per-column lazy) and reduce repeated per-role decode fetch overhead.