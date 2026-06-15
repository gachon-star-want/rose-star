; callwave 하네스: in_callwave 보너스 지급 검증
; Phase1: wave_scd=90 → starlight += 90/30=3 → starlight=13, wave_scd=0
; Phase2: wave_scd=0  → 보너스 없음, starlight=10 유지
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
; Phase 1: wave_scd=90 → starlight += 3 → 13
        mov     dword [game_state],   GS_PLAY
        mov     dword [starlight],    10
        mov     dword [wave_scd],     90
        mov     dword [in_callwave],  1
        mov     dword [petals],       5
        ; to_spawn=0, wave_pending=0, active_count=0 (BSS=0)

        stdcall update

        mov     eax, [starlight]
        mov     [statebuf + 0], eax     ; 기대 13
        mov     eax, [wave_scd]
        mov     [statebuf + 4], eax     ; 기대 0

; Phase 2: wave_scd=0 → 보너스 없음
        mov     dword [game_state],   GS_PLAY
        mov     dword [starlight],    10
        mov     dword [wave_scd],     0
        mov     dword [in_callwave],  1
        mov     dword [petals],       5

        stdcall update

        mov     eax, [starlight]
        mov     [statebuf + 8], eax     ; 기대 10

        mov     eax, 4
        mov     ebx, 1
        mov     ecx, statebuf
        mov     edx, 24
        int     0x80
        mov     eax, 1
        xor     ebx, ebx
        int     0x80
