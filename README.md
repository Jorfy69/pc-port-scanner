# PC Port Scanner

A lightweight Windows utility written in C that scans your PC's active network connections using the Windows API.

The program retrieves network connection information and separates the results into two categories:

- **TCP:** Displays active TCP connections.
- **UDP:** Displays UDP endpoints.

## Requirements

- Windows operating system
- GCC compiler (MinGW recommended)
- Windows SDK headers and libraries

### Required Headers

The project uses the following Windows API headers:

- `iphlpapi.h` – Retrieves TCP and UDP connection information.
- `psapi.h` – Provides access to process-related information.

## Compilation

This project has currently only been tested with GCC.

To compile, run:

```bash
gcc main.c -o network -lws2_32 -liphlpapi -lpsapi
```

### Linked Libraries

| Library | Purpose |
|---|---|
| `ws2_32` | Windows Sockets API |
| `iphlpapi` | IP Helper API for network information |
| `psapi` | Process Status API |

## Platform Compatibility

Currently designed for Windows and relies on Windows-specific APIs. Linux and macOS are not supported.
