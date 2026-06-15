; boost micro-harness: TW_FOX BOOST 체비셰프 1타일 단언
; FOX(col5,row5), SHEEP(col6,row5) 1타일 → boosted=1
; LAMP(col10,row5) 5타일 → boosted=0
; FOX_IDX=5*16+5=85, SHEEP_IDX=86, LAMP_IDX=90
format ELF executable 3
entry _start

include 'macro/proc32.inc'
include 'game_defs.inc'

segment readable writeable
  framebuffer  rd FB_W * FB_H
  tilemap      rb GRID_COLS * GRID_ROWS
  tower_grid   rb GRID_COLS * GRID_ROWS
  align 4
  starlight    dd ?
  in_cursor_cx dd ?
  in_cursor_cy dd ?
  in_place     dd ?
  in_sell      dd ?
  r_cx         dd ?
  r_cy         dd ?
  r_px         dd ?
  r_py         dd ?
  r_tx         dd ?
  r_ty         dd ?
  en_active    rb MAX_ENEMIES
  en_type      rb MAX_ENEMIES
  en_lane      rb MAX_ENEMIES
  en_aura_buff rb MAX_ENEMIES
  align 4
  en_pos       rd MAX_ENEMIES
  en_hp        rd MAX_ENEMIES
  en_x         rd MAX_ENEMIES
  en_y         rd MAX_ENEMIES
  en_slow_timer rd MAX_ENEMIES
  tower_cd     rd GRID_COLS * GRID_ROWS
  tower_boosted rb GRID_COLS * GRID_ROWS
  align 4
  seg_x1       rd MAX_PATH_SEGS
  seg_y1       rd MAX_PATH_SEGS
  seg_dx       rd MAX_PATH_SEGS
  seg_dy       rd MAX_PATH_SEGS
  seg_acc      rd MAX_PATH_SEGS
  seg_len      rd MAX_PATH_SEGS
  seg_count    dd ?
  path_end_px  dd ?
  wave_types   rb MAX_WAVES
  wave_counts  rb MAX_WAVES
  wave_spacings rb MAX_WAVES
  align 4
  wave_delays  rd MAX_WAVES
  wave_total   dd ?
  wave_cur     dd ?
  wave_timer   dd ?
  wave_pending dd ?
  wave_scd     dd ?
  game_mode    dd ?
  easy_income_cd dd ?
  prng_state   dd ?
  in_callwave  dd ?
  boss_phase   dd ?
  cur_tower    dd TW_SHEEP
  num_buf      dw 8 dup(0)
  record_code_buf db 20 dup(0)
  code_input_buf  db 20 dup(0)
  code_input_len  dd 0
  code_input_err  dd 0
  to_spawn     dd ?
  spawn_cd     dd ?
  active_count dd ?
  petals       dd ?
  game_state   dd ?
  statebuf     rd 6

FOX_IDX   = 5 * GRID_COLS + 5    ; (col5, row5) = 85
SHEEP_IDX = 5 * GRID_COLS + 6    ; (col6, row5) = 86  체비셰프=1 ≤ 1 → boosted
LAMP_IDX  = 5 * GRID_COLS + 10   ; (col10,row5) = 90  체비셰프=5 > 1 → not boosted

segment readable executable
include 'game.inc'
include 'font_data.inc'
include 'level_data.inc'

_start:
        mov     byte [tower_grid + FOX_IDX],   TW_FOX
        mov     byte [tower_grid + SHEEP_IDX], TW_SHEEP
        mov     byte [tower_grid + LAMP_IDX],  TW_LAMP

        stdcall boost_recompute

; statebuf[0]=tower_boosted[SHEEP_IDX] 기대 1
        movzx   eax, byte [tower_boosted + SHEEP_IDX]
        mov     [statebuf + 0], eax
; statebuf[1]=tower_boosted[LAMP_IDX] 기대 0
        movzx   eax, byte [tower_boosted + LAMP_IDX]
        mov     [statebuf + 4], eax

        mov     eax, 4
        mov     ebx, 1
        mov     ecx, statebuf
        mov     edx, 24
        int     0x80
        mov     eax, 1
        xor     ebx, ebx
        int     0x80
