# High Level Design (HLD) — dlt-viewer

Version: 0.1  
Author: (placeholder)  
Date: (placeholder)

## 1. Purpose
Describe architecture, components, flows, interfaces and non‑functional requirements for dlt-viewer.

## 2. Scope
Covers repository at c:\Users\BTI-001434\Documents\dlt_openSource\dlt-viewer — core (qdlt), UI (src/parser), plugins, CLI tools, build & packaging.

## 3. Goals & Constraints

## 4. Stakeholders
Performance, extensibility, portability, observability, security considerations.

- UI (Qt): MainWindow, models, views, background indexer (src/, parser/)
- Plugins: extend decoding/viewing (plugin/)
- CLI: parser, commander (parser/, commander/)
- Build/Packaging: CMake + scripts

## 7. Component Responsibilities (high level)
- qdlt: qdltfile, qdltmessagedecoder, qdltfilter*, plugininterface.h
- UI: MainWindow, tablemodel, dltfileindexer, filterdialog, project persistence
- Plugins: implement plugininterface to register viewers/decoders
- CLI: batch parsing/export using qdlt core
- Packaging: scripts/, CMakePresets.json

![Architecture diagram](architecture.png)

## 8. Key Data Flows
- File open → qdltfile (indexing) → qdltmessagedecoder → dltmsgqueue → TableModel → UI (Filter proxy)
- Plugin load → plugin registers hooks → decoder/UI invokes plugin callbacks → plugin augments UI/metadata

## 9. Interfaces & Contracts
- Public API: headers in qdlt/ (document exported functions/types)
- Plugin interface: qdlt/plugininterface.h — define lifecycle, threading expectations
- Indexer ↔ UI: Qt signals/slots and message queue (document names/payloads)

## 10. Concurrency Model
- Indexing & decoding: worker threads
- UI: Qt main thread; updates via queued signals/slots
- Plugins: must be thread-safe — document callback thread context

## 11. Persistence & Configuration
- Project files & filters stored via src/project.* and filters/*.dlf

## 12. Testing & Verification
- Unit tests for qdlt decoding/filters
- Integration tests opening representative DLT files
- UI tests for main flows
- Plugin conformance tests

## 13. Risks & Mitigations
- Large files → indexing/streaming, LRU cache
- Plugin instability → API versioning and safe error handling
- Cross‑platform packaging → CI per platform

## 14. Next Steps (placeholders)
- Finalize public API list (headers)
- Add PlantUML diagrams (component + sequence) — see architecture.puml
- Document plugin lifecycle threading rules
- Add unit/integration tests

## 15. Artifacts to Add
- architecture.puml (component + sequence)
- public_api.md (list of qdlt headers & exported symbols)


## References

- README.md, INSTALL.md

- qdlt/, parser/, plugin/, scripts/

