; ─────────────────────────────────────────────────────────────────────────
; _render_harness_body.inc — 헤드리스 검증 본체 (Linux i386 ELF).
;   래퍼(_render_harness.asm / _render_harness_loss.asm)가 SCENARIO·SIM_TICKS 를
;   정의한 뒤 이 파일을 include 한다.  SCENARIO: 0=WIN(양 3마리), 1=LOSS(타워 0).
;
; game.inc 를 ELF 로 감싸 Docker(qemu-i386)에서 update() 를 SIM_TICKS 회 돌리고,
; 상태 헤더(6 dword) + framebuffer 를 stdout 으로 덤프한다.
;   make render-test / render-test-loss → tools/check-state.py + tools/fb-to-png.py
;
; 상태 헤더 순서(dword): game_state, petals, starlight, active_count, to_spawn, en_pos[0]
; ─────────────────────────────────────────────────────────────────────────
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
  ; 확장 적 필드 (Compile 1 추가 — game.inc는 아직 미사용)
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
  ; 확장 타워 필드
  tower_boosted rb GRID_COLS * GRID_ROWS
  align 4
  ; 경로 세그먼트 (Compile 2에서 사용)
  seg_x1       rd MAX_PATH_SEGS
  seg_y1       rd MAX_PATH_SEGS
  seg_dx       rd MAX_PATH_SEGS
  seg_dy       rd MAX_PATH_SEGS
  seg_acc      rd MAX_PATH_SEGS
  seg_len      rd MAX_PATH_SEGS
  seg_count    dd ?
  path_end_px  dd ?
  ; 웨이브 큐 (Compile 3에서 사용)
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
  ; 기타 상태
  game_mode    dd ?
  easy_income_cd dd ?
  prng_state   dd ?
  in_callwave  dd ?
  boss_phase   dd ?
  cur_tower       dd TW_SHEEP
  num_buf         dw 8 dup(0)
  record_code_buf db 20 dup(0)
  code_input_buf  db 20 dup(0)
  code_input_len  dd 0
  code_input_err  dd 0
  ; 원래 변수 (현재 game.inc 사용 중)
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

; 타입 지정 타워 설치 (update 없이, init 후 직접 배치)
macro place cx, cy {
        stdcall place_tower, TW_SHEEP, cx, cy
}
macro place_t type, cx, cy {
        stdcall place_tower, type, cx, cy
}

; LEVEL_IDX: 0=1-1, 1=1-2, 2=1-3, ... (각 .asm이 정의)
; SCENARIO: 0=WIN(타워 배치), 1=LOSS(타워 없음)

_start:
        stdcall init_level, LEVEL_IDX

; ── WIN 타워 배치 (SCENARIO=0) ──
; T0/T1/T5(직선·낮은직선·스위치백): row4 3마리면 전체 커버
; T2/T3/T4(꺾임): row4+row8 조합
if SCENARIO = 0
  if LEVEL_IDX = 0    ; 1-1 T0
        place   3, 4
        place   9, 4
        place   12, 8
  else if LEVEL_IDX = 1  ; 1-2 T0
        place   3, 4
        place   9, 4
        place   12, 8
  else if LEVEL_IDX = 2  ; 1-3 T2 LCORNER
        ; T2: col2~10 가로 + col10 세로 + col10~15 가로
        ; BUILD: (2,5,7,1) (6,5,4,3) (11,7,4,1)
        place   4, 5                            ; 왼쪽 가로구간 커버
        place   8, 5                            ; 중간 커버
        place   12, 7                           ; 오른쪽 커버
  else if LEVEL_IDX = 3  ; 1-4 T2 2 웨이브
        place   4, 5
        place   8, 5
        place   12, 7
  else if LEVEL_IDX = 4  ; 1-5 T0 싹×16
        place   3, 4
        place   9, 4
        place   12, 8
  else if LEVEL_IDX = 11  ; 2-1 T0 새싹×8
        ; 화산(range=2) col5 row4, 가로등(range=4) col11 row4
        place_t TW_VOLCANO, 5, 4
        place_t TW_LAMP, 11, 4
  else if LEVEL_IDX = 12  ; 2-2 T1 싹×10+새싹×4
        place   3, 7
        place_t TW_VOLCANO, 8, 7
        place   12, 7
  else if LEVEL_IDX = 13  ; 2-3 T2 새싹×10
        place_t TW_VOLCANO, 4, 5
        place   8, 5
        place_t TW_LAMP, 12, 7
  else if LEVEL_IDX = 14  ; 2-4 T3 새싹×8+싹×8
        place   2, 5
        place_t TW_VOLCANO, 5, 5
        place_t TW_LAMP, 10, 5
  else if LEVEL_IDX = 15  ; 2-5 T4 새싹×6+싹×12
        place_t TW_VOLCANO, 3, 4
        place   7, 8
        place_t TW_LAMP, 11, 7
  else if LEVEL_IDX = 16  ; 2-6 T5 새싹×10
        place_t TW_LAMP, 5, 3
        place_t TW_LAMP, 5, 6
        place_t TW_VOLCANO, 10, 3
  else if LEVEL_IDX = 17  ; 2-7 T0 새싹×12+뱀×3
        place_t TW_LAMP, 3, 4
        place_t TW_VOLCANO, 7, 4
        place_t TW_LAMP, 12, 8
  else if LEVEL_IDX = 18  ; 2-8 T2 새싹×8+뱀×4
        place_t TW_LAMP, 4, 5
        place_t TW_VOLCANO, 7, 5
        place_t TW_LAMP, 12, 7
  else if LEVEL_IDX = 19  ; 2-9 T3 HALF 새싹×10+×6
        place_t TW_LAMP, 2, 5
        place_t TW_VOLCANO, 5, 5
        place_t TW_LAMP, 10, 5
  else if LEVEL_IDX = 20  ; 2-10 T4 새싹×14+뱀×3
        place_t TW_LAMP, 2, 4
        place_t TW_VOLCANO, 5, 4
        place_t TW_LAMP, 10, 8
  else if LEVEL_IDX = 21  ; 2-11 T5 Endless 해금
        place_t TW_LAMP, 4, 3
        place_t TW_VOLCANO, 7, 3
        place_t TW_LAMP, 11, 6
  ; 달3 (LEVEL_IDX 22~32)
  else if LEVEL_IDX = 22  ; 3-1 T0 묘목×6
        place_t TW_LAMP, 4, 4
        place_t TW_LAMP, 9, 4
        place_t TW_LAMP, 13, 8
  else if LEVEL_IDX = 23  ; 3-2 T2 묘목+새싹
        place_t TW_LAMP, 4, 5
        place_t TW_VOLCANO, 7, 5
        place_t TW_LAMP, 12, 7
  else if LEVEL_IDX = 24  ; 3-3 T3 묘목+뱀
        place_t TW_LAMP, 2, 5
        place_t TW_LAMP, 7, 5
        place_t TW_VOLCANO, 10, 5
  else if LEVEL_IDX = 25  ; 3-4 T5 묘목+새싹
        place_t TW_LAMP, 4, 3
        place_t TW_LAMP, 8, 3
        place_t TW_LAMP, 5, 6
  else if LEVEL_IDX = 26  ; 3-5 T4 묘목+뱀
        place_t TW_LAMP, 2, 4
        place_t TW_LAMP, 7, 4
        place_t TW_LAMP, 12, 8
  else if LEVEL_IDX = 27  ; 3-6 T1 묘목 사막바람
        place_t TW_LAMP, 3, 7
        place_t TW_WIND, 7, 7
        place_t TW_LAMP, 11, 7
  else if LEVEL_IDX = 28  ; 3-7 T2 묘목+뱀
        place_t TW_LAMP, 4, 5
        place_t TW_WIND, 7, 5
        place_t TW_LAMP, 12, 7
  else if LEVEL_IDX = 29  ; 3-8 T3 묘목+새싹
        place_t TW_LAMP, 2, 5
        place_t TW_WIND, 5, 5
        place_t TW_LAMP, 10, 5
  else if LEVEL_IDX = 30  ; 3-9 T2 SPOTS
        place_t TW_LAMP, 7, 5
        place_t TW_WIND, 8, 5
  else if LEVEL_IDX = 31  ; 3-10 T5 압박
        place_t TW_LAMP, 4, 3
        place_t TW_WIND, 7, 3
        place_t TW_LAMP, 11, 6
  else if LEVEL_IDX = 32  ; 3-11 T4 Random 해금 (묘목×6+뱀×5+새싹×10)
        place_t TW_LAMP, 2, 5    ; 빌드존0 (col1~3,row4~9)
        place_t TW_LAMP, 3, 8    ; 빌드존0
        place_t TW_WIND, 7, 8    ; 빌드존1 (col6~8,row8~9)
        place_t TW_LAMP, 12, 7   ; 빌드존2 (col11~14,row7~8)
        place_t TW_LAMP, 13, 8   ; 빌드존2
  ; 달4 (LEVEL_IDX 33~43)
  else if LEVEL_IDX = 33  ; 4-1 T0 허영+새싹
        place_t TW_FOX, 5, 4
        place_t TW_LAMP, 8, 4
        place_t TW_WIND, 11, 4
        place_t TW_LAMP, 13, 8
  else if LEVEL_IDX = 34  ; 4-2 T2 허영+묘목
        place_t TW_LAMP, 4, 5
        place_t TW_FOX, 7, 5
        place_t TW_LAMP, 10, 5
        place_t TW_LAMP, 13, 7
  else if LEVEL_IDX = 35  ; 4-3 T3 허영+뱀
        place_t TW_LAMP, 2, 5
        place_t TW_WIND, 5, 5
        place_t TW_FOX, 8, 5
        place_t TW_LAMP, 11, 5
  else if LEVEL_IDX = 36  ; 4-4 T5 거목×3
        place_t TW_LAMP, 3, 3
        place_t TW_LAMP, 8, 3
        place_t TW_FOX, 5, 3
        place_t TW_LAMP, 4, 6
        place_t TW_LAMP, 9, 6
  else if LEVEL_IDX = 37  ; 4-5 T4 거목+새싹 (FOX+LAMP+VOLCANO)
        place_t TW_FOX, 3, 4
        place_t TW_LAMP, 2, 4
        place_t TW_VOLCANO, 7, 8
        place_t TW_LAMP, 12, 7
  else if LEVEL_IDX = 38  ; 4-6 T2 사업+술꾼
        place_t TW_LAMP, 4, 5
        place_t TW_WIND, 7, 5
        place_t TW_FOX, 9, 5
        place_t TW_LAMP, 12, 7
  else if LEVEL_IDX = 39  ; 4-7 T3 사업+묘목
        place_t TW_LAMP, 2, 5
        place_t TW_WIND, 5, 5
        place_t TW_FOX, 8, 5
        place_t TW_LAMP, 11, 5
  else if LEVEL_IDX = 40  ; 4-8 T5 술꾼+거목
        place_t TW_LAMP, 3, 3
        place_t TW_WIND, 6, 3
        place_t TW_FOX, 9, 3
        place_t TW_LAMP, 4, 6
        place_t TW_LAMP, 11, 6
  else if LEVEL_IDX = 41  ; 4-9 T4 특수4종+바오밥
        place_t TW_LAMP, 2, 4
        place_t TW_WIND, 5, 4
        place_t TW_FOX, 8, 4
        place_t TW_LAMP, 11, 4
        place_t TW_LAMP, 12, 8
  else if LEVEL_IDX = 42  ; 4-10 T5 HALF 모든 종
        place_t TW_LAMP, 4, 3
        place_t TW_WIND, 7, 3
        place_t TW_FOX, 10, 3
        place_t TW_LAMP, 13, 3
  else if LEVEL_IDX = 43  ; 4-11 T5 최종 보스
        place_t TW_LAMP, 3, 3
        place_t TW_LAMP, 6, 3
        place_t TW_FOX, 9, 3
        place_t TW_LAMP, 12, 3
        place_t TW_WIND, 4, 6
        place_t TW_LAMP, 8, 6
        place_t TW_LAMP, 11, 6
  end if
end if
        ; (SCENARIO=1: 타워 없음 → 누수 → 패배)

        mov     ecx, SIM_TICKS
  .tick:
        push    ecx
        stdcall update
        pop     ecx
        loop    .tick

        stdcall render_world

        ; 상태 헤더 채우기
        mov     eax, [game_state]
        mov     [statebuf + 0], eax
        mov     eax, [petals]
        mov     [statebuf + 4], eax
        mov     eax, [starlight]
        mov     [statebuf + 8], eax
        mov     eax, [active_count]
        mov     [statebuf + 12], eax
        mov     eax, [to_spawn]
        mov     [statebuf + 16], eax
        mov     eax, [en_pos + 0]
        mov     [statebuf + 20], eax

        ; sys_write(stdout, statebuf, 24)
        mov     eax, 4
        mov     ebx, 1
        mov     ecx, statebuf
        mov     edx, 24
        int     0x80
        ; sys_write(stdout, framebuffer, FB_W*FB_H*4)
        mov     eax, 4
        mov     ebx, 1
        mov     ecx, framebuffer
        mov     edx, FB_W * FB_H * 4
        int     0x80
        ; exit(0)
        mov     eax, 1
        xor     ebx, ebx
        int     0x80
