# 7S-01 SCOPE - simple_watcher

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher
**Purpose**: SCOOP-compatible file system watcher using Win32 ReadDirectoryChangesW

## Problem Domain

simple_watcher addresses the need for file system change monitoring in Eiffel applications. Applications often need to react to file changes for hot-reloading, synchronization, or event-driven processing.

## Scope Boundaries

### In Scope
- Monitor directory for file changes
- Watch for file/directory creation, deletion, modification, rename
- Optional recursive subdirectory watching
- Non-blocking poll for events
- Blocking wait with timeout
- SCOOP-compatible design

### Out of Scope
- Cross-platform support (Windows only via ReadDirectoryChangesW)
- Network file system watching
- File content change detection
- Debouncing rapid changes
- Pattern filtering (e.g., *.txt only)
- Symbolic link handling

## Target Users

- Eiffel developers needing file change notifications
- Hot-reload implementations
- File synchronization tools
- Build systems watching for changes

## Success Criteria

1. Detect file create, modify, delete, rename events
2. Support recursive and non-recursive watching
3. Provide both polling and blocking APIs
4. Clean start/stop lifecycle
