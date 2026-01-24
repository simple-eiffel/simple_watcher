# S03 CONTRACTS - simple_watcher

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## SIMPLE_WATCHER Contracts

### make
```eiffel
make (a_path: READABLE_STRING_GENERAL; a_watch_subtree: BOOLEAN; a_flags: INTEGER)
    require
        path_not_empty: not a_path.is_empty
    -- Post: handle set (may be invalid if path doesn't exist)
```

### start
```eiffel
start
    require
        valid: is_valid
    -- Post: last_start_succeeded indicates result
```

### poll
```eiffel
poll: detachable SIMPLE_WATCH_EVENT
    require
        valid: is_valid
        watching: is_watching
    -- Returns: Event if pending, Void otherwise
```

### wait
```eiffel
wait (a_timeout_ms: INTEGER): detachable SIMPLE_WATCH_EVENT
    require
        valid: is_valid
        watching: is_watching
    -- Use -1 for infinite wait
    -- Returns: Event if received, Void on timeout
```

### close
```eiffel
close
    ensure
        closed: handle = default_pointer
```

## SIMPLE_WATCH_EVENT Contracts

### make
```eiffel
make (a_type: INTEGER; a_filename: STRING_8)
    -- Post: event_type = a_type, filename = a_filename
```

## Class Invariants

### SIMPLE_WATCHER
- None explicitly defined
- Implicit: handle valid when is_valid = True

### SIMPLE_WATCH_EVENT
- None explicitly defined
- Implicit: event_type in 1..4
