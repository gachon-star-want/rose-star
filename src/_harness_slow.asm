; slow/decay micro-harness
; EN_SEEDLING (speed=128), slow_timer=10 → move_tick 1틱 → pos=64
; decay_tick 1틱 → slow_timer=9
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

segment readable executable
include 'game.inc'
include 'font_data.inc'
include 'level_data.inc'

_start:
; 경로: 수평 1세그, 충분히 길게
        mov     dword [path_end_px], 0x7FFF
        mov     dword [seg_count],   1
        mov     dword [seg_x1],      0
        mov     dword [seg_y1],      0
        mov     dword [seg_dx],      1
        mov     dword [seg_dy],      0
        mov     dword [seg_acc],     0
        mov     dword [seg_len],     0x7FFF

; slot0: EN_SEEDLING (speed=128), SLOW 상태
        mov     byte  [en_active + 0], 1
        mov     byte  [en_type   + 0], EN_SEEDLING
        mov     dword [en_pos    + 0*4], 0
        mov     dword [en_slow_timer + 0*4], 10

        mov     dword [active_count], 1
        mov     dword [petals], 5

; move_tick 1틱: 128 * 128 / 256 = 64 → en_pos[0]=64
        stdcall move_tick

; decay_tick 1틱: slow_timer[0] = 10-1 = 9
        stdcall decay_tick

; statebuf[0]=en_pos[0] 기대 64
        mov     eax, [en_pos + 0*4]
        mov     [statebuf + 0], eax
; statebuf[1]=en_slow_timer[0] 기대 9
        mov     eax, [en_slow_timer + 0*4]
        mov     [statebuf + 4], eax

        mov     eax, 4
        mov     ebx, 1
        mov     ecx, statebuf
        mov     edx, 24
        int     0x80
        mov     eax, 1
        xor     ebx, ebx
        int     0x80
