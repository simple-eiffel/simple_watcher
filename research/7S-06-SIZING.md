# 7S-06 SIZING - simple_watcher

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## Codebase Metrics

### Source Files
| File | Lines | Purpose |
|------|-------|---------|
| simple_watcher.e | ~280 | Main watcher class |
| simple_watch_event.e | ~70 | Event data class |
| **Total** | ~350 | |

### Class Count
- 2 production classes
- 2 test classes (test_app.e, lib_tests.e)

## Complexity Analysis

### SIMPLE_WATCHER
- Features: ~15
- Cyclomatic complexity: Low
- Most complexity in C layer

### SIMPLE_WATCH_EVENT
- Features: ~8
- Cyclomatic complexity: Very Low
- Simple data class

## Memory Footprint

| Component | Typical Size |
|-----------|-------------|
| SIMPLE_WATCHER | ~100 bytes + C handle |
| SIMPLE_WATCH_EVENT | ~50 bytes |
| C watcher struct | ~200 bytes + buffer |
| Event buffer | 64KB typical |

## Performance Characteristics

| Operation | Expected Time |
|-----------|--------------|
| Create watcher | 1-5ms |
| Start watching | 1-10ms |
| Poll (no event) | < 1ms |
| Poll (with event) | < 1ms |
| Wait (timeout) | Up to timeout |
| Close | 1-5ms |

## Scalability

| Aspect | Limit |
|--------|-------|
| Max watchers | OS handle limit (~10000) |
| Events per poll | 1 (use loop) |
| Recursive depth | Unlimited |
| Path length | MAX_PATH (260) or long path |
