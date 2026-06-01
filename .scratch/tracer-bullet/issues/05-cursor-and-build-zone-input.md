# Cursor And Build Zone Input

Status: ready-for-agent

## What to build

Turn the framebuffer smoke test into the first interactive 「어린왕자: 장미의 별」 surface. The player should move a cursor on the 16x11 play grid (상단 UI 1행 제외), see which tiles are Build Zones, and place or remove one 🐑양 placeholder using keyboard or mouse (D066).

## Acceptance criteria

- [ ] Arrow keys (or mouse move) move a visible cursor on the tile grid.
- [ ] Build Zones are visually distinct from Path and blocked tiles.
- [ ] The player can place a 🐑양 placeholder on a valid Build Zone (Z/Space/Enter 또는 좌클릭).
- [ ] The player cannot place a Tower on the Path or outside a Build Zone.
- [ ] `X`/Backspace 또는 우클릭 cancels/removes a placed placeholder.
- [ ] `Esc` still exits cleanly.

## Blocked by

- `.scratch/tracer-bullet/issues/04-framebuffer-and-render-smoke.md`
