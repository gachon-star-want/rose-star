# Tracer Bullet PRD

Status: ready-for-agent

## Goal

Build the smallest end-to-end playable proof that 「어린왕자: 장미의 별」 can become a Windows x86 PE `.exe`: a local build skeleton, a 256x192 framebuffer window, keyboard+mouse input, and one 해넘이(Sunset) with a minimal tower-defense loop.

## Non-Goals

- Do not build all 44 해넘이.
- Do not add audio.
- Do not add Record Code or Share Card generation.
- Do not optimize exact-fill release size yet.

## Scope

This PRD covers the first implementation slice only:

- Project Skeleton
- Win32 Framebuffer Tracer Bullet
- Playable One-Sunset Loop (해넘이 1-1)

The work is tracked as local markdown issues under `.scratch/tracer-bullet/issues/`.
용어·수치·문자열은 `CONTEXT.md` + `docs/`(03/09/10/11/12)를 단일 출처로 따른다.

## Success Criteria

- `make build` has a clear path to producing `dist/rose-star.exe`.
- `make size` reports `dist/` byte totals against the 1,474,560-byte limit.
- A Windows x86 PE executable displays a 256x192 internal framebuffer.
- Keyboard/mouse can move a cursor and exit with `Esc`.
- 해넘이 1-1 can be played with one Tower(🐑양), one Enemy(🌱싹), one Path, 별빛 spending, 장미 꽃잎(생명), and a win/fail/restart loop.
