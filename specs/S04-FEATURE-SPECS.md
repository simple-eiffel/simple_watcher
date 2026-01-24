# S04 FEATURE SPECS - simple_watcher

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## SIMPLE_WATCHER Features

### Initialization

| Feature | Signature | Description |
|---------|-----------|-------------|
| make | (STRING, BOOLEAN, INTEGER) | Create watcher for path |

### Status

| Feature | Signature | Description |
|---------|-----------|-------------|
| is_valid | BOOLEAN | Is handle valid? |
| is_watching | BOOLEAN | Actively watching? |
| watch_path | detachable STRING_8 | Path being watched |
| last_error | detachable STRING_32 | Error message |
| last_start_succeeded | BOOLEAN | Start result |

### Operations

| Feature | Signature | Description |
|---------|-----------|-------------|
| start | | Begin watching |
| poll | : detachable WATCH_EVENT | Non-blocking check |
| wait | (INTEGER): detachable WATCH_EVENT | Blocking wait |
| close | | Stop and release |

### Constants - Watch Flags

| Constant | Value | Description |
|----------|-------|-------------|
| Watch_file_name | 0x0001 | File name changes |
| Watch_dir_name | 0x0002 | Directory changes |
| Watch_attributes | 0x0004 | Attribute changes |
| Watch_size | 0x0008 | Size changes |
| Watch_last_write | 0x0010 | Write time changes |
| Watch_security | 0x0020 | Security changes |
| Watch_all | 0x003F | All changes |

### Constants - Event Types

| Constant | Value | Description |
|----------|-------|-------------|
| Event_file_added | 1 | File created |
| Event_file_removed | 2 | File deleted |
| Event_file_modified | 3 | File modified |
| Event_file_renamed | 4 | File renamed |

## SIMPLE_WATCH_EVENT Features

### Access

| Feature | Signature | Description |
|---------|-----------|-------------|
| event_type | INTEGER | Type code (1-4) |
| filename | STRING_8 | Affected filename |

### Status

| Feature | Signature | Description |
|---------|-----------|-------------|
| is_added | BOOLEAN | Type = 1? |
| is_removed | BOOLEAN | Type = 2? |
| is_modified | BOOLEAN | Type = 3? |
| is_renamed | BOOLEAN | Type = 4? |
| event_type_string | STRING_8 | Human-readable type |
