; aura micro-harness: EN_VAIN AURA 체비셰프 2타일 내/외 단언
; slot0=VAIN(0,0), slot1=SEEDLING(2*TILE,0)→범위내 aura_buff=1,
; slot2=SPROUT(3*TILE+1,0)→범위외 aura_buff=0
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
  gen_buf      rb 192

segment readable executable
include 'game.inc'
include 'font_data.inc'
include 'level_data.inc'

_start:
; slot0: EN_VAIN (type=6, AURA), 위치 (0,0)
        mov     byte  [en_active + 0], 1
        mov     byte  [en_type   + 0], EN_VAIN
        mov     dword [en_x      + 0*4], 0
        mov     dword [en_y      + 0*4], 0

; slot1: EN_SEEDLING (type=2), 위치 (2*TILE, 0) = (32, 0) → 체비셰프=32 ≤ 2*TILE → aura_buff=1
        mov     byte  [en_active + 1], 1
        mov     byte  [en_type   + 1], EN_SEEDLING
        mov     dword [en_x      + 1*4], 2 * TILE
        mov     dword [en_y      + 1*4], 0

; slot2: EN_SPROUT (type=1), 위치 (3*TILE+1, 0) = (49, 0) → 체비셰프=49 > 32 → aura_buff=0
        mov     byte  [en_active + 2], 1
        mov     byte  [en_type   + 2], EN_SPROUT
        mov     dword [en_x      + 2*4], 3 * TILE + 1
        mov     dword [en_y      + 2*4], 0

        stdcall aura_recompute

; statebuf[0]=en_aura_buff[1] 기대 1 (범위 내)
        movzx   eax, byte [en_aura_buff + 1]
        mov     [statebuf + 0], eax
; statebuf[1]=en_aura_buff[2] 기대 0 (범위 외)
        movzx   eax, byte [en_aura_buff + 2]
        mov     [statebuf + 4], eax
; statebuf[2]=en_aura_buff[0] 기대 0 (VAIN 자신은 aura 안 받음)
        movzx   eax, byte [en_aura_buff + 0]
        mov     [statebuf + 8], eax

        mov     eax, 4
        mov     ebx, 1
        mov     ecx, statebuf
        mov     edx, 24
        int     0x80
        mov     eax, 1
        xor     ebx, ebx
        int     0x80
