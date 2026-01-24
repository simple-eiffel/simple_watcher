# 7S-04 SIMPLE-STAR - simple_watcher


**Date**: 2026-01-23

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## Ecosystem Dependencies

### Required Libraries
| Library | Purpose | Usage |
|---------|---------|-------|
| None | - | Standalone Win32 wrapper |

### External Dependencies
| Component | Purpose |
|-----------|---------|
| simple_watcher.h | C interface header |
| simple_watcher.c | C implementation |
| Win32 API | ReadDirectoryChangesW |

### Standard Library
| Component | Purpose |
|-----------|---------|
| C_STRING | Path string conversion |
| POINTER | Opaque handle management |

## Integration Points

### Watch Directory Pattern
```eiffel
create watcher.make ("D:/projects", True, watcher.Watch_all)
if watcher.is_valid then
    watcher.start
    from until not watcher.is_watching loop
        if attached watcher.poll as event then
            print (event.event_type_string + ": " + event.filename)
        end
        sleep_ms (100)
    end
    watcher.close
end
```

### Blocking Wait Pattern
```eiffel
watcher.start
event := watcher.wait (5000)  -- 5 second timeout
if event /= Void then
    -- Process event
end
```

## Ecosystem Position

simple_watcher enables:
- Hot-reload in simple_web servers
- File synchronization tools
- Build watch modes
- Editor auto-refresh
