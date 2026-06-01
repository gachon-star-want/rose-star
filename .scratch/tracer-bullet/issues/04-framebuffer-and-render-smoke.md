# 256x192 Framebuffer Render Smoke

Status: ready-for-agent

## What to build

Add a 256x192 (4:3) internal framebuffer to the minimal Win32 executable and display it window-only at integer scale (default 4x = 1024x768, D046). The output should visibly prove that frames are being drawn and updated, without requiring game art yet.

## Acceptance criteria

- [ ] The program maintains a 256x192 internal framebuffer.
- [ ] The framebuffer is displayed in the Win32 window at an integer-scaled size (window-only, no fullscreen).
- [ ] A visible frame counter, color cycle, or deterministic animation proves frames are advancing.
- [ ] Rendering does not require external image files.
- [ ] `make build` and `make size` still work after the framebuffer path is added.

## Blocked by

- `.scratch/tracer-bullet/issues/03-minimal-win32-pe-window.md`
