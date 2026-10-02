# Contributing

Thank you for helping improve the C Sprite Animation Engine.

## Before you begin

- Search existing issues before opening a new one.
- Keep each issue or pull request focused on one change.
- For a substantial API change, open an issue first so the design can be discussed.

## Development workflow

1. Fork the repository.
2. Create a focused branch:

   ```bash
   git switch -c feature/short-description
   ```

3. Build the portable targets:

   ```bash
   make clean
   make
   ```

4. Run both examples:

   ```bash
   make run
   make run-shapes
   ```

5. If you have a compatible external `libanimate` build, compile the server:

   ```bash
   make server
   ```

6. Commit the change with a clear message.
7. Open a pull request and explain what changed, why it changed, and how it was tested.

## Code guidelines

- Keep the public API declarations and ownership rules in `animate.h` documented.
- Check allocation and file-operation failures before using returned values.
- Release every successfully allocated resource along all error paths.
- Preserve the meaning of `canvas->bottom` and `canvas->top` when changing placement logic.
- Keep generated files and platform-specific build products out of commits.
- Keep signal handlers limited to async-signal-safe operations.
- Document the client/server message format before adding protocol commands.
- Prefer small, readable functions and descriptive names.

## Pull-request checklist

- [ ] The project builds with a C11 compiler.
- [ ] `make demos` completes successfully.
- [ ] Both example programs run successfully.
- [ ] The client builds successfully.
- [ ] Server changes were tested with a compatible `libanimate` build, or the platform limitation is stated.
- [ ] New public behavior is documented.
- [ ] Memory ownership remains clear.
- [ ] Generated binaries and `.o` files are not committed.
