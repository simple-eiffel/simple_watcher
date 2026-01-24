# 7S-02 STANDARDS - simple_watcher


**Date**: 2026-01-23

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## Applicable Standards

### Win32 API
- **ReadDirectoryChangesW**: Core Windows API for directory monitoring
- Uses overlapped I/O for asynchronous operation

### Watch Flags (FILE_NOTIFY_CHANGE_*)
| Flag | Value | Description |
|------|-------|-------------|
| Watch_file_name | 0x0001 | File create/delete/rename |
| Watch_dir_name | 0x0002 | Directory create/delete/rename |
| Watch_attributes | 0x0004 | Attribute changes |
| Watch_size | 0x0008 | Size changes |
| Watch_last_write | 0x0010 | Last write time changes |
| Watch_security | 0x0020 | Security descriptor changes |
| Watch_all | 0x003F | All of the above |

### Event Types (FILE_ACTION_*)
| Type | Value | Description |
|------|-------|-------------|
| Event_file_added | 1 | FILE_ACTION_ADDED |
| Event_file_removed | 2 | FILE_ACTION_REMOVED |
| Event_file_modified | 3 | FILE_ACTION_MODIFIED |
| Event_file_renamed | 4 | FILE_ACTION_RENAMED_* |

### Eiffel Standards
- **ECMA-367**: Eiffel language standard
- **Void Safety**: Full void-safe implementation
- **SCOOP Compatible**: Design supports concurrency

## C Interface Standards

Uses inline C externals with `simple_watcher.h` header. Functions follow `sw_` prefix convention.
