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
; ── test1: RS-1-1-E 정상 round-trip ─────────────────
        stdcall encode_record, 1, 1, 0, 5, 3, 0
        lea     esi, [record_code_buf]
        lea     edi, [code_input_buf]
        mov     ecx, 20
        rep     movsb
        stdcall decode_record
        mov     [statebuf + 0], eax         ; 기대: 0  (level_idx)
        mov     eax, [game_mode]
        mov     [statebuf + 4], eax         ; 기대: 0  (Easy)
; ── test2: RS-4-11-H 정상 round-trip ────────────────
        stdcall encode_record, 4, 11, 1, 3, 5, 2
        lea     esi, [record_code_buf]
        lea     edi, [code_input_buf]
        mov     ecx, 20
        rep     movsb
        stdcall decode_record
        mov     [statebuf + 8], eax         ; 기대: 43 (level_idx)
        mov     eax, [game_mode]
        mov     [statebuf + 12], eax        ; 기대: 1  (Hard)
; ── test3: 형식 올바르나 체크 틀린 코드 → 실패 ────────
; "RS-1-1-E-0000": check_stored=0, check_calc=(0+0+0+1+1)&7=2 → 불일치
        mov     byte [code_input_buf+0],  'R'
        mov     byte [code_input_buf+1],  'S'
        mov     byte [code_input_buf+2],  '-'
        mov     byte [code_input_buf+3],  '1'
        mov     byte [code_input_buf+4],  '-'
        mov     byte [code_input_buf+5],  '1'
        mov     byte [code_input_buf+6],  '-'
        mov     byte [code_input_buf+7],  'E'
        mov     byte [code_input_buf+8],  '-'
        mov     byte [code_input_buf+9],  '0'
        mov     byte [code_input_buf+10], '0'
        mov     byte [code_input_buf+11], '0'
        mov     byte [code_input_buf+12], '0'
        mov     byte [code_input_buf+13], 0
        stdcall decode_record
        mov     [statebuf + 16], eax        ; 기대: -1
; ── test4: 형식 오류 (첫 바이트가 'R' 아님) → 실패 ───
        mov     byte [code_input_buf+0], 'A'
        stdcall decode_record
        mov     [statebuf + 20], eax        ; 기대: -1
; ── 결과 덤프 후 exit ────────────────────────────────
        mov     eax, 4
        mov     ebx, 1
        mov     ecx, statebuf
        mov     edx, 24
        int     0x80
        mov     eax, 1
        xor     ebx, ebx
        int     0x80
