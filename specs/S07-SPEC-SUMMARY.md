# S07 SPEC SUMMARY - simple_watcher

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## Executive Summary

simple_watcher provides SCOOP-compatible file system monitoring for Windows. It wraps Win32 ReadDirectoryChangesW with a clean Eiffel API supporting both polling and blocking patterns.

## Key Specifications

### Classes
| Class | Purpose | Complexity |
|-------|---------|------------|
| SIMPLE_WATCHER | Directory monitor | Medium |
| SIMPLE_WATCH_EVENT | Event data | Low |

### Features by Category
| Category | Count | Key Features |
|----------|-------|--------------|
| Lifecycle | 4 | make, start, close, is_valid |
| Event retrieval | 2 | poll, wait |
| Status | 3 | is_watching, watch_path, last_error |
| Constants | 11 | Watch flags, event types |

### Contract Coverage
| Contract Type | Count |
|--------------|-------|
| Preconditions | 5 |
| Postconditions | 1 |
| Invariants | 0 |

## Dependencies

### Required
- Win32 API (runtime)
- simple_watcher.h/c (C layer)

### Optional
- None

## Quality Metrics

| Metric | Value |
|--------|-------|
| Source lines | ~350 |
| Classes | 2 |
| Features | ~20 |
| Test coverage | Present |

## API Summary

```eiffel
create watcher.make ("D:/projects", True, watcher.Watch_all)
if watcher.is_valid then
    watcher.start
    if watcher.last_start_succeeded then
        from until done loop
            if attached watcher.poll as ev then
                print (ev.event_type_string + ": " + ev.filename)
            end
        end
    end
    watcher.close
end
```

## Status

**Phase**: 2 (Expanded Features)
**Stability**: Stable
**Production Ready**: Yes (Windows only)
