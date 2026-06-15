; ─────────────────────────────────────────────────────────────────────────
; game_defs.inc — 게임 공유 상수 (Win32 비의존).
; PE 빌드(main.asm)와 헤드리스 ELF 하네스(_render_harness*.asm)가 함께 include.
; ─────────────────────────────────────────────────────────────────────────

; ── 화면 ──────────────────────────────────────────────
FB_W      = 256
FB_H      = 192
SCALE     = 4
CLIENT_W  = FB_W * SCALE        ; 1024
CLIENT_H  = FB_H * SCALE        ; 768

GRID_COLS = 16
GRID_ROWS = 12
TILE      = 16
HUD_ROWS  = 1                   ; 상단 UI 1행 → 플레이 영역 rows 1..11

; ── 타일 ID ───────────────────────────────────────────
T_GROUND  = 0
T_PATH    = 1
T_BUILD   = 2
T_ROSE    = 5

; ── 게임 상태 (check-state.py 단언값과 일치)
GS_PLAY   = 0
GS_WON    = 1
GS_LOST   = 2
; 타이틀/일시정지/코드입력은 main.asm에서만 쓰임 (하네스 무관)
GS_TITLE      = 3
GS_PAUSE      = 4
GS_CODE_INPUT = 5

; ── 타워 type ID ──────────────────────────────────────
TW_NONE    = 0
TW_SHEEP   = 1                  ; 🐑양
TW_VOLCANO = 2                  ; 🌋화산
TW_LAMP    = 3                  ; 💡가로등
TW_WIND    = 4                  ; 🏜️사막바람
TW_FOX     = 5                  ; 🦊여우

; tower_def 테이블 인덱스 (0-based, TW_SHEEP-1 = 0)
TD_COST      = 0                ; word (2B)
TD_DAMAGE    = 2                ; byte
TD_COOLDOWN  = 3                ; byte  (frames)
TD_RANGE     = 4                ; byte  (tiles, Chebyshev)
TD_FLAGS     = 5                ; byte
TD_SIZE      = 6                ; 바이트/엔트리

; 타워 플래그 (docs/10)
TOWER_DAMAGE = 0x01
TOWER_SPLASH = 0x02
TOWER_SLOW   = 0x04
TOWER_BOOST  = 0x08

; ── 적 type ID ────────────────────────────────────────
EN_SPROUT   = 1                 ; 🌱싹
EN_SEEDLING = 2                 ; 🌿새싹
EN_SAPLING  = 3                 ; 🪴묘목
EN_TREE     = 4                 ; 🌳거목
EN_SNAKE    = 5                 ; 🐍뱀
EN_VAIN     = 6                 ; 🎩허영쟁이
EN_BIZ      = 7                 ; 💼사업가
EN_DRUNK    = 8                 ; 🍷술꾼
EN_TREE_BOSS= 9                 ; 🌳거대 거목 보스 (4-11)

; enemy_def 테이블 인덱스 (0-based, EN_SPROUT-1 = 0)
ED_HP        = 0                ; byte
ED_SPEED     = 1                ; byte  (speed_fx ÷ 2 — 0..255 range, multiply ×2)
; speed_fx = ED_SPEED × 2, e.g. 싹 179 → store 89 (×2=178)? No - better store as word.
; Let's store speed_fx as word:
ED_SPEED_LO  = 1                ; low byte of speed_fx
ED_SPEED_HI  = 2                ; high byte of speed_fx
ED_REWARD    = 3                ; byte
ED_LEAK      = 4                ; byte
ED_FLAGS     = 5                ; byte
ED_SIZE      = 6                ; 바이트/엔트리

; 적 플래그 (docs/11)
ENEMY_FAST     = 0x01
ENEMY_HEAVY    = 0x02
ENEMY_ARMORED  = 0x04
ENEMY_EVADE    = 0x08
ENEMY_AURA     = 0x10
ENEMY_NO_REWARD= 0x20
ENEMY_ERRATIC  = 0x40
ENEMY_IS_BOSS  = 0x80

; enemy/tower def 레코드 크기
ED_REC = 6
TD_REC = 6

; ── 효과 상수 (docs/10/11) ────────────────────────────
SLOW_FACTOR   = 128             ; ÷256 = 50% 속도
SLOW_DURATION = 60              ; 틱(1초)
BOOST_FACTOR  = 218             ; ÷256 ≈ 85% 쿨다운
WAVE_BONUS_DIV= 30              ; call_wave_early 보너스: timer/30

; ── 생명·경제 ─────────────────────────────────────────
PETALS_START   = 5
SELL_PCT       = 60             ; 판매 환급 %  (cost × 60% = cost*3/5)

; Easy 모드 초당 별빛 +2 (60Hz → 120틱마다 +1, 즉 30틱마다 +0.5 → 간단히 60틱마다 +1 ×2)
EASY_INCOME_INTERVAL = 30       ; 틱. 30틱마다 +1 → 60Hz에서 초당 +2

; ── 1-1 하드코딩 상수 (현재 단계, 범용화 전) ────────────
START_LIGHT   = 60
SHEEP_COST    = 20
SELL_REFUND   = 12              ; 20 × 60%
PETALS_START  = 5
WAVE_COUNT    = 8
SPAWN_DELAY   = 30
SPAWN_SPACING = 48
SPROUT_HP     = 2
SPROUT_SPEED  = 179
SPROUT_REWARD = 1
SPROUT_LEAK   = 1
ENEMY_Y       = 104             ; row6 center = 6*16+8
PATH_END_FX   = 248 * 256       ; col15 center × 256
SHEEP_DMG     = 1
SHEEP_CD      = 20
SHEEP_RANGE   = 3

; ── 배열 크기 ─────────────────────────────────────────
MAX_ENEMIES    = 16
MAX_PATH_SEGS  = 8              ; 경로 세그먼트 최대 수
MAX_BUILDS     = 4              ; BUILD 직사각형 최대 수
MAX_WAVES      = 6              ; 한 해넘이 WAVE 최대 수
MAX_LANES      = 2              ; T6 합류 맵용 레인 수

; ── 색 (0x00RRGGBB — 32bpp BI_RGB) ───────────────────
C_GROUND  = 0x00142A1E
C_PATH    = 0x00CBA96B
C_BUILD   = 0x001A2747
C_BUILDE  = 0x005D72AD
C_ROSE    = 0x00C81E32
C_HUD     = 0x00060A1A
C_SHEEP   = 0x00EDE7D6
C_CURSOR  = 0x00FFE9A8
C_STAR    = 0x00FFD24A
C_PETAL   = 0x00C81E32
C_WIN     = 0x00A8E060
C_LOSS    = 0x00E0463C
C_SPROUT  = 0x008CC152
C_SEEDLING= 0x004A9B3A
C_SAPLING = 0x007A5230
C_TREE    = 0x003C2810
C_SNAKE   = 0x00D4B830
C_HUMAN   = 0x00C8A060
; 타워 종류별 임시 색 (Phase 2에서 스프라이트로 교체)
C_VOLCANO = 0x00E0482C
C_LAMP    = 0x00FFE96B
C_WIND    = 0x009CC8D8
C_FOX     = 0x00E08A3A
C_LOCK    = 0x00303848         ; 잠긴 타워 팔레트 칩 (어둡게)

; ── HUD 타워 팔레트 레이아웃 (draw_hud + main.asm 오버레이 공유) ──
PAL_Y    = 2
PAL_SZ   = 12
PAL_X0   = 152
PAL_STEP = 19                  ; 칩 5개: x = 152,171,190,209,228 (마지막 끝 240)
