# Changelog

All notable changes to this project will be documented in this file.

## [1.2.0] - 2025-05-22

### Fixed
- **Critical Crash (LoadProhibited):** Fixed an out-of-bounds memory access in `transmitUDP` caused by a parameter mismatch in the `printf` formatting string for RFC 3164 protocol.
- **Protocol Dispatch:** Refactored `transmitUDP` to use a `switch` statement based on the protocol version. This ensures that the correct number of arguments is passed to `printf` for each protocol (RFC 3164 vs RFC 5424), preventing potential memory corruption.
- **Safety:** Added a `default` case in the protocol dispatcher to prevent system crashes if an undefined protocol value is provided.
- **Memory Safety:** Initialized internal class buffers in the constructor to avoid accessing uninitialized pointers (wild pointers) if logs are sent before full initialization.

### Improved
- **Maintainability:** Improved internal documentation and structure of the `transmitUDP` method for better scalability when adding new protocols.
- **Documentation:** Added comprehensive API reference in `docs/API.md` for better developer experience.


