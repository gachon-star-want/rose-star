# Project-Local FASM Toolchain Path

Status: ready-for-agent

## What to build

Make the build harness use a project-local FASM path in a reproducible way. This issue does not require downloading FASM if network or platform constraints block it, but it must make the expected path, validation behavior, and failure mode concrete.

## Acceptance criteria

- [ ] The build harness has one canonical FASM path at `tools/toolchain/fasm` (Makefile `FASM` 변수).
- [ ] `make toolchain-check` prints the FASM path it intends to use.
- [ ] If FASM is absent, the command fails with a clear next-step message instead of continuing.
- [ ] If FASM is present, the command prints enough identifying information to compare against `tools/toolchain/README.md`.
- [ ] The behavior is documented so macOS development and Windows/Wine validation can use the same project standard.

## Blocked by

- `.scratch/tracer-bullet/issues/01-project-skeleton.md`
