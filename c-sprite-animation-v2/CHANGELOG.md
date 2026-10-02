# Changelog

All notable changes to this project are documented in this file. The project follows [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Planned

- Signal handlers for client connection requests
- FIFO creation, acknowledgement, and cleanup
- Client/server command protocol
- Multi-client integration tests
- Portable server dependency builds

## [0.2.0-alpha] - 2026-10-02

### Added

- Separate client and server entry points
- Server integration with an external `libanimate` static library
- Server canvas lifecycle smoke test
- Build targets for the engine, examples, client, and server
- Client–server architecture and status documentation

### Changed

- The default build now compiles portable engine examples and the client.
- Platform-specific server linking is an explicit `make server` step.

### Known incomplete work

- Client communication logic
- Signal registration and handling
- FIFO creation and cleanup
- Server acknowledgement and message processing

## [0.1.0] - 2026-10-02

### Added

- Canvas creation and destruction
- Programmatic rectangle and circle sprites
- Bitmap V5 sprite loading and input validation
- Reference-counted sprite ownership checks
- Multiple placements and layer reordering
- Velocity- and acceleration-based movement
- Transparent-pixel handling and canvas clipping
- Raw ARGB32 frame generation
- Basic and extended examples
