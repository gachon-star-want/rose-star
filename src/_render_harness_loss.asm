; LOSS 시나리오 래퍼 — 타워 0 → 8마리 전부 누수 → 꽃잎 소진으로 패배 검증.
;   기대: game_state=LOST(2), petals≤0, starlight=60(미사용), active_count=0.
format ELF executable 3
entry _start
LEVEL_IDX = 0
SCENARIO  = 1
SIM_TICKS = 850
include '_render_harness_body.inc'
