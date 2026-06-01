# 해넘이 1-1 Playable Loop

Status: ready-for-agent

## What to build

Create the first complete playable 해넘이: one fixed Path to the 🌹장미, one Build Zone layout, one 🐑양 Tower, one 🌱싹 Enemy type, a short Wave, 별빛 spending, 장미 꽃잎(생명) loss, win/fail states, and restart. This is the first true vertical slice of 「어린왕자: 장미의 별」.

## Acceptance criteria

- [ ] 해넘이 1-1 has a fixed Path from entry to the 🌹장미.
- [ ] A Wave spawns 🌱싹 enemies that follow the Path.
- [ ] A placed 🐑양 automatically attacks enemies in range.
- [ ] 별빛 decreases when placing a Tower; a 장미 꽃잎 is lost when an Enemy reaches the 장미 (start 5 petals, D063).
- [ ] The 해넘이 can be won by defeating the Wave.
- [ ] The 해넘이 can be failed by losing all 5 꽃잎 ("별이 어두워졌다").
- [ ] `R` restarts the 해넘이 from a clean state.
- [ ] Easy Mode grants passive 별빛(+2/초) and Hard Mode does not(0), even if mode selection is temporary or debug-only (D064).

## Blocked by

- `.scratch/tracer-bullet/issues/05-cursor-and-build-zone-input.md`
