# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-12-04

### Added
- Initial public release
- MutexGuard class for standard mutex RAII management
- RecursiveMutexGuard class for recursive mutex support
- Automatic mutex locking and unlocking using RAII pattern
- Timeout support for lock acquisition with `hasLock()` verification
- ISR context detection and protection
- Null handle protection
- Thread-safe operations
- Non-copyable, non-movable design for safe ownership
- Optional debug logging with MUTEXGUARD_DEBUG macro

### Notes
- Production-tested in multi-threaded FreeRTOS environment
- Used extensively for mutex management in complex embedded systems
- Previous internal versions (v1.x, v2.x) not publicly released
- Reset to v0.1.0 for clean public release start
