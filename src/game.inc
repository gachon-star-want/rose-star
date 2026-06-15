; ─────────────────────────────────────────────────────────────────────────
; game.inc — 게임 로직 (Win32/플랫폼 비의존, 순수 정수 연산).
;   호출측(main.asm=PE / _render_harness.asm=ELF)이 아래 rw 데이터를 선언한다:
;     framebuffer (FB_W*FB_H dword), tilemap / tower_grid (GRID byte),
;     starlight (dd), in_cursor_cx/cy (dd, 커서 타일), in_place/in_sell (dd, edge),
;     r_cx r_cy r_px r_py r_tx r_ty (dword 임시),
;     en_active (MAX_ENEMIES byte), en_pos / en_hp (MAX_ENEMIES dword, pos=fixed-point ×256),
;     tower_cd (GRID dword), to_spawn spawn_cd active_count petals game_state (dd)
;   입력원 비의존: 플랫폼이 마우스/키보드 → in_* 로 정규화, game 은 의도만 본다(seam).
;   어떤 함수도 OS API를 부르지 않는다 → ELF 하네스로 맥에서 헤드리스 검증 가능.
; ─────────────────────────────────────────────────────────────────────────

; ── UI 문자열 (UTF-16LE) ──────────────────────────────
; 각 문자열: 길이(db), 데이터(dw×n)
; 하네스/main 모두 접근 가능하도록 game.inc에 포함

; UTF-16LE 문자열: [길이 byte][codepoint word × n]
str_starlight:
    db 3
    dw 0xBCC4, 0xBE5B, 0x0020   ; 별빛 (space)
str_petals:
    db 3
    dw 0xAF43, 0xC78E, 0x0020   ; 꽃잎 (space)
str_wave:
    db 3
    dw 0xBB34, 0xB9AC, 0x0020   ; 무리 (space)
str_won:
    db 5
    dw 0xBCC4, 0xC744, 0x0020, 0xC9C0, 0xCF30  ; 별을 지켰
str_lost:
    db 5
    dw 0xBCC4, 0xC774, 0x0020, 0xC5B4, 0xB450  ; 별이 어두
str_paused:
    db 3
    dw 0xBA48, 0xCDC4, 0xB2E4   ; 멈췄다
str_next_wave:
    db 4
    dw 0xB2E4, 0xC74C, 0x0020, 0xBB34  ; 다음 무
str_title:
    db 11
    dw 0xC5B4, 0xB9B0, 0xC655, 0xC790, 0x003A, 0x0020, 0xC7A5, 0xBBF8, 0xC758, 0x0020, 0xBCC4  ; 어린왕자: 장미의 별
str_title_start:
    db 5
    dw 0xC0C8, 0xB85C, 0x0020, 0xC2DC, 0xC791  ; 새로 시작
str_next_level:
    db 5
    dw 0xB2E4, 0xC74C, 0x0020, 0xD574, 0xB113  ; 다음 해넘
str_continue:
    db 4
    dw 0xC774, 0xC5B4, 0xD558, 0xAE30           ; 이어하기
str_code_prompt:
    db 19
    dw 0xAE30, 0xB85D, 0x0020, 0xCF54, 0xB4DC, 0xB97C, 0x0020, 0xB123, 0xC73C, 0xBA74
    dw 0x0020, 0xC774, 0xC5B4, 0xAC08, 0x0020, 0xC218, 0x0020, 0xC788, 0xB2E4  ; 기록 코드를 넣으면 이어갈 수 있다
str_code_err:
    db 10
    dw 0xCF54, 0xB4DC, 0xAC00, 0x0020, 0xB9DE, 0xC9C0, 0x0020, 0xC54A, 0xB294, 0xB2E4  ; 코드가 맞지 않는다

; num_to_u16(val) → num_buf에 숫자 UTF-16LE, eax=글자수
proc num_to_u16 uses ebx ecx edx, val
        mov     eax, [val]
        ; 0~9999 범위 가정. 앞자리부터 생성.
        lea     ecx, [num_buf]
        xor     ebx, ebx                        ; 글자수
        cmp     eax, 1000
        jl      .h
        mov     edx, eax
        xor     eax, eax
        push    edx
        pop     edx
        ; 간단히: 4자리까지 직접 출력
        ; 1000s
        mov     edx, eax
        xor     eax, eax
        push    edx
        pop     edx
        ; 직접 itoa:
        mov     eax, [val]
        mov     ebx, 0
        cmp     eax, 1000
        jl      .no_th
        xor     edx, edx
        push    eax
        mov     eax, eax
        mov     ecx, 1000
        xor     edx, edx
        pop     eax
        div     ecx
        add     al, '0'
        mov     word [num_buf + ebx*2], ax
        inc     ebx
        mov     eax, edx
  .no_th:
        cmp     eax, 100
        jl      .no_hu
        xor     edx, edx
        push    eax
        mov     eax, eax
        mov     ecx, 100
        xor     edx, edx
        pop     eax
        div     ecx
        add     al, '0'
        mov     word [num_buf + ebx*2], ax
        inc     ebx
        mov     eax, edx
  .no_hu:
  .h:
        cmp     eax, 10
        jl      .no_ten
        xor     edx, edx
        mov     ecx, 10
        div     ecx
        add     al, '0'
        mov     word [num_buf + ebx*2], ax
        inc     ebx
        mov     eax, edx
  .no_ten:
        add     al, '0'
        mov     word [num_buf + ebx*2], ax
        inc     ebx
        mov     eax, ebx
        ret
endp

; num_buf는 호출측(main.asm / harness)의 rw 세그먼트에 선언해야 함
; 여기선 extern 참조 (호출측: num_buf dw 8 dup(?))

; ── 정적 테이블 ──────────────────────────────────────

; enemy_def_table: (EN_*-1) × ED_REC 바이트
; [0]=HP [1]=speed_lo [2]=speed_hi [3]=reward [4]=leak [5]=flags
enemy_def_table:
    db  2, 179, 0,  1, 1, ENEMY_FAST                            ; EN_SPROUT  idx0
    db  4, 128, 0,  2, 1, 0                                     ; EN_SEEDLING idx1
    db  9,  90, 0,  4, 1, ENEMY_HEAVY                           ; EN_SAPLING idx2
    db 18,  58, 0,  7, 2, ENEMY_ARMORED                         ; EN_TREE    idx3
    db  4, 218, 0,  4, 2, ENEMY_FAST or ENEMY_EVADE             ; EN_SNAKE   idx4
    db  8, 102, 0,  4, 1, ENEMY_AURA                            ; EN_VAIN    idx5
    db  7, 109, 0,  0, 1, ENEMY_NO_REWARD                       ; EN_BIZ     idx6
    db  6, 115, 0,  3, 1, ENEMY_ERRATIC                         ; EN_DRUNK   idx7
    db 120, 58, 0, 50, 5, ENEMY_HEAVY or ENEMY_ARMORED or ENEMY_IS_BOSS ; BOSS idx8

; tower_def_table: (TW_*-1) × TD_REC 바이트
; [0..1]=cost(word LE) [2]=damage [3]=cooldown [4]=range [5]=flags
tower_def_table:
    dw 20
    db 1, 20, 3, TOWER_DAMAGE                                   ; TW_SHEEP   idx0
    dw 40
    db 2, 55, 2, TOWER_DAMAGE or TOWER_SPLASH                   ; TW_VOLCANO idx1
    dw 45
    db 5, 70, 4, TOWER_DAMAGE                                   ; TW_LAMP    idx2
    dw 35
    db 0, 45, 3, TOWER_SLOW                                     ; TW_WIND    idx3
    dw 50
    db 0,  0, 1, TOWER_BOOST                                    ; TW_FOX     idx4

; 타워 종류별 렌더 색 (idx = TW_*-1). HUD 팔레트 칩 색으로 사용.
tower_color_table:
    dd C_SHEEP, C_VOLCANO, C_LAMP, C_WIND, C_FOX

; ── 스프라이트 데이터 (cast24 PNG → 자동 생성) ──
include 'sprites.inc'

; type → 스프라이트/팔레트 포인터 (idx = EN_*-1 / TW_*-1)
enemy_spr_table:
    dd spr_sprout, spr_seedling, spr_sapling, spr_giant
    dd spr_snake, spr_vain, spr_biz, spr_drunk, spr_giant   ; idx8 = 보스(거목 재사용)
enemy_pal_table:
    dd pal_sprout, pal_seedling, pal_sapling, pal_giant
    dd pal_snake, pal_vain, pal_biz, pal_drunk, pal_giant
tower_spr_table:
    dd spr_sheep, spr_volcano, spr_lamp, spr_wind, spr_fox
tower_pal_table:
    dd pal_sheep, pal_volcano, pal_lamp, pal_wind, pal_fox

; ─────────────────────────────────────────────────────

; 프레임버퍼에 색 사각형. (px,py)=좌상단, (pw,ph)=크기, col=0x00RRGGBB
proc fill_rect uses esi edi ebx, px,py,pw,ph,col
        mov     esi, [py]
  @@:
        mov     eax, esi
        imul    eax, FB_W
        add     eax, [px]
        lea     edi, [framebuffer + eax*4]
        mov     ecx, [pw]
        mov     eax, [col]
        rep     stosd
        inc     esi
        mov     ebx, [py]
        add     ebx, [ph]
        cmp     esi, ebx
        jl      @b
        ret
endp

; plot_px(px,py,col): 경계 검사 후 프레임버퍼에 1픽셀. (blit_sprite 전용)
proc plot_px uses eax edx, px, py, col
        mov     eax, [px]
        test    eax, eax
        js      .skip
        cmp     eax, FB_W
        jge     .skip
        mov     edx, [py]
        test    edx, edx
        js      .skip
        cmp     edx, FB_H
        jge     .skip
        imul    edx, FB_W
        add     edx, eax
        mov     eax, [col]
        mov     [framebuffer + edx*4], eax
  .skip:
        ret
endp

; blit_sprite(spr, pal, dx, dy, w, h): 4bpp 인덱스 스프라이트 blit (idx0=투명, 경계 클립).
;   바이트 = (왼픽셀<<4)|오른픽셀. 색 = pal[idx]. docs/18 §4.
;   sx/sy는 proc 지역변수(공유 temp r_* 와 무관) → draw 루프 카운터와 충돌 없음.
;   모든 레지스터를 uses로 보존 → 호출측 루프 카운터(esi 등) 안전.
proc blit_sprite uses eax ebx ecx edx esi edi, spr, pal, dx, dy, w, h
        locals
          sx dd ?
          sy dd ?
        endl
        mov     esi, [spr]
        mov     edi, [pal]
        mov     dword [sy], 0
  .row:
        mov     eax, [sy]
        cmp     eax, [h]
        jge     .done
        mov     dword [sx], 0
  .col:
        mov     eax, [sx]
        cmp     eax, [w]
        jge     .rownext
        movzx   ebx, byte [esi]
        inc     esi
        ; 왼쪽 픽셀 = 상위 니블
        mov     ecx, ebx
        shr     ecx, 4
        test    ecx, ecx
        jz      .right
        mov     eax, [edi + ecx*4]
        mov     ecx, [sx]
        add     ecx, [dx]
        mov     edx, [sy]
        add     edx, [dy]
        stdcall plot_px, ecx, edx, eax
  .right:
        mov     ecx, ebx
        and     ecx, 0Fh
        test    ecx, ecx
        jz      .colnext
        mov     eax, [edi + ecx*4]
        mov     ecx, [sx]
        add     ecx, [dx]
        inc     ecx
        mov     edx, [sy]
        add     edx, [dy]
        stdcall plot_px, ecx, edx, eax
  .colnext:
        add     dword [sx], 2
        jmp     .col
  .rownext:
        inc     dword [sy]
        jmp     .row
  .done:
        ret
endp

; 커서 타일(in_cursor) 의 선형 인덱스를 eax 로. (cy*COLS + cx)
proc cursor_index
        mov     eax, [in_cursor_cy]
        imul    eax, GRID_COLS
        add     eax, [in_cursor_cx]
        ret
endp

; init_level(level_idx) — level_table[level_idx] 데이터를 로드해 초기화.
; level_idx: 0-based (0=1-1 .. 43=4-11). 하네스는 0으로 고정 호출.
proc init_level uses eax ebx ecx edx esi edi, level_idx
        ; ── 레벨 포인터 ──
        mov     esi, [level_idx]
        mov     esi, [level_table + esi*4]      ; esi = 레벨 데이터

        ; ── 헤더 파싱 ──
        ; [0]=tower_mask [1]=start_light [2]=petals [3]=path_count [4]=build_count [5]=wave_count [6]=flags
        movzx   eax, byte [esi+1]
        mov     [starlight], eax
        movzx   eax, byte [esi+2]
        mov     [petals], eax
        movzx   eax, byte [esi+3]               ; path_count
        mov     [seg_count], eax
        movzx   eax, byte [esi+5]               ; wave_count
        mov     [wave_total], eax

        ; ── 타일맵 초기화 ──
        mov     edi, tilemap
        mov     ecx, GRID_COLS * GRID_ROWS
        xor     al, al
        rep     stosb

        ; ── PATH 세그먼트 파싱 → seg_* 배열 ──
        ; 각 4바이트: col(b) row(b) len(b) dir(b)
        ; dir: 0=R(+1,0) 1=D(0,+1) 2=L(-1,0) 3=U(0,-1)
        ; 첫 세그 진입 픽셀: R/L→(col*16, row*16+8), D/U→(col*16+8, row*16)
        lea     eax, [esi+7]                    ; PATH 데이터 시작
        mov     ebx, [seg_count]
        xor     edi, edi                        ; seg idx
        ; acc[0] = 0
        mov     dword [seg_acc], 0

        ; 첫 세그 진입 픽셀 계산
        movzx   ecx, byte [eax]                 ; col
        movzx   edx, byte [eax+1]               ; row
        movzx   ebx, byte [eax+3]               ; dir
        push    eax                             ; save PATH ptr
        mov     eax, ecx
        imul    eax, TILE
        cmp     ebx, 1
        je      .fe_du
        cmp     ebx, 3
        je      .fe_du
        ; R or L: x=col*16, y=row*16+8
        mov     [seg_x1], eax
        mov     eax, edx
        imul    eax, TILE
        add     eax, 8
        mov     [seg_y1], eax
        jmp     .fe_done
  .fe_du:
        ; D or U: x=col*16+8, y=row*16
        add     eax, 8
        mov     [seg_x1], eax
        mov     eax, edx
        imul    eax, TILE
        mov     [seg_y1], eax
  .fe_done:
        pop     eax                             ; restore PATH ptr
        mov     ebx, [seg_count]

        ; 세그먼트 파싱 루프
        xor     edi, edi
  .seg_loop:
        cmp     edi, ebx
        jge     .seg_done
        movzx   ecx, byte [eax+3]               ; dir
        ; dx/dy
        xor     edx, edx
        mov     dword [seg_dx + edi*4], 0
        mov     dword [seg_dy + edi*4], 0
        cmp     ecx, 0
        je      .sr
        cmp     ecx, 1
        je      .sd
        cmp     ecx, 2
        je      .sl
        mov     dword [seg_dy + edi*4], -1
        jmp     .sdir_done
  .sr:  mov     dword [seg_dx + edi*4],  1
        jmp     .sdir_done
  .sd:  mov     dword [seg_dy + edi*4],  1
        jmp     .sdir_done
  .sl:  mov     dword [seg_dx + edi*4], -1
  .sdir_done:
        ; seg_len = (len-1)*16+8
        movzx   edx, byte [eax+2]               ; len(tiles)
        dec     edx
        imul    edx, TILE
        add     edx, 8
        mov     [seg_len + edi*4], edx
        ; 다음 세그 x1/y1 = 현재 끝 픽셀
        mov     ecx, edi
        inc     ecx
        cmp     ecx, ebx
        jge     .no_next_seg
        push    eax
        mov     eax, [seg_x1 + edi*4]
        push    ecx
        mov     ecx, [seg_dx + edi*4]
        imul    ecx, edx
        add     eax, ecx
        pop     ecx
        mov     [seg_x1 + ecx*4], eax
        mov     eax, [seg_y1 + edi*4]
        push    ecx
        mov     ecx, [seg_dy + edi*4]
        imul    ecx, edx
        add     eax, ecx
        pop     ecx
        mov     [seg_y1 + ecx*4], eax
        ; acc[next] = acc[cur] + len
        mov     eax, [seg_acc + edi*4]
        add     eax, edx
        mov     [seg_acc + ecx*4], eax
        pop     eax
  .no_next_seg:
        add     eax, 4
        inc     edi
        jmp     .seg_loop
  .seg_done:
        ; path_end_px = seg_acc[last] + seg_len[last]
        mov     ecx, [seg_count]
        dec     ecx
        mov     edx, [seg_acc + ecx*4]
        add     edx, [seg_len + ecx*4]
        mov     [path_end_px], edx
        ; eax → BUILD 데이터 시작

        ; ── PATH 타일 마킹 ──
        xor     edi, edi
  .ptile:
        cmp     edi, [seg_count]
        jge     .ptile_done
        mov     ecx, [seg_x1 + edi*4]
        sar     ecx, 4                          ; start col
        mov     edx, [seg_y1 + edi*4]
        sar     edx, 4                          ; start row
        push    dword [seg_len + edi*4]
        push    edx
        push    ecx
        xor     ebx, ebx
  .pmark:
        mov     ecx, [esp]                      ; col
        mov     edx, [esp+4]                    ; row
        cmp     ecx, 0
        jl      .pmark_next
        cmp     ecx, GRID_COLS-1
        jg      .pmark_next
        cmp     edx, 0
        jl      .pmark_next
        cmp     edx, GRID_ROWS-1
        jg      .pmark_next
        push    ecx edx
        imul    edx, GRID_COLS
        add     edx, ecx
        mov     byte [tilemap + edx], T_PATH
        pop     edx ecx
  .pmark_next:
        mov     ecx, [seg_dx + edi*4]
        add     [esp], ecx
        mov     ecx, [seg_dy + edi*4]
        add     [esp+4], ecx
        inc     ebx
        mov     ecx, [esp+8]
        sar     ecx, 4
        add     ecx, 2
        cmp     ebx, ecx
        jl      .pmark
        add     esp, 12
        inc     edi
        jmp     .ptile
  .ptile_done:

        ; ROSE 타일 — 마지막 세그 끝 타일
        mov     ecx, [seg_count]
        dec     ecx                             ; last seg idx in ecx
        ; rose_x = (seg_x1[last] + seg_dx[last]*seg_len[last]) >> 4
        mov     edx, [seg_x1 + ecx*4]
        mov     ebx, [seg_dx + ecx*4]
        imul    ebx, [seg_len + ecx*4]
        add     edx, ebx
        sar     edx, 4                          ; rose col
        cmp     edx, GRID_COLS-1
        jle     .rose_colok
        mov     edx, GRID_COLS-1
  .rose_colok:
        ; rose_y = (seg_y1[last] + seg_dy[last]*seg_len[last]) >> 4
        mov     ebx, [seg_y1 + ecx*4]
        mov     esi, [seg_dy + ecx*4]
        imul    esi, [seg_len + ecx*4]
        add     ebx, esi
        sar     ebx, 4                          ; rose row
        cmp     ebx, GRID_ROWS-1
        jle     .rose_rowok
        mov     ebx, GRID_ROWS-1
  .rose_rowok:
        imul    ebx, GRID_COLS
        add     ebx, edx
        mov     byte [tilemap + ebx], T_ROSE

        ; ── BUILD 직사각형 마킹 ──
        ; eax가 현재 BUILD 데이터 가리킴
        mov     ecx, [level_idx]
        mov     ecx, [level_table + ecx*4]
        movzx   ebx, byte [ecx+4]               ; build_count
        xor     edi, edi
  .build:
        cmp     edi, ebx
        jge     .build_done
        movzx   ecx, byte [eax]                 ; base_col
        movzx   edx, byte [eax+1]               ; base_row
        push    dword 0                         ; row_i
  .brow:
        movzx   esi, byte [eax+3]               ; h
        cmp     [esp], esi
        jge     .brow_end
        push    dword 0                         ; col_i
  .bcol:
        movzx   esi, byte [eax+2]               ; w
        cmp     [esp], esi
        jge     .bcol_end
        push    ecx edx
        ; tile_row = base_row + row_i
        mov     esi, edx
        add     esi, [esp+8+4]                  ; [esp+8]=col_i [esp+8+4]? No...
        ; stack after 2 pushes: [esp]=edx [esp+4]=ecx [esp+8]=col_i [esp+12]=row_i
        mov     esi, [esp+12]                   ; row_i
        add     esi, edx
        imul    esi, GRID_COLS
        mov     ebx, [esp+8]                    ; col_i
        add     ebx, ecx
        add     esi, ebx
        cmp     byte [tilemap + esi], T_GROUND
        jne     .bcol_skip
        mov     byte [tilemap + esi], T_BUILD
  .bcol_skip:
        pop     edx ecx
        inc     dword [esp]                     ; col_i++
        jmp     .bcol
  .bcol_end:
        pop     ebx                             ; col_i (discard)
        inc     dword [esp]                     ; row_i++
        jmp     .brow
  .brow_end:
        pop     ebx                             ; row_i (discard)
        add     eax, 4
        inc     edi
        ; reload build_count (ebx was clobbered)
        push    eax
        mov     eax, [level_idx]
        mov     eax, [level_table + eax*4]
        movzx   ebx, byte [eax+4]
        pop     eax
        ; reload base_col/row if needed (edi is now correct)
        cmp     edi, ebx
        jl      .build_reload
        jmp     .build_done
  .build_reload:
        movzx   ecx, byte [eax]
        movzx   edx, byte [eax+1]
        jmp     .build
  .build_done:

        ; ── WAVE 큐 로드 ──
        ; eax → WAVE 데이터 (after BUILD)
        mov     ecx, [wave_total]
        xor     edi, edi
        mov     dword [to_spawn], 0             ; 총 스폰 수 (backward compat)
  .wload:
        cmp     edi, ecx
        jge     .wload_done
        mov     bl, [eax]
        mov     [wave_types + edi], bl
        mov     bl, [eax+1]
        mov     [wave_counts + edi], bl
        ; to_spawn += count
        movzx   ebx, bl
        add     [to_spawn], ebx
        mov     bl, [eax+2]
        mov     [wave_spacings + edi], bl
        movzx   ebx, word [eax+3]
        mov     [wave_delays + edi*4], ebx
        add     eax, 5
        inc     edi
        jmp     .wload
  .wload_done:

        ; 웨이브 상태 초기화 — wave 0 즉시 활성화
        mov     dword [wave_cur], 0
        mov     dword [wave_pending], 0
        mov     dword [wave_scd], 0
        cmp     dword [wave_total], 0
        je      .no_waves
        movzx   eax, byte [wave_counts]         ; counts[0]
        mov     [wave_pending], eax
        mov     eax, [wave_delays]              ; delays[0] = 첫 스폰 전 대기
        mov     [wave_scd], eax
  .no_waves:

        ; ── 나머지 초기화 ──
        mov     edi, en_active
        mov     ecx, MAX_ENEMIES
        xor     al, al
        rep     stosb
        mov     edi, en_type
        mov     ecx, MAX_ENEMIES
        rep     stosb
        mov     edi, en_lane
        rep     stosb
        mov     edi, en_aura_buff
        rep     stosb
        xor     eax, eax
        mov     edi, en_hp
        mov     ecx, MAX_ENEMIES
        rep     stosd
        mov     edi, en_pos
        rep     stosd
        mov     edi, en_x
        rep     stosd
        mov     edi, en_y
        rep     stosd
        mov     edi, en_slow_timer
        rep     stosd
        mov     edi, tower_grid
        mov     ecx, GRID_COLS * GRID_ROWS
        xor     al, al
        rep     stosb
        xor     eax, eax
        mov     edi, tower_cd
        mov     ecx, GRID_COLS * GRID_ROWS
        rep     stosd
        mov     edi, tower_boosted
        mov     ecx, GRID_COLS * GRID_ROWS
        xor     al, al
        rep     stosb

        mov     dword [active_count], 0
        mov     dword [spawn_cd], 0
        mov     dword [in_cursor_cx], 8
        mov     dword [in_cursor_cy], 6
        mov     dword [in_place], 0
        mov     dword [in_sell], 0
        mov     dword [in_callwave], 0
        mov     dword [boss_phase], 0
        mov     dword [game_state], GS_PLAY
        mov     dword [easy_income_cd], EASY_INCOME_INTERVAL
        ; PRNG 시드
        mov     eax, [level_idx]
        imul    eax, 2654435761
        mov     ecx, [game_mode]
        imul    ecx, 40503
        add     eax, ecx
        add     eax, 0x9E3779B9
        or      eax, 1
        mov     [prng_state], eax
        ret
endp

; place_tower(ttype, cx, cy) — 타입 지정 설치 (하네스 매크로 + 게임 내부용)
proc place_tower uses eax ecx edx, ttype, cx, cy
        mov     eax, [cx]
        mov     dword [in_cursor_cx], eax
        mov     eax, [cy]
        mov     dword [in_cursor_cy], eax
        call    cursor_index
        cmp     byte [tilemap + eax], T_BUILD
        jne     .no
        cmp     byte [tower_grid + eax], TW_NONE
        jne     .no
        mov     ecx, [ttype]
        push    ecx eax
        dec     ecx
        imul    ecx, TD_REC
        movzx   edx, word [tower_def_table + ecx]   ; cost
        cmp     dword [starlight], edx
        jl      .no_pop
        pop     eax ecx
        mov     byte [tower_grid + eax], cl
        mov     dword [tower_cd + eax*4], 0
        sub     dword [starlight], edx
        ret
  .no_pop:
        pop     eax ecx
  .no:
        ret
endp

; do_place: cur_tower 타입으로 커서 위치에 설치
proc do_place uses eax
        cmp     dword [cur_tower], TW_NONE
        je      .no
        stdcall place_tower, [cur_tower], [in_cursor_cx], [in_cursor_cy]
  .no:
        ret
endp

; do_sell: 커서 칸 타워 판매 (60% = cost*3/5)
proc do_sell uses eax ecx edx
        call    cursor_index
        movzx   ecx, byte [tower_grid + eax]
        cmp     ecx, TW_NONE
        je      .no
        ; tile idx in eax, type in ecx
        push    eax
        dec     ecx
        imul    ecx, TD_REC
        movzx   edx, word [tower_def_table + ecx]   ; cost
        imul    edx, 3
        ; edx = cost*3; divide by 5
        xor     ecx, ecx
        xchg    eax, edx                        ; eax=cost*3, edx=tile_idx → BUG: lost tile idx
        ; Fix: save tile idx before
        pop     eax                             ; tile idx
        push    eax                             ; save again
        movzx   ecx, byte [tower_grid + eax]    ; type
        dec     ecx
        imul    ecx, TD_REC
        movzx   edx, word [tower_def_table + ecx]   ; cost
        imul    edx, 3
        mov     eax, edx
        xor     edx, edx
        mov     ecx, 5
        idiv    ecx                             ; eax = refund
        add     dword [starlight], eax
        pop     eax
        mov     byte [tower_grid + eax], TW_NONE
  .no:
        ret
endp

; ── pos_to_xy: en_pos[ecx]→en_x[ecx]/en_y[ecx] ──────
; ecx=슬롯. seg_* 배열로 폴리라인 구현 (1세그→T0 동일).
proc pos_to_xy uses eax ebx edx esi, slot
        mov     ecx, [slot]
        mov     eax, [en_pos + ecx*4]
        shr     eax, 8                          ; 픽셀 거리
        xor     esi, esi
  .find:
        cmp     esi, [seg_count]
        jge     .last
        mov     ebx, [seg_acc + esi*4]
        add     ebx, [seg_len + esi*4]
        cmp     eax, ebx
        jl      .use
        inc     esi
        jmp     .find
  .last:
        mov     esi, [seg_count]
        dec     esi
  .use:
        sub     eax, [seg_acc + esi*4]          ; remaining
        mov     edx, [seg_dx + esi*4]
        imul    edx, eax
        add     edx, [seg_x1 + esi*4]
        mov     [en_x + ecx*4], edx
        mov     edx, [seg_dy + esi*4]
        imul    edx, eax
        add     edx, [seg_y1 + esi*4]
        mov     [en_y + ecx*4], edx
        ret
endp

; ── 시뮬레이션 (60Hz 고정 틱) ────────────────────────────
; spawn_tick: 웨이브 큐에서 적 스폰.
; 상태: wave_cur(현재웨이브인덱스) wave_pending(남은스폰수) wave_scd(스폰간격CD)
; init에서: wave_cur=0, wave_pending=counts[0], wave_scd=delays[0](첫스폰전 대기)
proc spawn_tick uses eax ebx ecx edx
  .again:
        ; 현재 웨이브에 남은 스폰 있는가?
        cmp     dword [wave_pending], 0
        jg      .do_scd
        ; 없음 — 다음 웨이브로
        mov     eax, [wave_cur]
        inc     eax
        cmp     eax, [wave_total]
        jge     .done                           ; 모든 웨이브 소진
        mov     [wave_cur], eax
        movzx   ebx, byte [wave_counts + eax]
        mov     [wave_pending], ebx
        mov     ebx, [wave_delays + eax*4]
        mov     [wave_scd], ebx                 ; delay = 이 웨이브 첫 스폰 전 대기
        jmp     .again
  .do_scd:
        cmp     dword [wave_scd], 0
        jg      .scd_wait
        ; 스폰 1마리
        mov     eax, [wave_cur]
        movzx   eax, byte [wave_types + eax]    ; enemy type
        xor     ecx, ecx
  .find:
        cmp     ecx, MAX_ENEMIES
        jge     .no_slot
        cmp     byte [en_active + ecx], 0
        je      .spawn
        inc     ecx
        jmp     .find
  .spawn:
        mov     byte [en_active + ecx], 1
        mov     [en_type + ecx], al
        mov     byte [en_lane + ecx], 0
        mov     byte [en_aura_buff + ecx], 0
        mov     dword [en_pos + ecx*4], 0
        mov     edx, [seg_x1]
        mov     [en_x + ecx*4], edx
        mov     edx, [seg_y1]
        mov     [en_y + ecx*4], edx
        mov     dword [en_slow_timer + ecx*4], 0
        ; HP from enemy_def
        push    ecx eax
        movzx   eax, al
        dec     eax
        imul    eax, ED_REC
        movzx   edx, byte [enemy_def_table + eax]
        pop     eax ecx
        mov     [en_hp + ecx*4], edx
        inc     dword [active_count]
        dec     dword [wave_pending]
        dec     dword [to_spawn]
        ; 다음 스폰 간격 설정
        mov     ecx, [wave_cur]
        movzx   eax, byte [wave_spacings + ecx]
        mov     [wave_scd], eax
        jmp     .done
  .no_slot:
        jmp     .done
  .scd_wait:
        dec     dword [wave_scd]
  .done:
        ret
endp

; aura_recompute: EN_VAIN(AURA) 반경 내 적에 aura_buff=1 설정. 매 틱 리셋 후 재계산.
proc aura_recompute uses eax ebx ecx edx
        ; 1단계: 전 적 aura_buff 리셋
        xor     ecx, ecx
  .rst:
        cmp     ecx, MAX_ENEMIES
        jge     .vloop
        mov     byte [en_aura_buff + ecx], 0
        inc     ecx
        jmp     .rst
  .vloop:
        xor     ecx, ecx
  .v:
        cmp     ecx, MAX_ENEMIES
        jge     .done
        cmp     byte [en_active + ecx], 0
        je      .vnext
        movzx   eax, byte [en_type + ecx]
        dec     eax
        imul    eax, ED_REC
        test    byte [enemy_def_table + eax + 5], ENEMY_AURA
        jz      .vnext
        xor     ebx, ebx
  .e:
        cmp     ebx, MAX_ENEMIES
        jge     .vnext
        cmp     ebx, ecx
        je      .enext
        cmp     byte [en_active + ebx], 0
        je      .enext
        ; chebyshev(v,e) in pixels: max(|vx-ex|, |vy-ey|) <= 2*TILE
        mov     eax, [en_x + ecx*4]
        sub     eax, [en_x + ebx*4]
        cdq
        xor     eax, edx
        sub     eax, edx
        mov     edx, [en_y + ecx*4]
        sub     edx, [en_y + ebx*4]
        push    eax
        mov     eax, edx
        cdq
        xor     eax, edx
        sub     eax, edx
        pop     edx
        cmp     eax, edx
        jge     .cheb_ok
        mov     eax, edx
  .cheb_ok:
        cmp     eax, 2 * TILE
        jg      .enext
        mov     byte [en_aura_buff + ebx], 1
  .enext:
        inc     ebx
        jmp     .e
  .vnext:
        inc     ecx
        jmp     .v
  .done:
        ret
endp

; boost_recompute: TW_FOX(BOOST) 체비셰프 1타일 내 DAMAGE 타워에 boosted=1.
proc boost_recompute uses eax ebx ecx edx esi edi
        ; 1단계: 전 타워 boosted 리셋
        xor     ecx, ecx
  .rst:
        cmp     ecx, GRID_COLS * GRID_ROWS
        jge     .floop
        mov     byte [tower_boosted + ecx], 0
        inc     ecx
        jmp     .rst
  .floop:
        xor     ecx, ecx
  .f:
        cmp     ecx, GRID_COLS * GRID_ROWS
        jge     .done
        movzx   eax, byte [tower_grid + ecx]
        test    eax, eax
        jz      .fnext
        dec     eax
        imul    eax, TD_REC
        test    byte [tower_def_table + eax + 5], TOWER_BOOST
        jz      .fnext
        ; 여우 타일 좌표: fx=esi, fy=edi
        mov     eax, ecx
        xor     edx, edx
        push    GRID_COLS
        pop     esi
        div     esi
        mov     edi, eax
        mov     esi, edx
        xor     ebx, ebx
  .t:
        cmp     ebx, GRID_COLS * GRID_ROWS
        jge     .fnext
        cmp     ebx, ecx
        je      .tnext
        movzx   eax, byte [tower_grid + ebx]
        test    eax, eax
        jz      .tnext
        dec     eax
        imul    eax, TD_REC
        test    byte [tower_def_table + eax + 5], TOWER_DAMAGE
        jz      .tnext
        ; |tx-fx| <= 1 and |ty-fy| <= 1
        push    ecx
        mov     eax, ebx
        xor     edx, edx
        mov     ecx, GRID_COLS
        div     ecx                ; eax=ty, edx=tx
        sub     edx, esi
        jns     .dx_ok
        neg     edx
  .dx_ok:
        cmp     edx, 1
        jg      .t_pop
        sub     eax, edi
        jns     .dy_ok
        neg     eax
  .dy_ok:
        cmp     eax, 1
        jg      .t_pop
        pop     ecx
        mov     byte [tower_boosted + ebx], 1
        jmp     .tnext
  .t_pop:
        pop     ecx
  .tnext:
        inc     ebx
        jmp     .t
  .fnext:
        inc     ecx
        jmp     .f
  .done:
        ret
endp

; evade_check(ex_tile, ey_tile) -> al: 1이면 어떤 DAMAGE 타워 체비셰프 사거리 안
proc evade_check uses ecx edx esi edi, ex_tile, ey_tile
        mov     esi, [ex_tile]
        mov     edi, [ey_tile]
        xor     ecx, ecx
  .ec_l:
        cmp     ecx, GRID_COLS * GRID_ROWS
        jge     .ec_no
        movzx   eax, byte [tower_grid + ecx]
        test    eax, eax
        jz      .ec_next
        dec     eax
        imul    eax, TD_REC
        test    byte [tower_def_table + eax + 5], TOWER_DAMAGE
        jz      .ec_next
        movzx   edx, byte [tower_def_table + eax + 4]   ; range
        ; tower (tx,ty)
        push    ecx edx
        mov     eax, ecx
        xor     edx, edx
        push    GRID_COLS
        pop     ecx
        div     ecx                ; eax=ty, edx=tx
        sub     edx, esi
        jns     .ec_dx_ok
        neg     edx
  .ec_dx_ok:
        sub     eax, edi
        jns     .ec_dy_ok
        neg     eax
  .ec_dy_ok:
        cmp     eax, edx
        jge     .ec_cmp
        mov     eax, edx
  .ec_cmp:
        pop     edx ecx
        cmp     eax, edx
        jle     .ec_yes
  .ec_next:
        inc     ecx
        jmp     .ec_l
  .ec_no:
        xor     eax, eax
        ret
  .ec_yes:
        mov     eax, 1
        ret
endp

; 이동: 활성 적 pos += speed, pos_to_xy로 en_x/en_y 갱신.
; 장미 도달(path_end_px*256) 시 누수 → 꽃잎 -1.
proc move_tick uses eax ebx ecx edx esi edi
        xor     ecx, ecx
  .l:
        cmp     ecx, MAX_ENEMIES
        jge     .done
        cmp     byte [en_active + ecx], 0
        je      .next
        ; 타입→def 오프셋 ebx
        movzx   ebx, byte [en_type + ecx]
        dec     ebx
        imul    ebx, ED_REC
        ; eff_speed = speed word
        movzx   edx, word [enemy_def_table + ebx + 1]
        ; flags in eax
        movzx   eax, byte [enemy_def_table + ebx + 5]

        ; ERRATIC: xorshift32 jitter
        test    eax, ENEMY_ERRATIC
        jz      .no_erratic
        push    eax ecx
        mov     eax, [prng_state]
        mov     ecx, eax
        shl     ecx, 13
        xor     eax, ecx
        mov     ecx, eax
        shr     ecx, 17
        xor     eax, ecx
        mov     ecx, eax
        shl     ecx, 5
        xor     eax, ecx
        mov     [prng_state], eax
        and     eax, 63
        sub     eax, 32
        add     edx, eax
        jns     .erratic_clamp
        xor     edx, edx
  .erratic_clamp:
        pop     ecx eax
        movzx   ebx, byte [en_type + ecx]
        dec     ebx
        imul    ebx, ED_REC
  .no_erratic:

        ; EVADE: 타워 사거리 안이면 speed * 3/2
        test    eax, ENEMY_EVADE
        jz      .no_evade
        mov     esi, [en_x + ecx*4]
        shr     esi, 4              ; ex_tile
        mov     edi, [en_y + ecx*4]
        shr     edi, 4              ; ey_tile
        push    eax ecx edx
        stdcall evade_check, esi, edi
        mov     esi, eax            ; 결과 임시 보존 (pop 전에)
        pop     edx ecx eax
        test    esi, esi
        jz      .no_evade
        lea     edx, [edx + edx*2]
        shr     edx, 1              ; * 3/2
  .no_evade:

        ; SLOW 적용: slow_timer > 0이면 속도 감쇄
        cmp     dword [en_slow_timer + ecx*4], 0
        jle     .no_slow
        test    eax, ENEMY_ERRATIC
        jz      .slow_full
        imul    edx, 192          ; 저항: ×192/256 ≈ 75%
        shr     edx, 8
        jmp     .slow_done
  .slow_full:
        imul    edx, SLOW_FACTOR  ; ×128/256 = 50%
        shr     edx, 8
  .slow_done:
  .no_slow:

        ; pos += eff_speed
        mov     eax, [en_pos + ecx*4]
        add     eax, edx
        mov     [en_pos + ecx*4], eax
        ; en_x/en_y 갱신 (pos_to_xy uses eax → eax 복원 보장)
        stdcall pos_to_xy, ecx
        ; leak 확인: pos >= path_end_px × 256
        mov     edx, [path_end_px]
        shl     edx, 8
        cmp     eax, edx
        jl      .next
        mov     byte [en_active + ecx], 0
        dec     dword [active_count]
        ; leak: enemy_def_table[(type-1)*ED_REC + 4]
        movzx   eax, byte [en_type + ecx]
        dec     eax
        imul    eax, ED_REC
        movzx   eax, byte [enemy_def_table + eax + 4]
        sub     dword [petals], eax
  .next:
        inc     ecx
        jmp     .l
  .done:
        ret
endp

; on_hit(slot, dmg) — ARMORED/AURA 경감, 처치 처리
; slot=en slot idx, dmg=base damage
proc on_hit uses eax ebx ecx edx, slot, dmg
        mov     ecx, [slot]
        mov     edx, [dmg]
        ; AURA 버프 → 피해 -1
        cmp     byte [en_aura_buff + ecx], 0
        je      .no_aura
        dec     edx
  .no_aura:
        ; ARMORED → 피해 -1
        movzx   eax, byte [en_type + ecx]
        dec     eax
        imul    eax, ED_REC
        test    byte [enemy_def_table + eax + 5], ENEMY_ARMORED
        jz      .no_armor
        dec     edx
  .no_armor:
        ; 최소 피해 1
        cmp     edx, 1
        jge     .apply
        mov     edx, 1
  .apply:
        sub     dword [en_hp + ecx*4], edx
        cmp     dword [en_hp + ecx*4], 0
        jg      .done
        ; 처치
        movzx   eax, byte [en_type + ecx]
        dec     eax
        imul    eax, ED_REC
        test    byte [enemy_def_table + eax + 5], ENEMY_NO_REWARD
        jnz     .kill
        movzx   ebx, byte [enemy_def_table + eax + 3]  ; reward
        add     dword [starlight], ebx
  .kill:
        mov     byte [en_active + ecx], 0
        dec     dword [active_count]
  .done:
        ret
endp

; chebyshev_in_range(eidx, tcol, trow, range) → al=1 if in range, 0 otherwise
proc cheby_in_range uses ebx ecx edx, eidx, tcol, trow, range
        mov     ecx, [eidx]
        mov     eax, [en_x + ecx*4]
        sar     eax, 4                          ; encol
        sub     eax, [tcol]
        mov     edx, eax
        sar     edx, 31
        xor     eax, edx
        sub     eax, edx                        ; abs(encol-tcol)
        cmp     eax, [range]
        jg      .out
        mov     eax, [en_y + ecx*4]
        sar     eax, 4                          ; enrow
        sub     eax, [trow]
        mov     edx, eax
        sar     edx, 31
        xor     eax, edx
        sub     eax, edx
        cmp     eax, [range]
        jg      .out
        mov     al, 1
        ret
  .out:
        xor     al, al
        ret
endp

; fire_tick: 범용 타워 발사 (DAMAGE/SPLASH/SLOW/BOOST)
proc fire_tick uses eax ebx ecx edx esi edi
        xor     esi, esi
  .cell:
        cmp     esi, GRID_COLS * GRID_ROWS
        jge     .done
        movzx   eax, byte [tower_grid + esi]
        cmp     eax, TW_NONE
        je      .nextcell
        ; 쿨다운 감소
        cmp     dword [tower_cd + esi*4], 0
        jle     .ready
        dec     dword [tower_cd + esi*4]
        jmp     .nextcell
  .ready:
        ; tower def 포인터 → edi
        mov     ecx, eax
        dec     ecx
        imul    ecx, TD_REC
        lea     edi, [tower_def_table + ecx]

        ; 타워 col/row
        mov     ebx, esi
        and     ebx, GRID_COLS-1               ; tcol
        mov     edx, esi
        shr     edx, 4                          ; trow

        ; BOOST: 공격 안 함 (boost_recompute에서 처리)
        test    byte [edi+5], TOWER_BOOST
        jnz     .set_cd

        ; SLOW: 사거리 내 모든 적 slow_timer 리셋
        test    byte [edi+5], TOWER_SLOW
        jz      .check_dmg
        movzx   eax, byte [edi+4]               ; range
        push    eax
        xor     ecx, ecx
  .slow_scan:
        cmp     ecx, MAX_ENEMIES
        jge     .slow_done
        cmp     byte [en_active + ecx], 0
        je      .slow_next
        push    ecx
        stdcall cheby_in_range, ecx, ebx, edx, dword [esp+4]
        pop     ecx
        test    al, al
        jz      .slow_next
        mov     dword [en_slow_timer + ecx*4], SLOW_DURATION
  .slow_next:
        inc     ecx
        jmp     .slow_scan
  .slow_done:
        pop     eax
        jmp     .set_cd

  .check_dmg:
        test    byte [edi+5], TOWER_DAMAGE
        jz      .set_cd

        ; 타겟: path_progress 최대 (en_pos 기준)
        movzx   eax, byte [edi+4]               ; range
        push    eax                             ; [esp]=range
        mov     dword [r_tx], -1                ; best_slot
        mov     dword [r_ty], -1                ; best_pos
        xor     ecx, ecx
  .tscan:
        cmp     ecx, MAX_ENEMIES
        jge     .tfire
        cmp     byte [en_active + ecx], 0
        je      .tnext
        push    ecx
        stdcall cheby_in_range, ecx, ebx, edx, dword [esp+4]
        pop     ecx
        test    al, al
        jz      .tnext
        mov     eax, [en_pos + ecx*4]
        cmp     eax, [r_ty]
        jle     .tnext
        mov     [r_ty], eax
        mov     [r_tx], ecx
  .tnext:
        inc     ecx
        jmp     .tscan
  .tfire:
        pop     eax                             ; range (discard)
        mov     eax, [r_tx]
        cmp     eax, -1
        je      .set_cd
        ; 피해 적용
        movzx   ecx, byte [edi+2]               ; damage
        push    esi
        stdcall on_hit, eax, ecx
        pop     esi

        ; SPLASH: 주 타겟 ±1 타일 체비셰프
        test    byte [edi+5], TOWER_SPLASH
        jz      .set_cd
        mov     eax, [r_tx]
        cmp     byte [en_active + eax], 0       ; 주 타겟이 살았으면 col/row 기준
        je      .splash_dead
        mov     ecx, [en_x + eax*4]
        sar     ecx, 4                          ; mtcol
        mov     edx, [en_y + eax*4]
        sar     edx, 4                          ; mtrow
        jmp     .splash_scan
  .splash_dead:
        ; 주 타겟이 죽었으면 타워 위치 기준으로 splash
        mov     ecx, esi
        and     ecx, GRID_COLS-1
        mov     edx, esi
        shr     edx, 4
  .splash_scan:
        push    ecx edx                         ; mtcol, mtrow on stack
        xor     eax, eax
  .sloop:
        cmp     eax, MAX_ENEMIES
        jge     .splash_done
        cmp     byte [en_active + eax], 0
        je      .snext
        push    eax
        stdcall cheby_in_range, eax, dword [esp+8], dword [esp+4], 1
        pop     eax
        test    al, al
        jz      .snext
        movzx   ecx, byte [edi+2]               ; damage
        push    eax esi
        stdcall on_hit, eax, ecx
        pop     esi eax
  .snext:
        inc     eax
        jmp     .sloop
  .splash_done:
        pop     edx ecx
        ; edi (tower_def ptr) is still valid
        ; edx was tcol/trow - restore for set_cd
        mov     edx, esi
        shr     edx, 4

  .set_cd:
        movzx   ecx, byte [edi+3]               ; cooldown
        ; 여우 부스트: boosted면 cd *= BOOST_FACTOR/256 (≈85%)
        cmp     byte [tower_boosted + esi], 0
        je      .no_boost
        imul    ecx, BOOST_FACTOR
        shr     ecx, 8
        test    ecx, ecx
        jnz     .no_boost
        inc     ecx
  .no_boost:
        mov     [tower_cd + esi*4], ecx
  .nextcell:
        inc     esi
        jmp     .cell
  .done:
        ret
endp

; decay_tick: slow_timer 감소 (매 틱 1씩, 0 하한)
proc decay_tick uses ecx
        xor     ecx, ecx
  .l:
        cmp     ecx, MAX_ENEMIES
        jge     .done
        cmp     dword [en_slow_timer + ecx*4], 0
        jle     .next
        dec     dword [en_slow_timer + ecx*4]
  .next:
        inc     ecx
        jmp     .l
  .done:
        ret
endp

; boss_burst_n(etype, cnt): free 슬롯에 etype 적 cnt마리 즉시 스폰 (호위 전용)
proc boss_burst_n uses eax ebx ecx edx, etype, cnt
  .bn_loop:
        cmp     dword [cnt], 0
        jle     .bn_done
        xor     ebx, ebx
  .bn_find:
        cmp     ebx, MAX_ENEMIES
        jge     .bn_done           ; free 슬롯 없음 → 스킵
        cmp     byte [en_active + ebx], 0
        je      .bn_slot
        inc     ebx
        jmp     .bn_find
  .bn_slot:
        movzx   eax, byte [etype]
        mov     byte  [en_active + ebx], 1
        mov     [en_type + ebx], al
        mov     byte  [en_lane + ebx], 0
        mov     byte  [en_aura_buff + ebx], 0
        mov     dword [en_pos + ebx*4], 0
        mov     ecx, [seg_x1]
        mov     [en_x + ebx*4], ecx
        mov     ecx, [seg_y1]
        mov     [en_y + ebx*4], ecx
        mov     dword [en_slow_timer + ebx*4], 0
        movzx   ecx, al
        dec     ecx
        imul    ecx, ED_REC
        movzx   edx, byte [enemy_def_table + ecx]
        mov     [en_hp + ebx*4], edx
        inc     dword [active_count]
        dec     dword [cnt]
        inc     ebx
        jmp     .bn_loop
  .bn_done:
        ret
endp

; boss_tick: ENEMY_IS_BOSS HP 임계(90/60/30) 시 호위 소환
boss_thresholds dd 90, 60, 30

proc boss_tick uses eax ecx edx
        ; 보스 슬롯 탐색
        xor     ecx, ecx
  .bsf:
        cmp     ecx, MAX_ENEMIES
        jge     .bt_done
        cmp     byte [en_active + ecx], 0
        je      .bsn
        movzx   eax, byte [en_type + ecx]
        dec     eax
        imul    eax, ED_REC
        test    byte [enemy_def_table + eax + 5], ENEMY_IS_BOSS
        jnz     .bt_found
  .bsn:
        inc     ecx
        jmp     .bsf
  .bt_found:
        ; phase 루프
  .bpl:
        cmp     dword [boss_phase], 3
        jge     .bt_done
        mov     eax, [boss_phase]
        lea     edx, [boss_thresholds]
        mov     edx, [edx + eax*4]
        cmp     dword [en_hp + ecx*4], edx
        jg      .bt_done
        ; 호위 소환
        push    ecx
        cmp     eax, 0
        je      .ph0
        cmp     eax, 1
        je      .ph1
        ; phase2: SAPLING×2 + SNAKE×1
        stdcall boss_burst_n, EN_SAPLING, 2
        stdcall boss_burst_n, EN_SNAKE,   1
        jmp     .ph_done
  .ph0:
        stdcall boss_burst_n, EN_SPROUT,  4
        jmp     .ph_done
  .ph1:
        stdcall boss_burst_n, EN_SEEDLING, 3
  .ph_done:
        pop     ecx
        inc     dword [boss_phase]
        jmp     .bpl
  .bt_done:
        ret
endp

; 승/패 판정
proc winloss_tick
        cmp     dword [petals], 0
        jg      .checkwin
        mov     dword [game_state], GS_LOST
        ret
  .checkwin:
        ; 모든 WAVE 소진 + pending=0 + active=0
        cmp     dword [to_spawn], 0
        jne     .done
        cmp     dword [wave_pending], 0
        jne     .done
        cmp     dword [active_count], 0
        jne     .done
        mov     dword [game_state], GS_WON
  .done:
        ret
endp

; 한 프레임(틱) 갱신. game_state 게이트가 최상단: 플레이 중에만 진행.
; 순서 = 입력 → 스폰 → 이동(누수) → 발사(처치) → 승패.
proc update
        cmp     dword [game_state], GS_PLAY
        jne     .frozen
        cmp     dword [in_place], 0
        je      .sell
        stdcall do_place
        mov     dword [in_place], 0
  .sell:
        cmp     dword [in_sell], 0
        je      .cwe
        stdcall do_sell
        mov     dword [in_sell], 0
  .cwe:
        ; [N] 조기 소환: 대기 틱 잔량 보너스 지급 후 즉시 소환
        cmp     dword [in_callwave], 0
        je      .sim
        mov     dword [in_callwave], 0
        cmp     dword [wave_scd], 0
        jle     .sim
        mov     eax, [wave_scd]
        cdq
        mov     ecx, WAVE_BONUS_DIV
        idiv    ecx
        add     [starlight], eax
        mov     dword [wave_scd], 0
  .sim:
        stdcall spawn_tick
        stdcall aura_recompute
        stdcall boost_recompute
        stdcall move_tick
        stdcall fire_tick
        stdcall boss_tick
        stdcall decay_tick
        stdcall winloss_tick
        ret
  .frozen:
        mov     dword [in_place], 0             ; 의도 비움(R=init_level 만 부활)
        mov     dword [in_sell], 0
        mov     dword [in_callwave], 0
        ret
endp

; ── 비트맵 폰트 텍스트 렌더링 ───────────────────────
; draw_char(codepoint, px, py, fg): 12×12 글리프 하나 렌더링
; 폰트 엔트리: [codepoint u16][bitmap 24B] = 26B
proc draw_char uses eax ebx ecx edx esi edi, codepoint, dpx, dpy, fg
        ; 이진탐색
        xor     esi, esi
        mov     edi, FONT_GLYPH_COUNT
        dec     edi
  .bs:
        cmp     esi, edi
        jg      .nf
        lea     eax, [esi + edi]
        shr     eax, 1                          ; mid
        mov     ecx, eax
        imul    ecx, 26
        lea     ecx, [font_glyph_table + ecx]
        movzx   ebx, word [ecx]
        cmp     ebx, [codepoint]
        je      .found
        jg      .lo
        mov     esi, eax
        inc     esi
        jmp     .bs
  .lo: mov     edi, eax
        dec     edi
        jmp     .bs
  .nf: ret
  .found:
        add     ecx, 2                          ; bitmap 시작
        ; row 0..11
        xor     edx, edx
  .row:
        cmp     edx, 12
        jge     .rdone
        ; 12비트 = (byte[edx*2]<<4) | (byte[edx*2+1]>>4)
        movzx   eax, byte [ecx + edx*2]
        shl     eax, 4
        movzx   ebx, byte [ecx + edx*2 + 1]
        shr     ebx, 4
        or      eax, ebx                        ; bits 11..0
        ; col 0..11: bit(11-col)
        push    edx
        xor     edx, edx
  .col:
        cmp     edx, 12
        jge     .cdone
        ; bt eax, (11-edx)
        mov     ebx, 11
        sub     ebx, edx
        bt      eax, ebx
        jnc     .cn
        ; fb[(dpy+row)*FB_W + (dpx+col)]
        mov     esi, [dpy]
        add     esi, [esp]                      ; [esp]=row (pushed edx)
        cmp     esi, 0
        jl      .cn
        cmp     esi, FB_H-1
        jg      .cn
        mov     edi, [dpx]
        add     edi, edx
        cmp     edi, 0
        jl      .cn
        cmp     edi, FB_W-1
        jg      .cn
        imul    esi, FB_W
        add     esi, edi
        mov     ebx, [fg]
        mov     [framebuffer + esi*4], ebx
  .cn:
        inc     edx
        jmp     .col
  .cdone:
        pop     edx
        inc     edx
        jmp     .row
  .rdone:
        ret
endp

; draw_hud_text(str_ptr_utf16, len, px, py, fg): HUD용 텍스트
; 간단히 draw_char 반복
proc draw_hud_text uses eax ecx esi edx, sptr, slen, px, py, fg
        mov     esi, [sptr]
        mov     ecx, [slen]
        mov     edx, [px]
  .l:
        cmp     ecx, 0
        jle     .done
        movzx   eax, word [esi]
        stdcall draw_char, eax, edx, [py], [fg]
        add     edx, 13                         ; 글자폭 12 + 간격 1
        add     esi, 2
        dec     ecx
        jmp     .l
  .done:
        ret
endp

C_TEXT     = 0x00E8E8E8
C_TEXT_DIM = 0x00888888

; ── 기록 코드 (Record Code) ─────────────────────────
; RS-{달}-{해넘이}-{모드}-{HEX4}
; 16비트 페이로드: [0..2]=꽃잎(3b) [3..7]=타워수(5b) [8..12]=판매수(5b) [13..15]=체크(3b)
; 체크 = (꽃잎+타워수+판매수+달+해넘이) & 7

; record_code_buf는 호출측(main.asm / harness) rw 세그먼트에 선언
; record_code_buf  db 20 dup(0)

; encode_record(moon, sunset, mode_e_or_h, petals, towers_placed, sells) → record_code_buf에 ASCII
; 단순 버전: mode=0→'E' mode=1→'H'
proc encode_record uses eax ebx ecx edx, moon, sunset, mode, petals_val, tcount, scount
        lea     edi, [record_code_buf]
        ; "RS-"
        mov     byte [edi], 'R'
        mov     byte [edi+1], 'S'
        mov     byte [edi+2], '-'
        add     edi, 3
        ; 달 (1자리)
        mov     eax, [moon]
        add     al, '0'
        mov     byte [edi], al
        mov     byte [edi+1], '-'
        add     edi, 2
        ; 해넘이 (1~2자리)
        mov     eax, [sunset]
        cmp     eax, 10
        jl      .s1
        ; 2자리
        xor     edx, edx
        mov     ecx, 10
        div     ecx
        add     al, '0'
        mov     byte [edi], al
        inc     edi
        add     dl, '0'
        mov     byte [edi], dl
        inc     edi
        jmp     .s_done
  .s1:
        add     al, '0'
        mov     byte [edi], al
        inc     edi
  .s_done:
        mov     byte [edi], '-'
        inc     edi
        ; 모드
        cmp     dword [mode], 0
        jne     .hard
        mov     byte [edi], 'E'
        jmp     .mode_done
  .hard:
        mov     byte [edi], 'H'
  .mode_done:
        inc     edi
        mov     byte [edi], '-'
        inc     edi
        ; 16비트 페이로드 계산
        mov     eax, [petals_val]
        and     eax, 7
        mov     ebx, [tcount]
        and     ebx, 31
        shl     ebx, 3
        or      eax, ebx
        mov     ebx, [scount]
        and     ebx, 31
        shl     ebx, 8
        or      eax, ebx
        ; 체크
        mov     ecx, [petals_val]
        add     ecx, [tcount]
        add     ecx, [scount]
        add     ecx, [moon]
        add     ecx, [sunset]
        and     ecx, 7
        shl     ecx, 13
        or      eax, ecx
        ; eax = 16비트 페이로드 → 4자리 HEX
        push    eax
        ; HEX digit 3 (most significant)
        shr     eax, 12
        and     eax, 0xF
        cmp     eax, 10
        jl      .d3
        add     eax, 'A'-10
        jmp     .d3d
  .d3:  add     eax, '0'
  .d3d: mov     byte [edi], al
        inc     edi
        pop     eax
        push    eax
        shr     eax, 8
        and     eax, 0xF
        cmp     eax, 10
        jl      .d2
        add     eax, 'A'-10
        jmp     .d2d
  .d2:  add     eax, '0'
  .d2d: mov     byte [edi], al
        inc     edi
        pop     eax
        push    eax
        shr     eax, 4
        and     eax, 0xF
        cmp     eax, 10
        jl      .d1
        add     eax, 'A'-10
        jmp     .d1d
  .d1:  add     eax, '0'
  .d1d: mov     byte [edi], al
        inc     edi
        pop     eax
        and     eax, 0xF
        cmp     eax, 10
        jl      .d0
        add     eax, 'A'-10
        jmp     .d0d
  .d0:  add     eax, '0'
  .d0d: mov     byte [edi], al
        inc     edi
        mov     byte [edi], 0              ; null 종료
        ret
endp

; decode_record → eax=level_idx(0~43) 성공 / eax=-1 실패
; 입력: code_input_buf(ASCII, null-terminated)
; 성공 시 game_mode 설정. r_cx=moon, r_cy=sunset, r_px=mode 임시 사용.
proc decode_record uses ebx ecx edx esi edi
        lea     esi, [code_input_buf]
        cmp     byte [esi],   'R'
        jne     .fail
        cmp     byte [esi+1], 'S'
        jne     .fail
        cmp     byte [esi+2], '-'
        jne     .fail
        add     esi, 3
        ; 달 1자리 '1'~'4'
        movzx   eax, byte [esi]
        sub     eax, '0'
        cmp     eax, 1
        jl      .fail
        cmp     eax, 4
        jg      .fail
        mov     [r_cx], eax
        inc     esi
        cmp     byte [esi], '-'
        jne     .fail
        inc     esi
        ; 해넘이 1~2자리 '1'~'11'
        movzx   eax, byte [esi]
        sub     eax, '0'
        cmp     eax, 0
        jl      .fail
        cmp     eax, 9
        jg      .fail
        movzx   ecx, byte [esi+1]
        cmp     ecx, '0'
        jl      .s1
        cmp     ecx, '9'
        jg      .s1
        sub     ecx, '0'
        imul    eax, 10
        add     eax, ecx
        add     esi, 2
        jmp     .sdone
  .s1:  inc     esi
  .sdone:
        cmp     eax, 1
        jl      .fail
        cmp     eax, 11
        jg      .fail
        mov     [r_cy], eax
        cmp     byte [esi], '-'
        jne     .fail
        inc     esi
        ; 모드 'E'=0 'H'=1
        movzx   eax, byte [esi]
        cmp     eax, 'E'
        je      .me
        cmp     eax, 'H'
        jne     .fail
        mov     dword [r_px], 1
        jmp     .mdone
  .me:  mov     dword [r_px], 0
  .mdone:
        inc     esi
        cmp     byte [esi], '-'
        jne     .fail
        inc     esi
        ; HEX 4자리 (대문자 A-F)
        xor     ebx, ebx
        mov     ecx, 4
  .hex: movzx   eax, byte [esi]
        cmp     eax, '0'
        jl      .fail
        cmp     eax, '9'
        jle     .hd
        cmp     eax, 'A'
        jl      .fail
        cmp     eax, 'F'
        jg      .fail
        sub     eax, 'A'-10
        jmp     .hok
  .hd:  sub     eax, '0'
  .hok: shl     ebx, 4
        or      ebx, eax
        inc     esi
        loop    .hex
        cmp     byte [esi], 0
        jne     .fail
        ; 페이로드 분해: [0..2]=꽃잎 [3..7]=타워수 [8..12]=판매수 [13..15]=체크
        mov     eax, ebx
        and     eax, 7                 ; petals
        mov     edx, ebx
        shr     edx, 3
        and     edx, 31               ; tcount
        mov     ecx, ebx
        shr     ecx, 8
        and     ecx, 31               ; scount
        mov     edi, ebx
        shr     edi, 13
        and     edi, 7                 ; check_stored
        ; 체크 검증
        add     eax, edx
        add     eax, ecx
        add     eax, [r_cx]
        add     eax, [r_cy]
        and     eax, 7
        cmp     eax, edi
        jne     .fail
        ; 성공
        mov     eax, [r_px]
        mov     [game_mode], eax
        mov     eax, [r_cx]
        dec     eax
        imul    eax, 11
        mov     ecx, [r_cy]
        dec     ecx
        add     eax, ecx
        ret
  .fail:
        mov     eax, -1
        ret
endp

; ── 렌더 ──────────────────────────────────────────────
; 타일맵 → 색 사각형.
proc draw_tiles uses eax ebx
        mov     dword [r_cy], 0
  .yl:
        mov     dword [r_cx], 0
  .xl:
        mov     eax, [r_cy]
        imul    eax, GRID_COLS
        add     eax, [r_cx]
        movzx   ebx, byte [tilemap + eax]
        mov     eax, [r_cx]
        shl     eax, 4
        mov     [r_px], eax
        mov     eax, [r_cy]
        shl     eax, 4
        mov     [r_py], eax
        cmp     ebx, T_PATH
        je      .path
        cmp     ebx, T_BUILD
        je      .bz
        cmp     ebx, T_ROSE
        je      .rose
        stdcall fill_rect, [r_px], [r_py], TILE, TILE, C_GROUND
        jmp     .next
  .path:
        stdcall fill_rect, [r_px], [r_py], TILE, TILE, C_PATH
        jmp     .next
  .rose:
        ; 장미는 경로 끝에 선다: 바닥은 path 색, 그 위 24px 장미 스프라이트(아래중앙 앵커)
        stdcall fill_rect, [r_px], [r_py], TILE, TILE, C_PATH
        mov     eax, [r_px]
        sub     eax, 4
        mov     [r_tx], eax
        mov     eax, [r_py]
        sub     eax, 8
        mov     [r_ty], eax
        stdcall blit_sprite, spr_rose, pal_rose, [r_tx], [r_ty], 24, 24
        jmp     .next
  .bz:
        stdcall fill_rect, [r_px], [r_py], TILE, TILE, C_BUILDE
        mov     eax, [r_px]
        inc     eax
        mov     [r_tx], eax
        mov     eax, [r_py]
        inc     eax
        mov     [r_ty], eax
        stdcall fill_rect, [r_tx], [r_ty], TILE-2, TILE-2, C_BUILD
  .next:
        inc     dword [r_cx]
        mov     eax, [r_cx]
        cmp     eax, GRID_COLS
        jl      .xl
        inc     dword [r_cy]
        mov     eax, [r_cy]
        cmp     eax, GRID_ROWS
        jl      .yl
        ret
endp

; tower_grid → 종류별 24px 스프라이트 (아래중앙 앵커). spr/pal = tower_*_table[type-1].
proc draw_towers uses eax ebx
        mov     dword [r_cy], 0
  .yl:
        mov     dword [r_cx], 0
  .xl:
        mov     eax, [r_cy]
        imul    eax, GRID_COLS
        add     eax, [r_cx]
        movzx   ebx, byte [tower_grid + eax]
        cmp     ebx, TW_NONE
        je      .next
        dec     ebx                            ; type-1
        mov     eax, [tower_spr_table + ebx*4]
        mov     [r_tx], eax
        mov     eax, [tower_pal_table + ebx*4]
        mov     [r_ty], eax
        ; 아래중앙 앵커: base_x = cx*16+8-12, base_y = cy*16+16-24
        mov     eax, [r_cx]
        shl     eax, 4
        sub     eax, 4
        mov     [r_px], eax
        mov     eax, [r_cy]
        shl     eax, 4
        sub     eax, 8
        mov     [r_py], eax
        stdcall blit_sprite, [r_tx], [r_ty], [r_px], [r_py], 24, 24
  .next:
        inc     dword [r_cx]
        mov     eax, [r_cx]
        cmp     eax, GRID_COLS
        jl      .xl
        inc     dword [r_cy]
        mov     eax, [r_cy]
        cmp     eax, GRID_ROWS
        jl      .yl
        ret
endp

; 활성 적 → 6×6 연두 사각형 (en_x, en_y 기반).
; 주의: 루프 카운터는 esi 사용 — fill_rect가 ecx를 rep stosd로 파괴하므로
; ecx를 카운터로 쓰면 활성 적에서 무한루프(esi/edi/ebx는 fill_rect가 보존).
proc draw_enemies uses eax ebx esi
        xor     esi, esi
  .l:
        cmp     esi, MAX_ENEMIES
        jge     .done
        cmp     byte [en_active + esi], 0
        je      .next
        movzx   ebx, byte [en_type + esi]
        dec     ebx                            ; type-1
        mov     eax, [enemy_spr_table + ebx*4]
        mov     [r_tx], eax
        mov     eax, [enemy_pal_table + ebx*4]
        mov     [r_ty], eax
        ; 아래중앙 앵커: base_x = en_x-12, base_y = en_y-16
        mov     eax, [en_x + esi*4]
        sub     eax, 12
        mov     [r_px], eax
        mov     eax, [en_y + esi*4]
        sub     eax, 16
        mov     [r_py], eax
        stdcall blit_sprite, [r_tx], [r_ty], [r_px], [r_py], 24, 24
  .next:
        inc     esi
        jmp     .l
  .done:
        ret
endp

; 커서 칸에 밝은 2px 테두리.
proc draw_cursor uses eax
        mov     eax, [in_cursor_cx]
        shl     eax, 4
        mov     [r_px], eax
        mov     eax, [in_cursor_cy]
        shl     eax, 4
        mov     [r_py], eax
        ; top
        stdcall fill_rect, [r_px], [r_py], TILE, 2, C_CURSOR
        ; bottom (py+14)
        mov     eax, [r_py]
        add     eax, TILE-2
        mov     [r_ty], eax
        stdcall fill_rect, [r_px], [r_ty], TILE, 2, C_CURSOR
        ; left
        stdcall fill_rect, [r_px], [r_py], 2, TILE, C_CURSOR
        ; right (px+14)
        mov     eax, [r_px]
        add     eax, TILE-2
        mov     [r_tx], eax
        stdcall fill_rect, [r_tx], [r_py], 2, TILE, C_CURSOR
        ret
endp

; HUD 바: 별빛 라벨+숫자(좌) · 꽃잎 pip(중앙) · 타워 팔레트 5칸(우, cur_tower 강조).
; 팔레트는 5종 색칩을 항상 그리고, 잠긴 종류 덧칠은 플랫폼(main.asm)이 cur_level_mask로 처리.
proc draw_hud uses eax
        stdcall fill_rect, 0, 0, FB_W, TILE, C_HUD
        ; 별빛 라벨 + 숫자 (좌)
        stdcall draw_hud_text, str_starlight+1, 3, 2, 2, C_TEXT
        stdcall num_to_u16, [starlight]
        stdcall draw_hud_text, num_buf, eax, 40, 2, C_STAR
        ; 꽃잎 pip (중앙)
        mov     dword [r_cy], 0
  .pip:
        mov     eax, [r_cy]
        cmp     eax, [petals]
        jge     .palette
        imul    eax, 8
        add     eax, 90
        stdcall fill_rect, eax, 5, 6, 6, C_PETAL
        inc     dword [r_cy]
        jmp     .pip
  .palette:
        ; 5 타워 칩 (우). 색=tower_color_table[i]. cur_tower 강조 테두리.
        mov     dword [r_cx], 0
  .pl:
        mov     eax, [r_cx]
        cmp     eax, 5
        jge     .pldone
        imul    eax, PAL_STEP
        add     eax, PAL_X0
        mov     [r_px], eax
        ; 선택 강조 (i+1 == cur_tower → C_CURSOR 테두리)
        mov     eax, [r_cx]
        inc     eax
        cmp     eax, [cur_tower]
        jne     .nohi
        mov     eax, [r_px]
        sub     eax, 2
        stdcall fill_rect, eax, PAL_Y-2, PAL_SZ+4, PAL_SZ+4, C_CURSOR
  .nohi:
        mov     eax, [r_cx]
        mov     eax, [tower_color_table + eax*4]
        mov     [r_ty], eax
        stdcall fill_rect, [r_px], PAL_Y, PAL_SZ, PAL_SZ, [r_ty]
        inc     dword [r_cx]
        jmp     .pl
  .pldone:
        ret
endp

; 화면 가장자리 4px 테두리 프레임(승/패 신호용).
proc draw_frame, col
        stdcall fill_rect, 0, 0, FB_W, 4, [col]
        stdcall fill_rect, 0, FB_H-4, FB_W, 4, [col]
        stdcall fill_rect, 0, 0, 4, FB_H, [col]
        stdcall fill_rect, FB_W-4, 0, 4, FB_H, [col]
        ret
endp

; 현재 상태를 framebuffer 로 그린다. 승/패 시 테두리 프레임으로 결과를 알린다.
proc render_world
        cmp     dword [game_state], GS_TITLE
        je      .title
        cmp     dword [game_state], GS_CODE_INPUT
        je      .code_input
        stdcall draw_tiles
        stdcall draw_towers
        stdcall draw_enemies
        stdcall draw_cursor
        stdcall draw_hud
        cmp     dword [game_state], GS_WON
        je      .won
        cmp     dword [game_state], GS_LOST
        je      .lost
        ret
  .won:
        stdcall draw_frame, C_WIN
        stdcall draw_hud_text, str_won+1, 5, 70, 88, C_WIN
        stdcall draw_hud_text, str_next_level+1, 5, 68, 108, C_TEXT
        ret
  .lost:
        stdcall draw_frame, C_LOSS
        stdcall draw_hud_text, str_lost+1, 5, 70, 88, C_LOSS
        ret
  .code_input:
        stdcall fill_rect, 0, 0, FB_W, FB_H, C_GROUND
        ; "이어하기" 4자=48px, x=(256-48)/2=104
        stdcall draw_hud_text, str_continue+1, 4, 104, 72, C_STAR
        ; 프롬프트 19자=228px, x=(256-228)/2=14
        stdcall draw_hud_text, str_code_prompt+1, 19, 14, 92, C_TEXT_DIM
        ; code_input_buf ASCII → draw_char 한 자씩 (14자 RS-M-SS-X-XXXX)
        push    dword 0                ; [esp] = 문자 인덱스
  .ci_loop:
        mov     ecx, [esp]
        movzx   eax, byte [code_input_buf + ecx]
        test    eax, eax
        jz      .ci_done
        mov     edx, ecx
        imul    edx, 12
        add     edx, 44                ; x = 44 + idx*12 (14자: 44~204)
        stdcall draw_char, eax, edx, 112, C_WIN
        inc     dword [esp]
        jmp     .ci_loop
  .ci_done:
        pop     eax
        cmp     dword [code_input_err], 0
        je      .ci_ret
        ; "코드가 맞지 않는다" 10자=120px, x=(256-120)/2=68
        stdcall draw_hud_text, str_code_err+1, 10, 68, 132, C_LOSS
  .ci_ret:
        ret
  .title:
        stdcall fill_rect, 0, 0, FB_W, FB_H, C_GROUND
        ; '어린왕자: 장미의 별' 11자 = 132px, x=(256-132)/2=62
        stdcall draw_hud_text, str_title+1, 11, 62, 72, C_STAR
        ; '새로 시작' 5자 = 60px, x=(256-60)/2=98
        stdcall draw_hud_text, str_title_start+1, 5, 98, 108, C_WIN
        ret
endp
