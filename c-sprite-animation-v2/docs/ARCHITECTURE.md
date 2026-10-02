# Version 2 Architecture

## Goal

Version 2 is intended to move animation operations behind a server process. A client will establish a connection using Unix signals and communicate through a named pipe (FIFO).

## Current implementation

- `animate_client.c` provides a compilable process entry point.
- `animate_server.c` links to `libanimate`, creates a canvas, and destroys it.
- The Makefile validates that the external library header and archive exist before linking the server.
- Connection establishment and application messages are not implemented yet.

## Intended connection sequence

1. Start the server and make its process ID available to clients.
2. A client sends a documented connection-request signal to the server.
3. The server creates a unique FIFO for that client.
4. The server sends an acknowledgement signal.
5. The client opens the FIFO and begins exchanging protocol messages.
6. Both processes close descriptors and remove the FIFO on completion or failure.

## Decisions still required

The source does not yet define these details, so contributors should agree on them before implementation:

- Which Unix signals represent connection and acknowledgement
- How the client identifies itself and discovers the FIFO path
- Whether communication uses one FIFO or separate request/response FIFOs
- The binary or text message format
- Message framing and maximum payload sizes
- Timeout, retry, interruption, and malformed-input behaviour
- How concurrent clients are isolated
- Which process owns FIFO deletion
- How server shutdown notifies connected clients

## Safety considerations

- Signal handlers should perform only async-signal-safe operations.
- Shared state modified by a handler should use appropriate `volatile sig_atomic_t` flags.
- FIFO names must not trust unchecked client input.
- FIFO creation should reject existing paths and use restrictive permissions.
- Every error path should close file descriptors and remove owned FIFOs.
- Protocol messages must be length-checked before allocation or processing.

## External dependency

The supplied `libanimate.a` contains ELF AArch64 objects. It is suitable only for a compatible 64-bit ARM Linux toolchain. The external dependency stays outside version control by default so platform-specific binaries and potentially restricted course resources are not accidentally published.
