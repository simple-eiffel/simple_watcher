# S02 CLASS CATALOG - simple_watcher

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## Class Hierarchy

```
SIMPLE_WATCHER
  - Directory change monitor
  - No inheritance

SIMPLE_WATCH_EVENT
  - Change event data
  - No inheritance
```

## Class Descriptions

### SIMPLE_WATCHER

**Purpose**: Monitor directory for file system changes

**Responsibilities**:
- Create watcher for specified directory
- Configure watch flags and recursion
- Start/stop watching
- Poll or wait for events
- Report errors

**Key Features**:
- `make`: Initialize watcher
- `start`: Begin monitoring
- `poll`: Non-blocking event check
- `wait`: Blocking event wait with timeout
- `close`: Stop and release resources
- `is_watching`: Active status
- `watch_path`: Monitored directory
- `last_error`: Error information

### SIMPLE_WATCH_EVENT

**Purpose**: Represent a single file system change

**Responsibilities**:
- Store event type and filename
- Provide type classification

**Key Features**:
- `event_type`: Change type code (1-4)
- `filename`: Affected file name
- `is_added`, `is_removed`, `is_modified`, `is_renamed`: Type queries
- `event_type_string`: Human-readable type

## Constants

### Watch Flags (SIMPLE_WATCHER)
| Constant | Value | Description |
|----------|-------|-------------|
| Watch_file_name | 0x0001 | File name changes |
| Watch_dir_name | 0x0002 | Directory name changes |
| Watch_attributes | 0x0004 | Attribute changes |
| Watch_size | 0x0008 | Size changes |
| Watch_last_write | 0x0010 | Last write changes |
| Watch_security | 0x0020 | Security changes |
| Watch_all | 0x003F | All changes |

### Event Types (SIMPLE_WATCHER)
| Constant | Value | Description |
|----------|-------|-------------|
| Event_file_added | 1 | File created |
| Event_file_removed | 2 | File deleted |
| Event_file_modified | 3 | File modified |
| Event_file_renamed | 4 | File renamed |
