; ─────────────────────────────────────────────────────────────────────────
; 어린왕자: 장미의 별 — src/main.asm (플랫폼 레이어, Win32 PE)
; 창 생성 / 메시지 루프 / 입력 정규화 / StretchDIBits 출력만 담당.
; 게임 로직·렌더는 game.inc (OS API 비의존). 입력은 in_* 로 정규화해 game 에 넘긴다.
; 렌더/입력 실동작 검증은 Windows 환경 대기. 로직은 _render_harness.asm 로 맥에서 확인.
; ─────────────────────────────────────────────────────────────────────────
include 'win32ax.inc'
include 'game_defs.inc'

; winmm.dll 임포트 (SFX용)
; waveOutOpen, waveOutPrepareHeader, waveOutWrite, waveOutClose
; WAVEFORMATEX: PCM 11025Hz 8-bit mono
WAVE_FORMAT_PCM = 1
SFX_SAMPLE_RATE = 11025
SFX_CHANNELS    = 1
SFX_BITS        = 8
SFX_BUF_SIZE    = 2205              ; 0.2초 분량 (11025 × 0.2)

; SFX 이벤트 ID
SFX_PLACE   = 0
SFX_SELL    = 1
SFX_KILL    = 2
SFX_LEAK    = 3
SFX_WIN     = 4
SFX_LOSE    = 5
SFX_DENY    = 6

WIN_STYLE      = WS_CAPTION or WS_SYSMENU or WS_MINIMIZEBOX   ; 리사이즈 금지(D046)
DIB_RGB_COLORS = 0
SRCCOPY        = 0x00CC0020
; 마우스 client → framebuffer 타일: ÷SCALE(4) ÷TILE(16) = ÷64 = shr 6
CLICK_SHIFT    = 6

.data
  class_name   db 'RoseStarWindow',0
  window_title db 'The Rose Star',0
  hInstance    dd ?
  hwnd         dd ?
  hdc          dd ?
  win_w        dd ?
  win_h        dd ?
  msg          MSG
  wc           WNDCLASS
  rc           RECT
  ps           PAINTSTRUCT
  bmi          BITMAPINFOHEADER
  ; ── 게임 상태 (game.inc 가 참조하는 rw 데이터) ──
  align 4
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
  ; ── 적 슬롯 ──
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
  ; ── 타워 ──
  tower_cd     rd GRID_COLS * GRID_ROWS
  tower_boosted rb GRID_COLS * GRID_ROWS
  align 4
  ; ── 경로 세그먼트 ──
  seg_x1       rd MAX_PATH_SEGS
  seg_y1       rd MAX_PATH_SEGS
  seg_dx       rd MAX_PATH_SEGS
  seg_dy       rd MAX_PATH_SEGS
  seg_acc      rd MAX_PATH_SEGS
  seg_len      rd MAX_PATH_SEGS
  seg_count    dd ?
  path_end_px  dd ?
  ; ── 웨이브 큐 ──
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
  ; ── 게임 상태 ──
  cur_tower       dd TW_SHEEP
  speed_mul       dd 1
  num_buf         dw 8 dup(0)
  record_code_buf db 20 dup(0)
  code_input_buf  db 20 dup(0)
  code_input_len  dd 0
  code_input_err  dd 0
  towers_placed   dd 0
  sells_count     dd 0
  ; ── SFX (winmm 동적 로딩) ──
  sfx_muted       dd 0
  hWaveOut        dd 0
  sfx_buf         db SFX_BUF_SIZE dup(0)
  ; WAVEHDR (32바이트: lpData,dwBufferLength,dwBytesRecorded,dwUser,dwFlags,dwLoops,lpNext,reserved)
  sfx_hdr         dd sfx_buf, SFX_BUF_SIZE, 0, 0, 0, 0, 0, 0
  ; WAVEFORMATEX (18바이트)
  wfx             dw WAVE_FORMAT_PCM, SFX_CHANNELS
                  dd SFX_SAMPLE_RATE, SFX_SAMPLE_RATE
                  dw 1, SFX_BITS, 0
  ; winmm 함수 포인터 (동적 로딩)
  pfn_waveOutOpen          dd 0
  pfn_waveOutPrepareHeader dd 0
  pfn_waveOutWrite         dd 0
  pfn_waveOutClose         dd 0
  ; 문자열
  s_winmm         db 'winmm.dll', 0
  s_waveOutOpen   db 'waveOutOpen', 0
  s_waveOutPrepHdr db 'waveOutPrepareHeader', 0
  s_waveOutWrite  db 'waveOutWrite', 0
  s_waveOutClose  db 'waveOutClose', 0
  game_mode    dd ?
  easy_income_cd dd ?
  prng_state   dd ?
  in_callwave  dd ?
  boss_phase   dd ?
  to_spawn     dd ?
  spawn_cd     dd ?
  active_count dd ?
  petals       dd ?
  game_state   dd ?
  cur_level_idx dd 0
  cur_level_mask dd 0            ; 현재 해넘이 tower_mask (타워 선택 게이팅)
  share_dir    db 'share', 0
  fmt_filepath db 'share/%s.bmp', 0
  share_filepath db 128 dup(0)
  temp_utf16_buf dw 128 dup(0)
  card_save_status dd 0
  card_save_timer  dd 0
  align 2
  card_str_title:
      db 11
      dw 0xC5B4, 0xB9B0, 0xC655, 0xC790, 0x003A, 0x0020, 0xC7A5, 0xBBF8, 0xC758, 0x0020, 0xBCC4
  card_str_sunset:
      db 4
      dw 0xD574, 0xB118, 0xC774, 0x0020
  card_str_easy:
      db 2
      dw 0xC26C, 0xC6C0
  card_str_hard:
      db 3
      dw 0xC5B4, 0xB824, 0xC6C0
  card_str_petals:
      db 3
      dw 0xAF43, 0xC78E, 0x0020
  card_str_towers:
      db 3
      dw 0xD0C0, 0xC6CC, 0x0020
  card_str_sells:
      db 3
      dw 0xD310, 0xBAE4, 0x0020
  card_str_code:
      db 3
      dw 0xCF54, 0xB4DC, 0x0020
  card_str_save_ok:
      db 7
      dw 0xCE74, 0xB4DC, 0xB97C, 0x0020, 0xB0A8, 0xACBC, 0xB2E4
  card_str_save_fail:
      db 11
      dw 0xCE74, 0xB4DC, 0xB97C, 0x0020, 0xB0A8, 0xAE30, 0xC9C0, 0x0020, 0xBABB, 0xD588, 0xB2E4
  card_buffer rb 480 * 480 * 3


.code
include 'game.inc'
include 'font_data.inc'
include 'level_data.inc'

start:
        invoke  GetModuleHandle, 0
        mov     [hInstance], eax

        mov     [wc.style], CS_HREDRAW or CS_VREDRAW
        mov     [wc.lpfnWndProc], WindowProc
        mov     [wc.cbClsExtra], 0
        mov     [wc.cbWndExtra], 0
        mov     eax, [hInstance]
        mov     [wc.hInstance], eax
        mov     [wc.hIcon], 0
        invoke  LoadCursor, 0, IDC_ARROW
        mov     [wc.hCursor], eax
        mov     [wc.hbrBackground], 0
        mov     [wc.lpszMenuName], 0
        mov     [wc.lpszClassName], class_name
        invoke  RegisterClass, wc

        mov     [rc.left], 0
        mov     [rc.top], 0
        mov     [rc.right], CLIENT_W
        mov     [rc.bottom], CLIENT_H
        invoke  AdjustWindowRect, rc, WIN_STYLE, 0
        mov     eax, [rc.right]
        sub     eax, [rc.left]
        mov     [win_w], eax
        mov     eax, [rc.bottom]
        sub     eax, [rc.top]
        mov     [win_h], eax

        invoke  CreateWindowEx, 0, class_name, window_title, WIN_STYLE, \
                CW_USEDEFAULT, CW_USEDEFAULT, [win_w], [win_h], 0, 0, [hInstance], 0
        mov     [hwnd], eax

        mov     [bmi.biSize], sizeof.BITMAPINFOHEADER
        mov     [bmi.biWidth], FB_W
        mov     [bmi.biHeight], -FB_H           ; top-down
        mov     [bmi.biPlanes], 1
        mov     [bmi.biBitCount], 32
        mov     [bmi.biCompression], 0          ; BI_RGB
        mov     [bmi.biSizeImage], 0
        mov     [bmi.biXPelsPerMeter], 0
        mov     [bmi.biYPelsPerMeter], 0
        mov     [bmi.biClrUsed], 0
        mov     [bmi.biClrImportant], 0

        stdcall sfx_init
        ; 타이틀 화면으로 시작 (Space/Enter에서 init_level 호출)
        mov     dword [game_state], GS_TITLE
        mov     dword [game_mode], 0            ; 기본 쉬움 (타이틀 ←→ 로 토글)

        invoke  ShowWindow, [hwnd], SW_SHOW
        invoke  UpdateWindow, [hwnd]

  .loop:
        invoke  PeekMessage, msg, 0, 0, 0, PM_REMOVE
        test    eax, eax
        jz      .render
        cmp     [msg.message], WM_QUIT
        je      .done
        invoke  TranslateMessage, msg
        invoke  DispatchMessage, msg
        jmp     .loop
  .render:
        ; 일시정지 중엔 update 스킵 (입력만 처리됨)
        cmp     dword [game_state], GS_PAUSE
        je      .skip_update
        ; [F] 2배속
        stdcall update
        call    apply_easy_income
        cmp     dword [speed_mul], 1
        jle     .one_tick
        stdcall update
        call    apply_easy_income
  .one_tick:
  .skip_update:
        stdcall render_world
        stdcall draw_overlays          ; 타워 팔레트 잠금/비용 + 타이틀 모드
        ; 일시정지 오버레이
        cmp     dword [game_state], GS_PAUSE
        jne     .no_pause_overlay
        stdcall draw_hud_text, str_paused+1, 3, 92, 88, C_TEXT
  .no_pause_overlay:
        ; 공유 카드 저장 결과 오버레이
        cmp     dword [card_save_timer], 0
        jle     .no_save_overlay
        dec     dword [card_save_timer]
        cmp     dword [card_save_status], 1
        jne     .save_fail_draw
        stdcall draw_hud_text, card_str_save_ok+1, 7, 86, 110, C_WIN
        jmp     .no_save_overlay
  .save_fail_draw:
        cmp     dword [card_save_status], 2
        jne     .no_save_overlay
        stdcall draw_hud_text, card_str_save_fail+1, 11, 62, 110, C_LOSS
  .no_save_overlay:
        call    Present
        invoke  Sleep, 16
        jmp     .loop
  .done:
        invoke  ExitProcess, 0

; ── SFX ────────────────────────────────────────────
; sfx_play(event_id): 사각파 PCM 생성 후 비차단 재생
; freq표: docs/18 §5
sfx_freq_table:
    dd 523, 392, 1047, 147, 659, 330, 196   ; place,sell,kill,leak,win,lose,deny

proc sfx_init
        ; winmm.dll 동적 로딩
        invoke  LoadLibraryA, s_winmm
        test    eax, eax
        jz      .no_winmm
        ; eax = hWinmm
        mov     ebx, eax                        ; ebx = hWinmm
        invoke  GetProcAddress, ebx, s_waveOutOpen
        mov     [pfn_waveOutOpen], eax
        invoke  GetProcAddress, ebx, s_waveOutPrepHdr
        mov     [pfn_waveOutPrepareHeader], eax
        invoke  GetProcAddress, ebx, s_waveOutWrite
        mov     [pfn_waveOutWrite], eax
        invoke  GetProcAddress, ebx, s_waveOutClose
        mov     [pfn_waveOutClose], eax
        ; waveOut 열기
        cmp     dword [pfn_waveOutOpen], 0
        je      .no_winmm
        ; waveOutOpen(pHandle, WAVE_MAPPER, pFormat, 0, 0, 0)
        lea     eax, [hWaveOut]
        push    dword 0
        push    dword 0
        push    dword 0
        push    wfx
        push    dword -1
        push    eax
        call    dword [pfn_waveOutOpen]
        add     esp, 24
  .no_winmm:
        ret
endp

proc sfx_play uses eax ebx ecx edx, event_id
        cmp     dword [sfx_muted], 1
        je      .done
        cmp     dword [hWaveOut], 0
        je      .done
        mov     eax, [event_id]
        cmp     eax, 6
        jg      .done
        ; 주파수 가져오기
        mov     ecx, [sfx_freq_table + eax*4]   ; freq
        ; 사각파 생성: SFX_BUF_SIZE 샘플, 반주기마다 0x80↔0xFF 토글
        ; 반주기 = 샘플율 / (freq * 2)
        ; half_period = SFX_SAMPLE_RATE / (ecx * 2)
        push    ecx
        mov     eax, SFX_SAMPLE_RATE
        xor     edx, edx
        mov     ecx, [esp]
        shl     ecx, 1
        idiv    ecx
        pop     ecx
        ; eax = half_period
        cmp     eax, 0
        je      .done
        mov     ecx, eax                        ; half_period
        xor     ebx, ebx                        ; buf idx
        xor     edx, edx                        ; phase counter
        mov     ah, 0x80                        ; current level
  .gen:
        cmp     ebx, SFX_BUF_SIZE
        jge     .play
        mov     byte [sfx_buf + ebx], ah
        inc     edx
        cmp     edx, ecx
        jl      .no_toggle
        xor     edx, edx
        cmp     ah, 0x80
        je      .hi
        mov     ah, 0x80
        jmp     .no_toggle
  .hi:  mov     ah, 0xFF
  .no_toggle:
        inc     ebx
        jmp     .gen
  .play:
        ; 선형 감쇠 (끝으로 갈수록 0x80으로 수렴)
        xor     ebx, ebx
  .fade:
        cmp     ebx, SFX_BUF_SIZE
        jge     .submit
        movzx   eax, byte [sfx_buf + ebx]
        sub     eax, 0x80
        ; scale: eax * (SFX_BUF_SIZE - ebx) / SFX_BUF_SIZE
        imul    eax, SFX_BUF_SIZE
        sub     eax, ebx
        mov     ecx, SFX_BUF_SIZE
        cdq
        idiv    ecx
        add     eax, 0x80
        ; clamp 0~255
        cmp     eax, 0
        jge     @f
        xor     eax, eax
  @@:   cmp     eax, 255
        jle     @f
        mov     eax, 255
  @@:   mov     byte [sfx_buf + ebx], al
        inc     ebx
        jmp     .fade
  .submit:
        cmp     dword [pfn_waveOutPrepareHeader], 0
        je      .done
        ; WAVEHDR dwFlags = 0 (재사용 시 unprepare 필요하지만 단순 버전)
        mov     dword [sfx_hdr + 16], 0         ; dwFlags
        push    dword 32
        push    sfx_hdr
        push    dword [hWaveOut]
        call    dword [pfn_waveOutPrepareHeader]
        add     esp, 12
        push    dword 32
        push    sfx_hdr
        push    dword [hWaveOut]
        call    dword [pfn_waveOutWrite]
        add     esp, 12
  .done:
        ret
endp

; ── 프레임버퍼를 4배 확대 출력 (nearest, HALFTONE 금지=D072) ──
proc Present
        invoke  GetDC, [hwnd]
        mov     [hdc], eax
        invoke  StretchDIBits, [hdc], 0, 0, CLIENT_W, CLIENT_H, \
                0, 0, FB_W, FB_H, framebuffer, bmi, DIB_RGB_COLORS, SRCCOPY
        invoke  ReleaseDC, [hwnd], [hdc]
        ret
endp

proc find_glyph uses ecx esi, cp
        movzx   eax, word [cp]
        mov     esi, font_glyph_table
        mov     ecx, FONT_GLYPH_COUNT
  .loop:
        cmp     ecx, 0
        je      .not_found
        cmp     word [esi], ax
        je      .found
        add     esi, 26
        dec     ecx
        jmp     .loop
  .found:
        lea     eax, [esi + 2]
        ret
  .not_found:
        xor     eax, eax
        ret
endp

proc draw_card_char uses eax ebx ecx edx esi edi, cp, cx_val, cy_val, color
        stdcall find_glyph, [cp]
        test    eax, eax
        jz      .done
        mov     esi, eax                        ; esi = bitmap ptr
        xor     edx, edx                        ; row = 0
  .row_loop:
        cmp     edx, 12
        jge     .done
        ; 현재 row의 16비트 로드 (빅엔디안 12비트 저장 형태 대응)
        movzx   eax, byte [esi + edx*2]
        shl     eax, 8
        mov     al, byte [esi + edx*2 + 1]      ; ax = 16bit row data
        
        xor     ecx, ecx                        ; col = 0
  .col_loop:
        cmp     ecx, 12
        jge     .row_end
        ; ax의 비트 검사 (왼쪽 b15부터 오른쪽 b4까지)
        mov     ebx, 15
        sub     ebx, ecx
        bt      eax, ebx
        jnc     .bit_zero
        
        ; 클리핑 및 픽셀 칠하기
        mov     edi, [cy_val]
        add     edi, edx
        cmp     edi, 0
        jl      .bit_zero
        cmp     edi, 480
        jge     .bit_zero
        
        mov     ebx, [cx_val]
        add     ebx, ecx
        cmp     ebx, 0
        jl      .bit_zero
        cmp     ebx, 480
        jge     .bit_zero
        
        imul    edi, 480
        add     edi, ebx
        imul    edi, 3
        lea     edi, [card_buffer + edi]
        
        mov     ebx, [color]
        mov     [edi], bl       ; B
        mov     [edi+1], bh     ; G
        shr     ebx, 16
        mov     [edi+2], bl     ; R
        
  .bit_zero:
        inc     ecx
        jmp     .col_loop
  .row_end:
        inc     edx
        jmp     .row_loop
  .done:
        ret
endp

proc draw_card_text uses eax ebx ecx edx esi edi, pStr, len, cx_val, cy_val, color
        mov     esi, [pStr]
        mov     ecx, [len]
        mov     edx, [cx_val]                   ; x
  .loop:
        cmp     ecx, 0
        je      .done
        movzx   eax, word [esi]                 ; cp
        push    ecx esi edx
        stdcall draw_card_char, eax, edx, [cy_val], [color]
        pop     edx esi ecx
        add     edx, 12                         ; 12px 고정폭
        add     esi, 2                          ; UTF-16 = 2B
        dec     ecx
        jmp     .loop
  .done:
        ret
endp

proc save_share_card uses eax ebx ecx edx esi edi
        locals
          moon dd ?
          sunset dd ?
          hFile dd ?
          bytes_written dd ?
          src_x dd ?
          src_y dd ?
          bmp_file_hdr db 14 dup(?)
          bmp_info_hdr db 40 dup(?)
        endl
        
        ; 1. 달/해넘이 계산
        mov     eax, [cur_level_idx]
        xor     edx, edx
        mov     ecx, 11
        div     ecx
        inc     eax             ; moon
        inc     edx             ; sunset
        mov     [moon], eax
        mov     [sunset], edx
        
        ; 2. encode_record 호출
        stdcall encode_record, [moon], [sunset], [game_mode], [petals], [towers_placed], [sells_count]
        
        ; 3. share 디렉토리 생성
        invoke  CreateDirectoryA, share_dir, 0
        
        ; 4. 파일명 구성
        invoke  wsprintfA, share_filepath, fmt_filepath, record_code_buf
        
        ; 5. 파일 생성
        invoke  CreateFileA, share_filepath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
        cmp     eax, -1 ; INVALID_HANDLE_VALUE
        je      .fail
        mov     [hFile], eax
        
        ; 6. 배경 색상 채우기 (C_GROUND BGR: 0x1E, 0x2A, 0x14)
        mov     edi, card_buffer
        mov     ecx, 480 * 480
  .fill_bg:
        mov     byte [edi], 0x1E  ; B
        mov     byte [edi+1], 0x2A ; G
        mov     byte [edi+2], 0x14 ; R
        add     edi, 3
        dec     ecx
        jnz     .fill_bg
        
        ; 7. 프레임버퍼 1.5배 nearest neighbor 스냅샷 복사 (dst 가로 384, 세로 288)
        xor     esi, esi        ; esi = dst_y
  .snap_y_loop:
        cmp     esi, 288
        jge     .snap_y_done
        
        ; src_y = dst_y * 2 / 3
        mov     eax, esi
        shl     eax, 1
        xor     edx, edx
        mov     ecx, 3
        div     ecx
        mov     [src_y], eax
        
        xor     edi, edi        ; edi = dst_x
  .snap_x_loop:
        cmp     edi, 384
        jge     .snap_x_done
        
        ; src_x = dst_x * 2 / 3
        mov     eax, edi
        shl     eax, 1
        xor     edx, edx
        mov     ecx, 3
        div     ecx
        mov     [src_x], eax
        
        ; framebuffer 픽셀 읽기
        mov     eax, [src_y]
        shl     eax, 8          ; src_y * 256
        add     eax, [src_x]
        mov     ebx, [framebuffer + eax*4]
        
        ; card_buffer 오프셋: ((dst_y + 96) * 480 + (dst_x + 48)) * 3
        mov     eax, esi
        add     eax, 96
        imul    eax, 480
        add     eax, edi
        add     eax, 48
        imul    eax, 3
        lea     ecx, [card_buffer + eax]
        
        mov     [ecx], bl       ; B
        mov     [ecx+1], bh     ; G
        shr     ebx, 16
        mov     [ecx+2], bl     ; R
        
        inc     edi
        jmp     .snap_x_loop
  .snap_x_done:
        inc     esi
        jmp     .snap_y_loop
  .snap_y_done:
        
        ; 8. 텍스트 그리기
        ; 제목: '어린왕자: 장미의 별' (x=48, y=32, 색상=0x00FFFFFF)
        stdcall draw_card_text, card_str_title+1, 11, 48, 32, 0x00FFFFFF
        
        ; 위치: '해넘이 M-S'
        lea     edi, [temp_utf16_buf]
        lea     esi, [card_str_sunset+1]
        cld
        movsd
        movsd
        
        ; moon 숫자
        stdcall num_to_u16, [moon]
        mov     ecx, eax
        lea     esi, [num_buf]
  .copy_moon:
        movsw
        dec     ecx
        jnz     .copy_moon
        
        ; '-' (0x002D)
        mov     word [edi], 0x002D
        add     edi, 2
        
        ; sunset 숫자
        stdcall num_to_u16, [sunset]
        mov     ecx, eax
        lea     esi, [num_buf]
  .copy_sunset:
        movsw
        dec     ecx
        jnz     .copy_sunset
        
        lea     eax, [temp_utf16_buf]
        sub     edi, eax
        shr     edi, 1                  ; edi = 총 글자수
        stdcall draw_card_text, temp_utf16_buf, edi, 48, 56, 0x00FFFFFF
        
        ; 모드: '쉬움' 또는 '어려움'
        cmp     [game_mode], 0
        jne     .mode_hard
        stdcall draw_card_text, card_str_easy+1, 2, 384, 56, 0x00FFFFFF
        jmp     .mode_done
  .mode_hard:
        stdcall draw_card_text, card_str_hard+1, 3, 384, 56, 0x00FFFFFF
  .mode_done:
        
        ; 꽃잎 N
        lea     edi, [temp_utf16_buf]
        lea     esi, [card_str_petals+1]
        cld
        movsd
        movsw
        stdcall num_to_u16, [petals]
        mov     ecx, eax
        lea     esi, [num_buf]
  .copy_petals:
        movsw
        dec     ecx
        jnz     .copy_petals
        lea     eax, [temp_utf16_buf]
        sub     edi, eax
        shr     edi, 1
        stdcall draw_card_text, temp_utf16_buf, edi, 48, 400, 0x00FFFFFF
        
        ; 타워 N
        lea     edi, [temp_utf16_buf]
        lea     esi, [card_str_towers+1]
        cld
        movsd
        movsw
        stdcall num_to_u16, [towers_placed]
        mov     ecx, eax
        lea     esi, [num_buf]
  .copy_towers:
        movsw
        dec     ecx
        jnz     .copy_towers
        lea     eax, [temp_utf16_buf]
        sub     edi, eax
        shr     edi, 1
        stdcall draw_card_text, temp_utf16_buf, edi, 176, 400, 0x00FFFFFF
        
        ; 판매 N
        lea     edi, [temp_utf16_buf]
        lea     esi, [card_str_sells+1]
        cld
        movsd
        movsw
        stdcall num_to_u16, [sells_count]
        mov     ecx, eax
        lea     esi, [num_buf]
  .copy_sells:
        movsw
        dec     ecx
        jnz     .copy_sells
        lea     eax, [temp_utf16_buf]
        sub     edi, eax
        shr     edi, 1
        stdcall draw_card_text, temp_utf16_buf, edi, 304, 400, 0x00FFFFFF
        
        ; 코드 RS-...
        lea     edi, [temp_utf16_buf]
        lea     esi, [card_str_code+1]
        cld
        movsd
        movsw
        lea     esi, [record_code_buf]
  .copy_code_loop:
        movzx   eax, byte [esi]
        test    al, al
        jz      .copy_code_done
        mov     [edi], ax
        add     edi, 2
        inc     esi
        jmp     .copy_code_loop
  .copy_code_done:
        lea     eax, [temp_utf16_buf]
        sub     edi, eax
        shr     edi, 1
        stdcall draw_card_text, temp_utf16_buf, edi, 48, 424, 0x00FFD24A ; C_STAR
        
        ; 9. BITMAPFILEHEADER 및 BITMAPINFOHEADER 파일 쓰기
        lea     edx, [bmp_file_hdr]
        mov     word [edx], 0x4D42 ; "BM"
        mov     dword [edx+2], 14 + 40 + 480 * 480 * 3
        mov     word [edx+6], 0
        mov     word [edx+8], 0
        mov     dword [edx+10], 54
        
        lea     eax, [bytes_written]
        invoke  WriteFile, [hFile], edx, 14, eax, 0
        
        lea     edx, [bmp_info_hdr]
        mov     dword [edx], 40
        mov     dword [edx+4], 480
        mov     dword [edx+8], 480
        mov     word [edx+12], 1
        mov     word [edx+14], 24
        mov     dword [edx+16], 0
        mov     dword [edx+20], 480 * 480 * 3
        mov     dword [edx+24], 0
        mov     dword [edx+28], 0
        mov     dword [edx+32], 0
        mov     dword [edx+36], 0
        
        lea     eax, [bytes_written]
        invoke  WriteFile, [hFile], edx, 40, eax, 0
        
        ; 10. 픽셀 데이터를 뒤집어서 쓰기 (Top-down -> Bottom-up)
        mov     esi, 479        ; y = 479
  .write_rows:
        cmp     esi, 0
        jl      .write_rows_done
        
        mov     eax, esi
        imul    eax, 1440
        lea     ebx, [card_buffer + eax]
        
        lea     ecx, [bytes_written]
        invoke  WriteFile, [hFile], ebx, 1440, ecx, 0
        
        dec     esi
        jmp     .write_rows
  .write_rows_done:
        
        invoke  CloseHandle, [hFile]
        
        mov     dword [card_save_status], 1
        mov     dword [card_save_timer], 90
        ret
        
  .fail:
        mov     dword [card_save_status], 2
        mov     dword [card_save_timer], 90
        ret
endp


; lparam(client 좌표) → in_cursor 타일. (x=low16, y=high16) ÷64
proc set_cursor_from_lparam, lp
        mov     eax, [lp]
        movzx   ecx, ax                         ; x
        shr     eax, 16                          ; y (상위워드)
        shr     ecx, CLICK_SHIFT
        mov     [in_cursor_cx], ecx
        shr     eax, CLICK_SHIFT
        mov     [in_cursor_cy], eax
        ret
endp

; ── 레벨 진입: init_level + tower_mask 로드 + 기본 타워 선택 ──
; 해넘이를 시작할 땐 항상 이 래퍼를 거친다(하네스는 init_level 직접 호출).
proc enter_level uses esi, idx
        mov     eax, [idx]
        mov     [cur_level_idx], eax
        stdcall init_level, eax
        ; tower_mask = level_table[idx][0]
        mov     eax, [cur_level_idx]
        mov     esi, [level_table + eax*4]
        movzx   eax, byte [esi]
        mov     [cur_level_mask], eax
        call    select_first_tower
        ret
endp

; cur_level_mask 에서 가장 낮은 해금 타워를 cur_tower 로. (최소 양은 항상 해금)
proc select_first_tower uses eax ecx edx
        mov     ecx, TW_SHEEP
  .l:
        mov     eax, 1
        shl     eax, cl
        test    [cur_level_mask], eax
        jnz     .found
        inc     ecx
        cmp     ecx, TW_FOX
        jle     .l
        mov     ecx, TW_SHEEP                   ; 안전망: 아무것도 없으면 양
  .found:
        mov     [cur_tower], ecx
        ret
endp

; cur_tower 를 다음 해금 타워로 순환 (최대 5회 시도).
proc cycle_tower uses eax ecx edx
        mov     ecx, [cur_tower]
        mov     edx, 5
  .nxt:
        inc     ecx
        cmp     ecx, TW_FOX
        jle     .chk
        mov     ecx, TW_SHEEP
  .chk:
        mov     eax, 1
        shl     eax, cl
        test    [cur_level_mask], eax
        jnz     .found
        dec     edx
        jnz     .nxt
        ret                                     ; 해금 타워 없음 → 유지
  .found:
        mov     [cur_tower], ecx
        ret
endp

; select_tower_n(n): n번 타워가 해금돼 있으면 cur_tower=n.
proc select_tower_n, n
        mov     ecx, [n]
        mov     eax, 1
        shl     eax, cl
        test    [cur_level_mask], eax
        jz      .no
        mov     eax, [n]
        mov     [cur_tower], eax
  .no:
        ret
endp

; Easy(game_mode=0) + GS_PLAY 일 때 별빛 자동 수입 (틱당 호출, 30틱마다 +1 → 초당 +2).
; sim(update) 밖에서 처리 → 헤드리스 하네스(Hard 경제)에 영향 없음.
proc apply_easy_income
        cmp     dword [game_mode], 0
        jne     .ret
        cmp     dword [game_state], GS_PLAY
        jne     .ret
        dec     dword [easy_income_cd]
        cmp     dword [easy_income_cd], 0
        jg      .ret
        add     dword [starlight], 1
        mov     dword [easy_income_cd], EASY_INCOME_INTERVAL
  .ret:
        ret
endp

; render_world 위에 그리는 플랫폼 오버레이:
;   GS_PLAY/PAUSE → 잠긴 타워 칩 덧칠(cur_level_mask) + 선택 타워 비용(HUD 빈칸)
;   GS_TITLE      → 쉬움/어려움 모드 선택 표시
proc draw_overlays uses eax ecx edx
        cmp     dword [game_state], GS_TITLE
        je      .title
        cmp     dword [game_state], GS_PLAY
        je      .play
        cmp     dword [game_state], GS_PAUSE
        je      .play
        ret
  .play:
        xor     ecx, ecx                        ; i = 0..4
  .dimloop:
        cmp     ecx, 5
        jge     .cost
        push    ecx
        lea     eax, [ecx+1]                    ; tower id (1..5)
        mov     edx, 1
        mov     cl, al
        shl     edx, cl                         ; 1 << id
        pop     ecx
        test    [cur_level_mask], edx
        jnz     .dimnext
        mov     eax, ecx
        imul    eax, PAL_STEP
        add     eax, PAL_X0
        push    ecx
        stdcall fill_rect, eax, PAL_Y, PAL_SZ, PAL_SZ, C_LOCK
        pop     ecx
  .dimnext:
        inc     ecx
        jmp     .dimloop
  .cost:
        ; 선택 타워 비용 (HUD 중앙 빈칸 x128)
        mov     eax, [cur_tower]
        dec     eax
        imul    eax, TD_REC
        movzx   eax, word [tower_def_table + eax]
        stdcall num_to_u16, eax
        stdcall draw_hud_text, num_buf, eax, 128, 2, C_STAR
        ret
  .title:
        ; 쉬움 / 어려움 — 선택 모드 강조
        cmp     dword [game_mode], 0
        jne     .t_hard
        stdcall draw_hud_text, card_str_easy+1, 2, 88, 128, C_WIN
        stdcall draw_hud_text, card_str_hard+1, 3, 132, 128, C_TEXT_DIM
        ret
  .t_hard:
        stdcall draw_hud_text, card_str_easy+1, 2, 88, 128, C_TEXT_DIM
        stdcall draw_hud_text, card_str_hard+1, 3, 132, 128, C_WIN
        ret
endp

; ── 윈도우 프로시저 ──
proc WindowProc, hw, wmsg, wparam, lparam
        mov     eax, [wmsg]
        cmp     eax, WM_DESTROY
        je      .destroy
        cmp     eax, WM_MOUSEMOVE
        je      .mmove
        cmp     eax, WM_LBUTTONDOWN
        je      .lbtn
        cmp     eax, WM_RBUTTONDOWN
        je      .rbtn
        cmp     eax, WM_KEYDOWN
        je      .keydown
        cmp     eax, WM_CHAR
        je      .wchar
        cmp     eax, WM_PAINT
        je      .paint
  .default:
        invoke  DefWindowProc, [hw], [wmsg], [wparam], [lparam]
        ret
  .mmove:
        stdcall set_cursor_from_lparam, [lparam]
        jmp     .zero
  .lbtn:
        ; HUD 행(상단)에서 타워 팔레트 클릭이면 종류 선택, 아니면 설치 의도.
        mov     eax, [lparam]
        movzx   ecx, ax                         ; client x
        shr     eax, 16                         ; client y
        shr     ecx, 2                          ; fb_x = client_x / 4
        shr     eax, 2                          ; fb_y = client_y / 4
        cmp     eax, TILE
        jge     .lbtn_play                      ; 플레이 영역(y>=16)
        sub     ecx, PAL_X0
        jl      .zero                           ; 팔레트 왼쪽(별빛/꽃잎) → 무시
        cmp     ecx, 5 * PAL_STEP
        jge     .zero                           ; 팔레트 오른쪽 → 무시
        mov     eax, ecx
        xor     edx, edx
        mov     ecx, PAL_STEP
        div     ecx                             ; eax = 칩 인덱스 0..4
        inc     eax                             ; 타워 id
        stdcall select_tower_n, eax
        jmp     .zero
  .lbtn_play:
        stdcall set_cursor_from_lparam, [lparam]
        mov     dword [in_place], 1
        jmp     .zero
  .rbtn:
        stdcall set_cursor_from_lparam, [lparam]
        mov     dword [in_sell], 1
        jmp     .zero
  .keydown:
        mov     eax, [wparam]
        ; GS_CODE_INPUT 전용 키 처리
        cmp     dword [game_state], GS_CODE_INPUT
        je      .kcode_dispatch
        cmp     eax, VK_ESCAPE
        je      .destroy
        cmp     eax, VK_RETURN
        je      .kplace
        cmp     eax, VK_LEFT
        je      .kleft
        cmp     eax, VK_RIGHT
        je      .kright
        cmp     eax, VK_UP
        je      .kup
        cmp     eax, VK_DOWN
        je      .kdown
        cmp     eax, VK_SPACE
        je      .kplace
        cmp     eax, 'S'
        je      .ks_key
        cmp     eax, 'R'
        je      .krestart
        cmp     eax, 'P'
        je      .kpause
        cmp     eax, 'F'
        je      .kfast
        cmp     eax, 'N'
        je      .kcallwave
        cmp     eax, 'M'
        je      .kmute
        cmp     eax, VK_TAB
        je      .ktab
        cmp     eax, '1'
        jb      .default
        cmp     eax, '5'
        ja      .default
        ; '1'..'5' → 타워 직접 선택
        sub     eax, '0'
        stdcall select_tower_n, eax
        jmp     .zero
  .ktab:
        call    cycle_tower
        jmp     .zero
  .kcode_dispatch:
        ; GS_CODE_INPUT: ESC→타이틀, ENTER→decode, BACKSPACE→삭제
        cmp     eax, VK_ESCAPE
        je      .kcode_cancel
        cmp     eax, VK_RETURN
        je      .kcode_confirm
        cmp     eax, VK_BACK
        je      .kcode_back
        jmp     .zero
  .kcode_cancel:
        mov     dword [game_state], GS_TITLE
        jmp     .zero
  .kcode_confirm:
        stdcall decode_record
        cmp     eax, -1
        je      .kcode_fail
        stdcall enter_level, eax
        mov     dword [game_state], GS_PLAY
        jmp     .zero
  .kcode_fail:
        mov     dword [code_input_err], 1
        jmp     .zero
  .kcode_back:
        cmp     dword [code_input_len], 0
        jle     .zero
        dec     dword [code_input_len]
        mov     ecx, [code_input_len]
        mov     byte [code_input_buf + ecx], 0
        jmp     .zero
  .ks_key:
        cmp     dword [game_state], GS_WON
        je      .ksave_card
        cmp     dword [game_state], GS_LOST
        je      .ksave_card
        jmp     .ksell
  .ksave_card:
        stdcall save_share_card
        jmp     .zero
  .krestart:
        ; 타이틀 화면에서는 R 무시
        cmp     dword [game_state], GS_TITLE
        je      .zero
        stdcall enter_level, [cur_level_idx]
        jmp     .zero
  .kpause:
        ; [P]: GS_PLAY↔GS_PAUSE 토글
        cmp     dword [game_state], GS_PLAY
        jne     .try_unpause
        mov     dword [game_state], GS_PAUSE
        jmp     .zero
  .try_unpause:
        cmp     dword [game_state], GS_PAUSE
        jne     .default
        mov     dword [game_state], GS_PLAY
        jmp     .zero
  .kfast:
        ; [F]: 속도 토글 1×↔2×
        cmp     dword [speed_mul], 1
        je      .set_fast
        mov     dword [speed_mul], 1
        jmp     .zero
  .set_fast:
        mov     dword [speed_mul], 2
        jmp     .zero
  .kcallwave:
        mov     dword [in_callwave], 1
        jmp     .zero
  .kmute:
        ; [M]: 음소거 토글
        xor     dword [sfx_muted], 1
        jmp     .zero
  .kleft:
        cmp     dword [game_state], GS_TITLE
        je      .mode_easy
        cmp     dword [in_cursor_cx], 0
        jle     .zero
        dec     dword [in_cursor_cx]
        jmp     .zero
  .mode_easy:
        mov     dword [game_mode], 0
        jmp     .zero
  .kright:
        cmp     dword [game_state], GS_TITLE
        je      .mode_hard
        cmp     dword [in_cursor_cx], GRID_COLS-1
        jge     .zero
        inc     dword [in_cursor_cx]
        jmp     .zero
  .mode_hard:
        mov     dword [game_mode], 1
        jmp     .zero
  .kup:
        cmp     dword [in_cursor_cy], HUD_ROWS
        jle     .zero
        dec     dword [in_cursor_cy]
        jmp     .zero
  .kdown:
        cmp     dword [in_cursor_cy], GRID_ROWS-1
        jge     .zero
        inc     dword [in_cursor_cy]
        jmp     .zero
  .kplace:
        ; GS_TITLE → 레벨 0부터 새로 시작
        cmp     dword [game_state], GS_TITLE
        jne     .kplace_notatitle
        stdcall enter_level, 0
        jmp     .zero
  .kplace_notatitle:
        ; GS_WON → 다음 레벨로 진행
        cmp     dword [game_state], GS_WON
        jne     .kplace_notwon
        mov     eax, [cur_level_idx]
        cmp     eax, 43
        jge     .kplace_endgame
        inc     dword [cur_level_idx]
        stdcall enter_level, [cur_level_idx]
        jmp     .zero
  .kplace_endgame:
        ; 4-11 클리어 → 타이틀로
        mov     dword [game_state], GS_TITLE
        jmp     .zero
  .kplace_notwon:
        ; GS_PLAY → 타워 설치 의도
        mov     dword [in_place], 1
        jmp     .zero
  .ksell:
        mov     dword [in_sell], 1
        jmp     .zero
  .wchar:
        ; WM_CHAR: GS_TITLE에서 'I'/'i' → 코드 입력 모드 진입
        ; (WM_KEYDOWN이 아닌 WM_CHAR에서 처리해 'I' 자체가 버퍼에 새어들지 않게 함)
        mov     eax, [wparam]
        cmp     dword [game_state], GS_TITLE
        jne     .wc_code_check
        cmp     eax, 'I'
        je      .wc_open
        cmp     eax, 'i'
        jne     .zero
  .wc_open:
        mov     dword [code_input_len], 0
        mov     byte  [code_input_buf], 0
        mov     dword [code_input_err], 0
        mov     dword [game_state], GS_CODE_INPUT
        jmp     .zero
  .wc_code_check:
        ; GS_CODE_INPUT: 출력 가능 ASCII 0x20~0x7E 저장
        cmp     dword [game_state], GS_CODE_INPUT
        jne     .zero
        cmp     eax, 0x20
        jl      .zero
        cmp     eax, 0x7E
        jg      .zero
        ; 최대 14자 (최장 코드 RS-4-11-H-FFFF = 13자, 여유 1자)
        cmp     dword [code_input_len], 14
        jge     .zero
        ; 소문자를 대문자로 변환
        cmp     eax, 'a'
        jl      .wc_store
        cmp     eax, 'z'
        jg      .wc_store
        sub     eax, 0x20
  .wc_store:
        mov     ecx, [code_input_len]
        mov     byte [code_input_buf + ecx], al
        inc     ecx
        mov     [code_input_len], ecx
        mov     byte [code_input_buf + ecx], 0
        mov     dword [code_input_err], 0
        jmp     .zero
  .destroy:
        invoke  PostQuitMessage, 0
        xor     eax, eax
        ret
  .paint:
        invoke  BeginPaint, [hw], ps
        invoke  StretchDIBits, [ps.hdc], 0, 0, CLIENT_W, CLIENT_H, \
                0, 0, FB_W, FB_H, framebuffer, bmi, DIB_RGB_COLORS, SRCCOPY
        invoke  EndPaint, [hw], ps
        xor     eax, eax
        ret
  .zero:
        xor     eax, eax
        ret
endp

.end start
