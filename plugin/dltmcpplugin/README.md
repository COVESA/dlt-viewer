# DLT Viewer MCP Plugin

This plugin implements the Model Context Protocol (MCP) server for DLT Viewer, enabling AI assistants (e.g. VS Code, Windsurf, or Claude Desktop) to directly access and analyze DLT log files.

## Features

- Read trace context, decoded messages, and error messages.
- Search by payload pattern, DLT headers, or timestamp, then navigate to results.
- Inspect and manage filters, including temporary filters that do not modify the saved project.
- Inspect and control configured ECU connections.

## MCP Tools Exposed

All tools are called by the AI via the MCP `tools/call` method. Parameters marked
`(optional)` have sensible defaults and can be omitted.

### File & Message Access

For tools with `use_filter` (default `true`), `true` uses the active filtered view when one exists, otherwise the full log. Set it to `false` to always use the unfiltered file.

| Tool | Parameters | Description |
|------|-----------|-------------|
| `get_context` | — | Returns a high-level summary of the current DLT Viewer context: loaded trace/file, filter state, and a sampled overview of message types, ECUs, APIDs, and CTIDs. Useful when the AI first connects. |
| `get_dictionary` | `term` (string, optional) | Returns a DLT terminology dictionary. Explains terms such as `trace`, `filtered view`, `unfiltered view`, `ECUID`, `APID`, `CTID`, Time, Timestamp, message types, and log subtypes/levels. If `term` is provided, focuses on that entry. |
| `get_file_info` | — | Returns total and filtered message counts. **Call this first** to understand the data set size before reading ranges. |
| `get_messages_range` | `start` (int), `end` (int), `use_filter` (bool, optional) | Reads messages `[start, end)`. Max 500 per call. Results include wall-clock time, normalized monotonic timestamp, raw timestamp, ECU/APID/CTID, type, subtype, mode, count, SessionId, and payload. |
| `search_logs` | `pattern` (string, optional), `start_index` (int, optional), `max_scan` (int, optional), `ecuid` (string, optional), `apid` (string, optional), `ctid` (string, optional), `type` (string, optional), `subtype` (string, optional), `mode` (string, optional), `session_id` (string, optional), `use_filter` (bool, optional), `max_results` (int, optional) | Searches messages by payload regex and/or structured DLT header filters. `pattern` is a case-insensitive regular expression matched against the decoded payload. At least one of `pattern`, `ecuid`, `apid`, `ctid`, `type`, `subtype`, `mode`, or `session_id` is required. Large-trace searches can be resumed with `start_index`, `max_scan`, and the returned `next_index`; see the tool description for the cursor and completion contract. `max_results` defaults to 100 and is capped at 200. |
| `find_message_by_timestamp` | `timestamp` (string), `use_filter` (bool, optional) | Finds the message nearest to a wall-clock timestamp. Use this when you know a user-visible time and need the corresponding log position and monotonic timestamp anchor. Supported timestamp formats: `yyyy/MM/dd HH:mm:ss[.zzz]` or `HH:mm:ss[.zzz]`. |
| `search_logs_around_index` | `index` (int), `pattern` (string, optional), `ecuid` (string, optional), `apid` (string, optional), `ctid` (string, optional), `type` (string, optional), `subtype` (string, optional), `mode` (string, optional), `session_id` (string, optional), `window_before_ms` (int, optional), `window_after_ms` (int, optional), `use_filter` (bool, optional), `max_results` (int, optional) | Searches around a known anchor global index using payload regex and/or structured DLT header filters. `pattern` is a case-insensitive regular expression matched against the decoded payload. The search window is defined around the anchor message's DLT monotonic timestamp. When timestamps are ordered, this can avoid scanning the entire file; with out-of-order timestamps, the search may scan a wider range and can miss in-window messages beyond an out-of-window boundary in file order. At least one of `pattern`, `ecuid`, `apid`, `ctid`, `type`, `subtype`, `mode`, or `session_id` must be provided. Defaults to a -5000 ms / +5000 ms monotonic window. `max_results` defaults to 100 and is capped at 200. |
| `get_error_messages` | `use_filter` (bool, optional), `max_results` (int, optional) | Returns Fatal, Error, and Warning messages. `max_results` defaults to 100 and is capped at 200. For exhaustive coverage beyond the cap, use `search_logs` with `subtype` filters (`Fatal`, `Error`, `Warn`) and paginate with `start_index`/`next_index`. |
| `navigate_to_message` | `index` (int) | Scrolls DLT Viewer to the given global message index and selects it in the table. Use the `global_index` values returned by read/search tools. |
| `clear_logs` | — | Clears all messages from the current DLT Viewer trace view. |

### ECU Connection Control

| Tool | Parameters | Description |
|------|-----------|-------------|
| `list_connections` | — | Lists every configured ECU connection (index, description, optional hostname). Use this before per-ECU connect/disconnect operations. |
| `get_connection_states` | — | Returns the current state (`offline`, `connecting`, `online`, `error`) for each configured ECU. |
| `connect_ecu` | `index` (int) | Connects a single ECU (by index from `list_connections`) so DLT Viewer starts receiving messages from that target. |
| `disconnect_ecu` | `index` (int) | Disconnects a single ECU (by index) so DLT Viewer stops receiving messages from that target. |
| `connect_all_ecus` | — | Connects to all configured ECUs. Use this when the user says “start logging” or “reconnect everything.” |
| `disconnect_all_ecus` | — | Disconnects from all configured ECUs. Use this when the user says “stop logging” or “disconnect everything.” |

> **Note:** These tools operate at the transport level (TCP/serial connect/disconnect). Fine-grained log level control (`SET_LOG_LEVEL`, `SET_TRACE_STATUS`) is not available to plugins without modifying DLT Viewer core.

### Filter Management

| Tool | Parameters | Description |
|------|-----------|-------------|
| `list_filters` | — | Lists all configured filters with their index, type, criteria, and enabled state. Use the returned index in `remove_filter` and `set_filter_enabled`. |
| `add_filter` | `type` (string, optional), `name` (string, optional), `ecuid`, `apid`, `ctid`, `header`, `payload` (all string, optional) | Creates a new filter. `type` is `"positive"` (default, show only matching), `"negative"` (hide matching), or `"marker"` (highlight only). Provide at least one criterion. The filtered view is rebuilt immediately. On very large traces, MCP may reject this operation to avoid freezing DLT Viewer during a full filtered-view rebuild. |
| `remove_filter` | `index` (int) | Permanently removes the filter at the given index. The filtered view is rebuilt immediately. On very large traces, MCP may reject this operation to avoid freezing DLT Viewer during a full filtered-view rebuild. |
| `set_filter_enabled` | `index` (int), `enabled` (bool) | Activates or deactivates a filter without removing it. Useful for temporarily suspending a filter. On very large traces, MCP may reject this operation to avoid freezing DLT Viewer during a full filtered-view rebuild. |

> **Important — filters added via MCP are transient:**
> - They modify `QDltFile`'s in-memory filter state directly.
> - They do **not** appear in the DLT Viewer filter panel UI.
> - They are **not** saved to the `.dlp` project file.
> - They are lost when DLT Viewer is closed or the file is reloaded.
>
> This is intentional — the AI's filters should not pollute the user's saved project configuration.
>
> **Large trace safeguard:**
> - MCP filter mutation tools (`add_filter`, `remove_filter`, `set_filter_enabled`) can be blocked on large traces.
> - This safeguard exists because rebuilding the filtered view is a synchronous full-file operation in DLT Viewer and can freeze the UI.
> - If the AI only needs to inspect the full trace temporarily, prefer read/search tools with `use_filter=false` instead of changing filter state.

## Quick Start

1. Build DLT Viewer with this plugin enabled
2. Start DLT Viewer
3. The MCP server starts automatically with the configured endpoint and binding; see [MCP Endpoint URLs](#mcp-endpoint-urls).
4. Copy the bearer token from the plugin status panel (see [Authentication](#authentication))
5. Configure your AI assistant (e.g. VS Code, Windsurf, or Claude Desktop) to connect to the MCP server and send the token
6. Start analyzing logs with AI!

## Authentication

Every MCP request must carry an `Authorization: Bearer <token>` header. The token is generated on first launch and persisted to:

- Linux/macOS: `~/.dlt/mcp/token.txt`
- Windows: `%USERPROFILE%\.dlt\mcp\token.txt`

The token is also displayed at the top of the plugin's status panel (dropdown arrow next to the plugin name), so you can copy it without opening the file. Requests without a valid token receive `401 Unauthorized`.

The token is a shared-secret gate: any client with it can call the MCP tools. It does not identify a particular AI harness.

To rotate the token: stop DLT Viewer, delete the token file, and start DLT Viewer again. A new token is generated on the next launch. Update the MCP client configuration accordingly.

## Privacy and LLM Use

The DLT MCP plugin does not select or approve the LLM or AI service used by your MCP client. As the DLT Viewer user, you are responsible for choosing a service and confirming that its use is permitted for your environment and the data in your trace, including applicable organizational security and privacy requirements.

Connected MCP clients can request DLT data, including message payloads that may contain sensitive or personal information. Read tools can access the full loaded trace with `use_filter=false`, even when a DLT Viewer filter is active. Tool results are returned to the MCP client and may be sent to or processed by the model provider, depending on the client and deployment.

Before connecting, review the trace contents, your MCP client's data handling, and the provider's retention, training, and data residency policies. Do not connect a service that is not approved for the data.

## MCP Endpoint URLs

The plugin uses HTTP/SSE and listens on port `3719` by default. By default, it binds to loopback only; the canonical URL is:

```text
http://127.0.0.1:3719
```

For compatibility with clients that expect an explicit MCP path, the plugin also accepts:

```text
http://127.0.0.1:3719/mcp
```

Setting `allowRemoteConnections` to `true` binds the server to all network interfaces. HTTP does not encrypt the bearer token or DLT data in transit, so enable remote access only on a trusted, access-controlled network. If using a secure tunnel, restrict direct access to the listener so traffic cannot bypass it.

## Client Configuration

### VS Code MCP Configuration

Configure VS Code to connect to this MCP server via `mcp.json`.

Configuration location:
- Workspace: `.vscode/mcp.json`
- User profile: Command Palette -> `MCP: Open User Configuration`

Configuration example:

```json
{
  "servers": {
    "dlt-viewer": {
      "type": "http",
      "url": "http://127.0.0.1:3719",
      "headers": {
        "Authorization": "Bearer <paste-token-from-plugin-panel>"
      }
    }
  }
}
```

After saving `mcp.json`, start or restart the server from VS Code if prompted, then confirm trust when VS Code asks.

### Windsurf MCP Configuration

Configure Windsurf to connect to this MCP server via `mcp_config.json`.

Configuration location:
- `~/.codeium/windsurf/mcp_config.json`

Configuration example:

```json
{
  "mcpServers": {
    "dlt-viewer": {
      "disabled": false,
      "url": "http://127.0.0.1:3719",
      "headers": {
        "Authorization": "Bearer <paste-token-from-plugin-panel>"
      }
    }
  }
}
```

After saving `mcp_config.json`, restart Windsurf so the MCP server configuration is reloaded.

### Claude Desktop MCP Configuration

Configure Claude Desktop to connect to this MCP server as a remote MCP connector.

Configuration location:
- Claude Desktop settings -> Connectors -> Add custom connector

Configuration value:
- MCP server URL: `http://127.0.0.1:3719`
- Optional explicit MCP path (if required by the client): `http://127.0.0.1:3719/mcp`
- Custom header: `Authorization: Bearer <paste-token-from-plugin-panel>`

After adding the connector, complete any trust/auth prompts in Claude Desktop and reload the session if requested.

## Plugin Configuration

The plugin can load its settings from an XML config file referenced by the DLT Viewer project (`.dlp`). The XML file is **not** created automatically. If you want to use an external plugin config, create the XML file manually and configure the plugin to use it so that the `.dlp` stores a reference to that file. When DLT Viewer loads the plugin configuration, the MCP server starts only after that configuration has been applied. If no external config is provided, the plugin falls back to the built-in defaults.

### Settings Reference

| Setting | Default | Description |
|---------|---------|-------------|
| `port` | `3719` | TCP port the MCP server listens on. Change it if the port is already in use. |
| `allowRemoteConnections` | `false` | Set to `true` to allow remote clients; see [MCP Endpoint URLs](#mcp-endpoint-urls) for binding and security details. |
| `filterMutationMessageLimit` | empty | Optional limit for the number of messages that can be processed when mutating filters. If not set, the default limit of 100000 is applied. If set to -1, no limit is applied. A limit is recommended to prevent UI freezing during filter changes. |
| `dictionary` | empty | Optional custom dictionary entries for `get_dictionary`. These can add new terms or override built-in ones by using the same key. |

### Configuration File Example

The config file is a simple XML file:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<DltMcpPlugin>
    <port>3719</port>
    <allowRemoteConnections>false</allowRemoteConnections>
    <filterMutationMessageLimit>100000</filterMutationMessageLimit>
    <dictionary>
        <entry key="trace" aliases="capture, session">Custom explanation for trace.</entry>
        <entry key="myapp" aliases="my app, feature x">Application-specific explanation.</entry>
    </dictionary>
</DltMcpPlugin>
```

> **There is no settings UI.** Settings are changed by editing the config file directly.
>
> The MCP server waits until `loadConfig()` has run before it binds its TCP port. This allows an externally provided plugin config to override the default port before startup. When you select a plugin config file in DLT Viewer, the configuration is loaded automatically and the MCP server is restarted automatically with the new host/port.

### Operational Tasks

#### Change the Port

Edit `<port>` in the plugin XML config and reload/select the config in DLT Viewer; if you do not reload it, restart DLT Viewer. Update the port in the endpoint configured in your MCP client.

#### Enable Remote Connections (VM / container use case)

Only needed if DLT Viewer runs inside a VM or container and your MCP client runs on the host machine.

Set `<allowRemoteConnections>true</allowRemoteConnections>` in the plugin XML config and reload/select it in DLT Viewer; if you do not reload it, restart DLT Viewer. Configure the client to use the VM's IP address and the configured port. Review the security warning under [MCP Endpoint URLs](#mcp-endpoint-urls) before enabling remote access.

The current host, port, and server status are visible in the DLT Viewer plugin (click the dropdown arrow next to the plugin name in the plugin list).

## Troubleshooting

- Connection failure: confirm DLT Viewer and the plugin are running, check the client's host and port against the plugin settings, and reload the config or restart DLT Viewer after changes.
- Remote connection failure: use the VM/container IP and follow [Enable Remote Connections](#enable-remote-connections-vm--container-use-case).
- Requests fail with `401 Unauthorized`:
  - The `Authorization: Bearer <token>` header is missing or the token is wrong.
  - Copy the current token from the plugin status panel (or `~/.dlt/mcp/token.txt`) into your MCP client config and reload.
- Requests fail with expired session errors:
  - Re-establish the SSE connection to obtain a fresh session and retry.

## Example Workflows

### Context and Dictionary

```text
User: "Before analyzing, summarize the trace and explain what 'trace' means here."

AI: get_context()
  → returns the current file name, filter state, and a sampled overview of
    message types, ECUs, APIDs, and CTIDs
AI: get_dictionary(term="trace")
  → explains that "trace" means the DLT data currently loaded in DLT Viewer,
    usually the open .dlt file / current trace session
```

### Error Analysis with Source Code Correlation
```text
User: "Are there any errors in the DLT logs? Find the relevant code and explain why."

AI: get_file_info()
  → "120,000 total, 45,000 filtered messages"
AI: get_error_messages()
  → finds 3 Fatal messages from APID=NETM, CTID=CONN
AI: search_logs("connection refused")
  → 12 matches, global_index values 1042, 1891, ...
AI: navigate_to_message(1042)
  → DLT Viewer jumps to the first occurrence
AI: correlates payload with network stack source code in workspace
  → "The error originates in NetworkManager.cpp:247 — the retry
    timeout is too short when the server is under load."
```

### Focused Investigation Without Changing User Filters
```text
User: "Check only the MYAPP application for memory issues,
       but don't change my current filter setup."

AI: add_filter(type="positive", apid="MYAP", payload="memory")
AI: get_error_messages()
  → only MYAP memory-related errors visible
AI: search_logs("heap")
  → finds heap allocation failures
AI: remove_filter(index=<new index>)
  → user's original filter view is restored
```

### Temporarily Bypass the Active Filter
```text
User: "I have a filter active, but check the whole file for crashes."

AI: get_error_messages(use_filter=false)
  → scans all 120,000 messages regardless of filter
AI: search_logs("SIGSEGV", use_filter=false)
  → finds crash reports hidden by the active filter
```

### Structured Header Search and Navigation
```text
User: "Jump to the first warn message."

AI: search_logs(subtype="warn")
  → first result has global_index 1653
AI: navigate_to_message(1653)
  → DLT Viewer jumps to the first warn message in the current scope
```

### Regex Payload Search
```text
User: "Find startApp calls for message_handling."

AI: search_logs(pattern="startApp.*message_handling")
  → matches payloads such as
    AppsRuntimeController::startApp(message_handling, ...)
```

### Header-Filtered Search Around a Timestamp
```text
User: "Find warn messages around 14:00:48.105."

AI: find_message_by_timestamp(timestamp="14:00:48.105")
  → returns anchor global_index 1653
AI: search_logs_around_index(index=1653, subtype="warn")
  → returns warn messages in the monotonic timestamp window around that anchor
```

### Filter Inspection and Management
```text
User: "What filters are currently active?"

AI: list_filters()
  → shows all filters with index, type, criteria, enabled state

User: "Disable the APID filter temporarily."

AI: set_filter_enabled(index=2, enabled=false)
  → filter suspended, view updates immediately
AI: set_filter_enabled(index=2, enabled=true)
  → filter restored
```

## Known Limitations

### Single concurrent long-running search

The plugin supports multiple simultaneous MCP client connections (e.g. two IDE windows pointing at the same DLT Viewer instance), but only one instance of each long-running search operation can execute at a time:

- `search_logs` and `get_error_messages` share one execution slot and cancel each other.
- `find_message_by_timestamp` has its own slot; a second request cancels the first.
- `search_logs_around_index` has its own slot; a second request cancels the first.

**Effect with multiple clients**: if Client A has a search in progress and Client B issues a search of the same type, Client A's search is immediately cancelled and receives a JSON-RPC error response. Client B's search then runs to completion. Clients should be prepared to retry searches that return a cancellation error.

Fast, synchronous tools (`get_context`, `get_file_info`, `get_messages_range`, filter tools, navigation tools) are not affected — they always complete immediately regardless of how many clients are connected.

## Build

This plugin is built as part of DLT Viewer. See [INSTALL.md](../../INSTALL.md) for build instructions. Requirements: Qt 6 (including `QtNetwork` and `QtWidgets`), the DLT Viewer core library (`qdlt`), CMake 3.15 or later, and MSVC 2022 with C++17 support on Windows.
