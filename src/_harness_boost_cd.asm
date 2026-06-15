; boost-cd 하네스: TW_SHEEP boosted → CD=17, normal → CD=20
; FOX(col5,row5)+SHEEP_B(col6,row5) → boosted, SHEEP_N(col0,row0) → normal
; CD 계산: 20 * 218 >> 8 = 4360 >> 8 = 17
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

FOX_IDX  = 5 * GRID_COLS + 5    ; row5 col5 = 85
SHEEP_B  = 5 * GRID_COLS + 6    ; row5 col6 = 86  FOX 1타일 내 → boosted
SHEEP_N  = 0                     ; row0 col0 = 0   → normal (FOX와 5타일 이상 거리)

segment readable executable
include 'game.inc'
include 'font_data.inc'
include 'level_data.inc'

_start:
        mov     byte [tower_grid + FOX_IDX],  TW_FOX
        mov     byte [tower_grid + SHEEP_B],  TW_SHEEP
        mov     byte [tower_grid + SHEEP_N],  TW_SHEEP

        stdcall boost_recompute
        stdcall fire_tick

; statebuf[0] = tower_cd[SHEEP_B] 기대 17  (20*218>>8)
        mov     eax, [tower_cd + SHEEP_B*4]
        mov     [statebuf + 0], eax
; statebuf[1] = tower_cd[SHEEP_N] 기대 20  (부스트 없음)
        mov     eax, [tower_cd + SHEEP_N*4]
        mov     [statebuf + 4], eax

        mov     eax, 4
        mov     ebx, 1
        mov     ecx, statebuf
        mov     edx, 24
        int     0x80
        mov     eax, 1
        xor     ebx, ebx
        int     0x80
