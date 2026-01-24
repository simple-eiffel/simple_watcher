# S06 BOUNDARIES - simple_watcher

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## System Boundaries

```
+--------------------------------------------------+
|                    Client Code                    |
+--------------------------------------------------+
                        |
                        v
+--------------------------------------------------+
|                  SIMPLE_WATCHER                   |
|        make / start / poll / wait / close         |
+--------------------------------------------------+
                        |
                        v
+--------------------------------------------------+
|              C Layer (simple_watcher.h)           |
|   sw_create, sw_start, sw_poll, sw_wait, sw_close |
+--------------------------------------------------+
                        |
                        v
+--------------------------------------------------+
|               Win32 API                           |
|   ReadDirectoryChangesW, WaitForSingleObject      |
+--------------------------------------------------+
                        |
                        v
+--------------------------------------------------+
|               File System                         |
+--------------------------------------------------+
```

## Interface Boundaries

### Public Interface (Eiffel)
- SIMPLE_WATCHER: Main watcher class
- SIMPLE_WATCH_EVENT: Event data class

### C Interface (simple_watcher.h)
- `sw_create()`: Create watcher handle
- `sw_start()`: Begin monitoring
- `sw_poll()`: Non-blocking event check
- `sw_wait()`: Blocking wait
- `sw_close()`: Release resources
- `sw_free_event()`: Free event memory

## Data Boundaries

### Input Data
| Data | Source | Validation |
|------|--------|------------|
| Watch path | Client | Path exists check |
| Flags | Client | None (OS validates) |
| Timeout | Client | None |

### Output Data
| Data | Destination | Format |
|------|-------------|--------|
| Events | Client | SIMPLE_WATCH_EVENT |
| Status | Client | BOOLEAN |
| Errors | Client | STRING_32 |

## Trust Boundaries

| Zone | Trust Level | Validation |
|------|-------------|------------|
| Client code | Trusted | Preconditions |
| C layer | Trusted | Internal |
| Win32 API | Trusted | System |
| File system | External | OS permissions |
