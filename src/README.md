# src/ — 게임 소스 (FASM)

「어린왕자: 장미의 별」의 어셈블리 소스를 둔다(Windows x86 PE, 무엔진).

- 진입점: `src/main.asm` — Win32 PE 빈 창 + 256x192 프레임버퍼(이슈 03/04). 아직 미구현.
- 빌드: 루트에서 `make build` (프로젝트 로컬 FASM 필요, `tools/toolchain/`).
- 용어·수치·문자열은 `CONTEXT.md` 글로서리와 `docs/`(특히 03/09/10/11/12)를 단일 출처로 따른다.
