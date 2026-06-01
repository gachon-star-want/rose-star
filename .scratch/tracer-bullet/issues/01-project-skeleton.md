# Project Skeleton And Public Build Commands

Status: done (D067 — git init + src/tools/build/dist + Makefile + tools/toolchain/README.md, make size/release-check 검증, dist/rose-star.exe)

## What to build

Create the repository skeleton needed for the first playable tracer bullet. The result should give future implementation agents a stable project surface: source directory, tool directory, build output directory, dist output directory, and Makefile entry points that match the documented build harness.

## Acceptance criteria

- [ ] The project has `src/`, `tools/`, `build/`, and `dist/` directories or tracked placeholders where appropriate.
- [ ] A top-level `Makefile` exposes at least `toolchain-check`, `build`, `size`, and `clean`.
- [ ] `make size` reports the total byte size of `dist/` and compares it against 1,474,560 bytes.
- [ ] `make toolchain-check` clearly reports that the project-local FASM toolchain is missing or valid; it must not silently use an unknown global assembler.
- [ ] `tools/toolchain/README.md` documents the expected FASM location, version/SHA256 recording policy, and that tools are excluded from the final submission.

## Blocked by

None - can start immediately
