# 어린왕자: 장미의 별 — Context

This glossary defines the domain language for the 1.44MB GAME_DEV CONTEST project. It is intentionally about game concepts, not implementation details. The world is a pure *Le Petit Prince* fable; retro pixel styling is a visual layer only (D049).

## Language

**어린왕자: 장미의 별** (Le Petit Prince: The Rose's Star):
The title of the fixed-path micro tower-defense game. Working English handle: _The Rose's Star_.
_Avoid_: Floppy Defense, Sector Defense, Bytewall, Disk Guard

**달** (Moon):
A group of eleven sequential **해넘이** sharing one Act. There are exactly four 달, and each 달 equals one Act whose dominant baobab growth stage rises by one (달1 싹 / 달2 새싹 / 달3 묘목 / 달4 거목).
_Avoid_: World, chapter, Disk

**해넘이** (Sunset):
One playable stage — surviving one day until sunset. There are 44 해넘이 total (4 달 × 11). Echoes the original "I have watched the sunset forty-four times" and the 1.44MB rhyme.
_Avoid_: Sector, level, mission

**Stage**:
The generic planning term for one short playable challenge, equivalent to one **해넘이** when in-world precision is not needed.
_Avoid_: Board, round

**Easy Mode**:
A mode that grants passive **별빛** income over time.
_Avoid_: Normal, casual

**Hard Mode**:
A mode that removes passive **별빛** income and uses a distinct high-contrast game palette.
_Avoid_: Expert, nightmare

**별빛** (Starlight):
The spendable resource used to summon and upgrade **Towers**. Earned by defeating **Enemies** and as a per-Stage starting grant. Lore: defeating an enemy releases trapped starlight back to the sky, and that starlight calls the guardians.
_Avoid_: Gold, money, cash, mana, BYTES

**장미 꽃잎** (Rose Petal):
Life. Each baobab that reaches the **🌹장미** (the rose at the planet's core) costs one petal; losing the last petal is defeat.
_Avoid_: HP, lives, FREE BYTES

**🌹장미** (Rose):
The single rose on B-612 that the player defends. The emotional core of the fable ("you become responsible, forever, for what you have tamed").

**바오밥** (Baobab):
The primary **Enemy**. Grows in four stages (🌱싹 → 🌿새싹 → 🪴묘목 → 🌳거목); higher stages are tankier and slower with greater reward. The original lesson "a baobab must be pulled while small" expressed as a growth curve.
_Avoid_: Monster, creep, CORRUPT BYTES

**Path**:
The fixed route enemies follow from entry to the rose.
_Avoid_: Road, lane, track

**Build Zone**:
A tile region where Towers may be placed.
_Avoid_: Build area, tower tile, plot

**Tower**:
A player-summoned guardian that attacks, slows, or supports along the Path. Five kinds: 🐑양 / 🌋화산 / 💡가로등 / 🏜️사막바람 / 🦊여우 (D054).
_Avoid_: Turret, building, unit

**Enemy**:
An advancing threat that follows the Path and costs a 장미 꽃잎 if it reaches the rose. Eight kinds: four baobab stages + 🐍뱀 / 🎩허영쟁이 / 💼사업가 / 🍷술꾼 (D053).
_Avoid_: Monster, creep, mob

**Wave**:
A scheduled group of enemies spawned during a 해넘이.
_Avoid_: Round, spawn batch

**Record Code**:
A short text proof of a clear result, mode, stage, and score-like state.
_Avoid_: Leaderboard entry, save code

**Share Card**:
A generated 1080x1080 BMP image summarizing a clear result for external sharing.
_Avoid_: Screenshot, poster

## Bonus Modes

**Endless** — 🏮점등인의 별 (Lamplighter's Star):
Fixed-path map with infinite scaling waves; survived wave count is the score. Framed as the lamplighter's planet of endless repetition. Unlocked by clearing the 22nd 해넘이 (end of 달2).

**Random** — 🏜️사막의 신기루 (Desert Mirage):
Procedurally generated map (path + build zones) each run. Framed as a desert path that rises in a new shape every time. Unlocked by clearing the 33rd 해넘이 (end of 달3).

## Relationships

- A **달** contains exactly eleven **해넘이**; there are four **달** → **44 해넘이** total.
- A **달** equals one Act, whose dominant **바오밥** growth stage is fixed (달1 싹 / 달2 새싹 / 달3 묘목 / 달4 거목).
- A **해넘이** is one **Stage**. The 44th 해넘이 is the giant 거목 boss.
- A **Stage** has one **Path**, one or more **Build Zones**, and one or more **Waves**.
- **Easy Mode** and **Hard Mode** share the same **Stage** data.
- **별빛** is spent on **Towers**; a **장미 꽃잎** is lost when an **Enemy** reaches the rose.
- A clear result may produce one **Record Code** and one **Share Card**.

## Example Dialogue

> **Dev:** "Should 달3 introduce a new Tower?"
> **Domain expert:** "Yes. 달3 (묘목) teaches 🏜️사막바람 against tankier baobabs, but each 해넘이 should still focus on one idea."

## Flagged Ambiguities

- "Level", "stage", and "sector" were used interchangeably in the old digital framing. Resolution: use **해넘이** for the in-world 4×11 structure and **Stage** for generic planning language.
- "Money", "gold", "resource", and the old "BYTES" all mean **별빛** in player-facing language.
- The old digital lore (Floppy/Disk/Sector/BYTES/Glitch) is retired (D049). Only the 1.44MB exact-fill size target (D004) survives as a technical constraint, not as narrative.
