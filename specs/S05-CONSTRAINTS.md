# S05 CONSTRAINTS - simple_watcher

**BACKWASH** - Generated: 2026-01-23
**Library**: simple_watcher

## Technical Constraints

### Platform
- **Target OS**: Windows only
- **API**: Win32 ReadDirectoryChangesW
- **Compiler**: EiffelStudio 25.02+

### Path Requirements
- Valid directory path
- Must have read permissions
- MAX_PATH (260 chars) or long path support

### Recursion
- Optional via make parameter
- No depth limit (Win32 handles)

## Operational Constraints

### Event Delivery
- Events delivered asynchronously
- One event per poll/wait call
- Events may be coalesced by OS
- Rapid changes may be missed (buffer overflow)

### Timing
| Scenario | Behavior |
|----------|----------|
| poll (no event) | Returns immediately with Void |
| poll (event) | Returns immediately with event |
| wait (timeout=0) | Same as poll |
| wait (timeout=-1) | Block forever |
| wait (timeout>0) | Block up to timeout ms |

## Performance Constraints

| Operation | Expected Time |
|-----------|--------------|
| make | 1-5ms |
| start | 1-10ms |
| poll | < 1ms |
| close | 1-5ms |

## Resource Constraints

| Resource | Limit |
|----------|-------|
| Max watchers | OS handle limit (~10K) |
| Buffer size | 64KB typical |
| Path length | MAX_PATH (260) |

## Lifecycle Constraints

### Required Sequence
1. make (creates handle)
2. start (begins watching)
3. poll/wait loop (process events)
4. close (releases handle)

### Error Handling
- Invalid path: is_valid = False after make
- Permission denied: start fails, check last_error
- Closed watcher: Operations require is_valid
