# 7S-03 SOLUTIONS - simple_watcher


**Date**: 2026-01-23

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## Alternative Approaches Considered

### 1. Polling File Timestamps
**Description**: Periodically stat files to detect changes
**Pros**: Cross-platform, simple
**Cons**: High CPU for large directories, delayed detection
**Decision**: Rejected - inefficient

### 2. inotify (Linux)
**Description**: Use Linux inotify API
**Pros**: Efficient, well-documented
**Cons**: Linux only
**Decision**: Not applicable (Windows target)

### 3. ReadDirectoryChangesW (Selected)
**Description**: Win32 directory change notification
**Pros**: Native, efficient, comprehensive events
**Cons**: Windows only
**Decision**: Selected - best for Windows target

### 4. Cross-Platform Library (libuv, etc.)
**Description**: Use existing cross-platform library
**Pros**: Portable
**Cons**: Additional dependency, complex integration
**Decision**: Rejected - simplicity preferred

## Implementation Strategy

1. C wrapper (simple_watcher.h/c) handles Win32 complexity
2. Eiffel class provides clean SCOOP-compatible API
3. Event objects carry change information
4. Both poll and wait patterns supported

## Technology Stack

- **Win32 API**: ReadDirectoryChangesW, overlapped I/O
- **C wrapper**: simple_watcher.h/c
- **Event structure**: sw_event with type and filename
