; 툴체인 스모크 테스트: FASM(Docker/linux386)이 유효한 Windows x86 PE를 방출하는지 검증용.
; 동작 게임 아님 — PE 헤더 구조(MZ / PE / i386 머신 / 섹션)만 확인한다. 실제 게임은 src/main.asm (이슈 03).
format PE GUI 4.0
entry start

section '.text' code readable executable
start:
        ret
