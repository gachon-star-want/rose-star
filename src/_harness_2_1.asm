; 2-1 WIN: T0 새싹×8 sp48, start=90
; 화산(AoE) + 가로등 조합: 화산 col5 row4(AoE 반경1), 가로등 col10 row4(range4)
; 별빛: 90 - 40(화산) - 45(가로등) = 5 + 처치보상(새싹×8=16) = 21
; → 실제는 하네스 결과로 확인
format ELF executable 3
entry _start
LEVEL_IDX = 11        ; level_table[11] = level_2_1
SCENARIO  = 0
SIM_TICKS = 1200
include '_render_harness_body.inc'
