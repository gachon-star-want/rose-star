; WIN 시나리오 래퍼 — 양 3마리로 8마리 전멸시켜 클리어(=한 판 성립) 검증.
;   기대: game_state=WON(1), petals=5, starlight=8(=0+8처치), active_count=0, to_spawn=0.
format ELF executable 3
entry _start
LEVEL_IDX = 0
SCENARIO  = 0
SIM_TICKS = 850
include '_render_harness_body.inc'
