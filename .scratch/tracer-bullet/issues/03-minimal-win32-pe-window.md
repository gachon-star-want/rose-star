# Minimal Windows x86 PE Window

Status: ready-for-agent

## What to build

Create the smallest useful 「어린왕자: 장미의 별」 Windows x86 PE executable: a Win32 window that opens, stays alive, and exits on `Esc` or close. This proves the executable format, assembler path, and platform loop before game logic is added.

## Acceptance criteria

- [ ] `make build` attempts to assemble a Windows x86 PE executable from `src/`.
- [ ] A successful build writes `dist/rose-star.exe`.
- [ ] Running the executable on a compatible Windows/Wine environment opens a window within 2 seconds.
- [ ] The window remains responsive for at least 60 seconds.
- [ ] `Esc` and the window close button exit cleanly.
- [ ] The built executable size is recorded in build output or a generated size report.

## Blocked by

- `.scratch/tracer-bullet/issues/02-fasm-toolchain-path.md`
