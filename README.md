<p align="center">
  <img src="https://raw.githubusercontent.com/simple-eiffel/claude_eiffel_op_docs/main/artwork/LOGO.png" alt="simple_ library logo" width="400">
</p>

# SIMPLE_WATCHER

**[Documentation](https://simple-eiffel.github.io/simple_watcher/)**

### File System Watcher Library for Eiffel

[![Language](https://img.shields.io/badge/language-Eiffel-blue.svg)](https://www.eiffel.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)]()
[![SCOOP](https://img.shields.io/badge/SCOOP-compatible-orange.svg)]()
[![Design by Contract](https://img.shields.io/badge/DbC-enforced-orange.svg)]()
[![Tests](https://img.shields.io/badge/tests-9%20passing-brightgreen.svg)]()

---

## Overview

SIMPLE_WATCHER provides SCOOP-compatible file system change monitoring for Eiffel applications. It wraps the Win32 ReadDirectoryChangesW API through a clean C interface, enabling real-time detection of file system changes without threading complications.

The library monitors directories for file additions, deletions, modifications, and renames, with configurable watch flags for filtering specific change types. Supports both single-directory and recursive subdirectory monitoring.

**Developed using AI-assisted methodology:** Built interactively with Claude Opus 4.5 following rigorous Design by Contract principles.

---

## Features

### Watch Capabilities

- **File Changes** - Detect file additions, deletions, modifications
- **Renames** - Track file and directory rename operations
- **Recursive** - Optional monitoring of subdirectories
- **Polling** - Non-blocking event retrieval

### Watch Flags

| Flag | Value | Description |
|------|-------|-------------|
| `Watch_file_name` | 0x0001 | File creation/deletion/rename |
| `Watch_dir_name` | 0x0002 | Directory creation/deletion/rename |
| `Watch_attributes` | 0x0004 | Attribute changes |
| `Watch_size` | 0x0008 | File size changes |
| `Watch_last_write` | 0x0010 | Last write time changes |
| `Watch_security` | 0x0020 | Security descriptor changes |
| `Watch_all` | 0x003F | All of the above |

### Event Types

| Type | Code | Description |
|------|------|-------------|
| Added | 1 | File/directory created |
| Removed | 2 | File/directory deleted |
| Modified | 3 | File/directory modified |
| Renamed | 4 | File/directory renamed |

---

## Quick Start

### Installation

1. Clone the repository:
```bash
git clone https://github.com/simple-eiffel/simple_watcher.git
```

2. Compile the C library:
```bash
cd simple_watcher/Clib
compile.bat
```

3. Set the ecosystem environment variable (one-time setup for all simple_* libraries):
```bash
set SIMPLE_EIFFEL=D:\prod
```

4. Add to your ECF file:
```xml
<library name="simple_watcher" location="$SIMPLE_EIFFEL/simple_watcher/simple_watcher.ecf"/>
```

### Basic Usage

```eiffel
class
    MY_APPLICATION

feature

    watch_directory
        local
            watcher: SIMPLE_WATCHER
            event: detachable SIMPLE_WATCH_EVENT
        do
            -- Create watcher for a directory
            create watcher.make ("C:\MyFolder", False, {SIMPLE_WATCHER}.Watch_all)

            if watcher.is_valid then
                watcher.start

                from
                until not watcher.is_watching
                loop
                    -- Poll for changes (non-blocking)
                    event := watcher.poll

                    if attached event as e then
                        print ("Event: " + e.event_type_string + " - " + e.filename + "%N")

                        if e.is_added then
                            print ("  File was added%N")
                        elseif e.is_removed then
                            print ("  File was removed%N")
                        elseif e.is_modified then
                            print ("  File was modified%N")
                        elseif e.is_renamed then
                            print ("  File was renamed%N")
                        end
                    end

                    -- Small delay to avoid busy-waiting
                    sleep (100)
                end

                watcher.close
            end
        end

    watch_recursive
        local
            watcher: SIMPLE_WATCHER
        do
            -- Watch directory and all subdirectories
            create watcher.make ("C:\MyFolder", True, {SIMPLE_WATCHER}.Watch_all)

            if watcher.is_valid then
                watcher.start
                -- ... monitoring loop ...
                watcher.close
            end
        end

end
```

---

## API Reference

### SIMPLE_WATCHER Class

#### Creation

```eiffel
make (a_path: STRING_8; a_recursive: BOOLEAN; a_flags: INTEGER)
    -- Create watcher for directory at a_path.
    -- If a_recursive, watch subdirectories too.
    -- a_flags specifies which changes to monitor.
```

#### Control

```eiffel
start
    -- Begin watching for changes.

close
    -- Stop watching and release resources.
```

#### Event Retrieval

```eiffel
poll: detachable SIMPLE_WATCH_EVENT
    -- Poll for next change event (non-blocking).
    -- Returns Void if no event available.

wait: detachable SIMPLE_WATCH_EVENT
    -- Wait for next change event (blocking).
```

#### Status Queries

```eiffel
is_valid: BOOLEAN
    -- Is the watcher properly initialized?

is_watching: BOOLEAN
    -- Is the watcher actively monitoring?

last_start_succeeded: BOOLEAN
    -- Did the last start operation succeed?

watch_path: STRING_8
    -- Path being watched.
```

### SIMPLE_WATCH_EVENT Class

#### Access

```eiffel
event_type: INTEGER
    -- Type of change (1-4).

filename: STRING_8
    -- Name of affected file/directory.
```

#### Status

```eiffel
is_added: BOOLEAN
    -- Was a file/directory created?

is_removed: BOOLEAN
    -- Was a file/directory deleted?

is_modified: BOOLEAN
    -- Was a file/directory modified?

is_renamed: BOOLEAN
    -- Was a file/directory renamed?

event_type_string: STRING_8
    -- Human-readable event type ("added", "removed", etc.)
```

---

## Building & Testing

### Build Library

```bash
cd simple_watcher
ec -config simple_watcher.ecf -target simple_watcher -c_compile
```

### Run Tests

```bash
ec -config simple_watcher.ecf -target simple_watcher_tests -c_compile
./EIFGENs/simple_watcher_tests/W_code/simple_watcher.exe
```

**Test Results:** 9 tests passing

---

## Project Structure

```
simple_watcher/
├── Clib/                       # C wrapper library
│   ├── simple_watcher.h        # C header file
│   ├── simple_watcher.c        # C implementation
│   └── compile.bat             # Build script
├── src/                        # Eiffel source
│   ├── simple_watcher.e        # Main watcher class
│   └── simple_watch_event.e    # Event class
├── testing/                    # Test suite
│   ├── application.e           # Test runner
│   └── test_simple_watcher.e   # Test cases
├── simple_watcher.ecf          # Library configuration
├── README.md                   # This file
└── LICENSE                     # MIT License
```

---

## Dependencies

- **Windows OS** - ReadDirectoryChangesW is Windows-specific
- **EiffelStudio 23.09+** - Development environment
- **Visual Studio C++ Build Tools** - For compiling C wrapper

---

## SCOOP Compatibility

SIMPLE_WATCHER is fully SCOOP-compatible. The C wrapper handles all Win32 API calls synchronously without threading dependencies, making it safe for use in concurrent Eiffel applications.

---

## License

MIT License - see [LICENSE](LICENSE) file for details.

---

## Contact

- **Author:** Larry Rix
- **Repository:** https://github.com/simple-eiffel/simple_watcher
- **Issues:** https://github.com/simple-eiffel/simple_watcher/issues

---

## Acknowledgments

- Built with Claude Opus 4.5 (Anthropic)
- Uses Win32 ReadDirectoryChangesW API (Microsoft)
- Part of the simple_ library collection for Eiffel
