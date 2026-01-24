# S01 PROJECT INVENTORY - simple_watcher

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## Project Structure

```
simple_watcher/
  simple_watcher.ecf        # Eiffel configuration file
  src/
    simple_watcher.e        # Main watcher class
    simple_watch_event.e    # Event data class
  c/
    simple_watcher.h        # C header (Win32 interface)
    simple_watcher.c        # C implementation
  testing/
    test_app.e              # Test application entry
    lib_tests.e             # Test cases
  research/                 # 7S research documents
  specs/                    # Specification documents
```

## Source Files

| File | Type | Lines | Description |
|------|------|-------|-------------|
| simple_watcher.e | Class | ~280 | Directory watcher |
| simple_watch_event.e | Class | ~70 | Change event data |

## Test Files

| File | Type | Tests | Description |
|------|------|-------|-------------|
| test_app.e | Root | - | Test application entry point |
| lib_tests.e | Tests | TBD | Library test cases |

## Dependencies

### Internal (simple_* ecosystem)
- None (standalone)

### External
- Win32 API (ReadDirectoryChangesW)
- C compiler (for inline C)

### Standard Library
- base (C_STRING, POINTER)

## Build Targets

| Target | Type | Description |
|--------|------|-------------|
| simple_watcher | Library | Main library target |
| simple_watcher_tests | Executable | Test runner |
