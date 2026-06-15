; ─────────────────────────────────────────────────────────────────────────
; level_data.inc — 44해넘이 정적 데이터 (docs/12 §44 해넘이 웨이브 스펙)
;
; 레벨 바이트 레이아웃:
;   [0]  tower_mask   ; 사용 가능 타워 bitmask (1 shl TW_x)
;   [1]  start_light  ; 시작 별빛
;   [2]  petals       ; 장미 꽃잎 (항상 5)
;   [3]  path_count   ; PATH 세그먼트 수
;   [4]  build_count  ; BUILD 직사각형 수
;   [5]  wave_count   ; WAVE 엔트리 수
;   [6]  flags        ; 예약
;   PATH: path_count × 4 bytes  (col, row, len, dir)
;   BUILD: build_count × 4 bytes (col, row, w, h)
;   WAVE:  wave_count × 5 bytes (type, count, spacing, delay_lo, delay_hi)
; ─────────────────────────────────────────────────────────────────────────

; WAVE 엔트리 헬퍼
macro WDEF _type, _count, _spacing, _delay {
    db _type, _count, _spacing
    dw _delay
}

; stub 레벨 (메커닉 미구현 해넘이 자리 표시자 — T0+싹×8)
macro STUB _light {
    db 0x1F, _light, 5, 1, 2, 1, 0
    db 0, 6, 16, 0
    db 2, 4, 12, 1
    db 2, 8, 12, 1
    WDEF EN_SPROUT, 8, 48, 30
}

; ────────────────────────── 달1 ─────────────────────────
; 1-1  T0 STRAIGHT FULL start=60  싹×8 sp48 d30  [LOCKED]
level_1_1:
    db (1 shl TW_SHEEP), 60, 5, 1, 2, 1, 0
    db 0, 6, 16, 0
    db 2, 4, 12, 1
    db 2, 8, 12, 1
    WDEF EN_SPROUT, 8, 48, 30

; 1-2  T0 STRAIGHT FULL start=60  싹×10 sp44  [READY]
level_1_2:
    db (1 shl TW_SHEEP), 60, 5, 1, 2, 1, 0
    db 0, 6, 16, 0
    db 2, 4, 12, 1
    db 2, 8, 12, 1
    WDEF EN_SPROUT, 10, 44, 0

; 1-3  T2 LCORNER FULL start=60  싹×12 sp44  [READY]
level_1_3:
    db (1 shl TW_SHEEP), 60, 5, 3, 3, 1, 0
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1
    db 6, 5, 4, 3
    db 11, 7, 4, 1
    WDEF EN_SPROUT, 12, 44, 0

; 1-4  T2 LCORNER FULL start=65  싹×7 sp40 d0; 싹×7 sp40 d120  [READY]
level_1_4:
    db (1 shl TW_SHEEP), 65, 5, 3, 3, 2, 0
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1
    db 6, 5, 4, 3
    db 11, 7, 4, 1
    WDEF EN_SPROUT, 7, 40, 0
    WDEF EN_SPROUT, 7, 40, 120

; 1-5  T0 STRAIGHT FULL start=65  싹×16 sp32  [READY]
level_1_5:
    db (1 shl TW_SHEEP), 65, 5, 1, 2, 1, 0
    db 0, 6, 16, 0
    db 2, 4, 12, 1
    db 2, 8, 12, 1
    WDEF EN_SPROUT, 16, 32, 0

; 1-6 T1 LOWLINE start=65 뱀×4 sp60 d0 [DRAFT — EVADE 미구현]
level_1_6:
    db (1 shl TW_SHEEP), 65, 5, 1, 2, 1, 0
    db 0, 9, 16, 0              ; T1: row9 가로 직선
    db 2, 7, 12, 1              ; 경로 위쪽 행7
    db 2, 5, 12, 1              ; 경로 위쪽 행5 (공간 더 넓음)
    WDEF EN_SNAKE, 4, 60, 0

; 1-7 T2 LCORNER start=70 싹×8 sp40 + 뱀×3 sp70 d150 [DRAFT]
level_1_7:
    db (1 shl TW_SHEEP), 70, 5, 3, 3, 2, 0
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1
    db 6, 5, 4, 3
    db 11, 7, 4, 1
    WDEF EN_SPROUT, 8, 40, 0
    WDEF EN_SNAKE, 3, 70, 150

; 1-8 T3 SCURVE start=70 싹×12 sp36 + 뱀×2 sp80 d160 [DRAFT]
level_1_8:
    db (1 shl TW_SHEEP), 70, 5, 3, 2, 2, 0
    db 0, 3, 8, 0
    db 7, 3, 6, 1
    db 7, 8, 9, 0
    db 1, 5, 5, 4
    db 9, 5, 6, 2
    WDEF EN_SPROUT, 12, 36, 0
    WDEF EN_SNAKE, 2, 80, 160

; 1-9 T5 SWITCHBACK start=70 싹×14 sp40 [DRAFT]
level_1_9:
    db (1 shl TW_SHEEP), 70, 5, 5, 2, 1, 0
    db 0, 2, 15, 0
    db 14, 2, 4, 1
    db 14, 5, 14, 2
    db 1, 5, 4, 1
    db 1, 8, 15, 0
    db 3, 3, 9, 2
    db 3, 6, 9, 2
    WDEF EN_SPROUT, 14, 40, 0

; 1-10 T2 HALF start=70 싹×12 sp40 + 뱀×2 sp80 d160 [DRAFT]
level_1_10:
    db (1 shl TW_SHEEP), 70, 5, 3, 1, 2, 0   ; build_count=1 (HALF)
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1              ; HALF: 첫 BUILD만
    WDEF EN_SPROUT, 12, 40, 0
    WDEF EN_SNAKE, 2, 80, 160

; 1-11 T3 SCURVE start=75 싹×10 sp36 + 뱀×4 sp60 d140 + 싹×6 sp36 d300 [DRAFT]
level_1_11:
    db (1 shl TW_SHEEP), 75, 5, 3, 2, 3, 0
    db 0, 3, 8, 0
    db 7, 3, 6, 1
    db 7, 8, 9, 0
    db 1, 5, 5, 4
    db 9, 5, 6, 2
    WDEF EN_SPROUT, 10, 36, 0
    WDEF EN_SNAKE, 4, 60, 140
    WDEF EN_SPROUT, 6, 36, 300

; ────────────────────────── 달2 ─────────────────────────
; 달2 타워: 양+화산(2-1부터), 가로등(2-6부터)
MASK_MOON2_EARLY = (1 shl TW_SHEEP) or (1 shl TW_VOLCANO)
MASK_MOON2_LATE  = MASK_MOON2_EARLY or (1 shl TW_LAMP)

; 2-1 T0 start=90 화산+새싹 도입 새싹×8 sp48 [DRAFT]
level_2_1:
    db MASK_MOON2_EARLY, 90, 5, 1, 2, 1, 0
    db 0, 6, 16, 0
    db 2, 4, 12, 1
    db 2, 8, 12, 1
    WDEF EN_SEEDLING, 8, 48, 0

; 2-2 T1 LOWLINE start=90 싹×10 sp32 + 새싹×4 sp60 d150 [DRAFT]
level_2_2:
    db MASK_MOON2_EARLY, 90, 5, 1, 2, 2, 0
    db 0, 9, 16, 0
    db 2, 7, 12, 1
    db 2, 5, 12, 1
    WDEF EN_SPROUT, 10, 32, 0
    WDEF EN_SEEDLING, 4, 60, 150

; 2-3 T2 LCORNER start=90 새싹×10 sp44 [DRAFT]
level_2_3:
    db MASK_MOON2_EARLY, 90, 5, 3, 3, 1, 0
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1
    db 6, 5, 4, 3
    db 11, 7, 4, 1
    WDEF EN_SEEDLING, 10, 44, 0

; 2-4 T3 SCURVE start=95 새싹×8 sp40 + 싹×8 sp32 d160 [DRAFT]
level_2_4:
    db MASK_MOON2_EARLY, 95, 5, 3, 2, 2, 0
    db 0, 3, 8, 0
    db 7, 3, 6, 1
    db 7, 8, 9, 0
    db 1, 5, 5, 4
    db 9, 5, 6, 2
    WDEF EN_SEEDLING, 8, 40, 0
    WDEF EN_SPROUT, 8, 32, 160

; 2-5 T4 DOUBLES start=95 새싹×6 sp48 + 싹×12 sp32 d180 [DRAFT]
level_2_5:
    db MASK_MOON2_EARLY, 95, 5, 5, 3, 2, 0
    db 0, 2, 6, 0
    db 5, 2, 5, 1
    db 5, 6, 6, 0
    db 10, 6, 5, 1
    db 10, 10, 6, 0
    db 1, 4, 3, 6
    db 6, 8, 3, 2
    db 11, 7, 4, 2
    WDEF EN_SEEDLING, 6, 48, 0
    WDEF EN_SPROUT, 12, 32, 180

; 2-6 T5 SWITCHBACK start=100 가로등 도입 새싹×10 sp44 [DRAFT]
level_2_6:
    db MASK_MOON2_LATE, 100, 5, 5, 2, 1, 0
    db 0, 2, 15, 0
    db 14, 2, 4, 1
    db 14, 5, 14, 2
    db 1, 5, 4, 1
    db 1, 8, 15, 0
    db 3, 3, 9, 2
    db 3, 6, 9, 2
    WDEF EN_SEEDLING, 10, 44, 0

; 2-7 T0 start=100 새싹×12 sp40 + 뱀×3 sp70 d200 [DRAFT]
level_2_7:
    db MASK_MOON2_LATE, 100, 5, 1, 2, 2, 0
    db 0, 6, 16, 0
    db 2, 4, 12, 1
    db 2, 8, 12, 1
    WDEF EN_SEEDLING, 12, 40, 0
    WDEF EN_SNAKE, 3, 70, 200

; 2-8 T2 start=100 새싹×8 sp44 + 뱀×4 sp60 d150 [DRAFT]
level_2_8:
    db MASK_MOON2_LATE, 100, 5, 3, 3, 2, 0
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1
    db 6, 5, 4, 3
    db 11, 7, 4, 1
    WDEF EN_SEEDLING, 8, 44, 0
    WDEF EN_SNAKE, 4, 60, 150

; 2-9 T3 HALF start=100 새싹×10 sp36 + 새싹×6 sp36 d250 [DRAFT]
level_2_9:
    db MASK_MOON2_LATE, 100, 5, 3, 1, 2, 0
    db 0, 3, 8, 0
    db 7, 3, 6, 1
    db 7, 8, 9, 0
    db 1, 5, 5, 4
    WDEF EN_SEEDLING, 10, 36, 0
    WDEF EN_SEEDLING, 6, 36, 250

; 2-10 T4 start=95 새싹×14 sp40 + 뱀×3 sp70 d260 [DRAFT]
level_2_10:
    db MASK_MOON2_LATE, 95, 5, 5, 3, 2, 0
    db 0, 2, 6, 0
    db 5, 2, 5, 1
    db 5, 6, 6, 0
    db 10, 6, 5, 1
    db 10, 10, 6, 0
    db 1, 4, 3, 6
    db 6, 8, 3, 2
    db 11, 7, 4, 2
    WDEF EN_SEEDLING, 14, 40, 0
    WDEF EN_SNAKE, 3, 70, 260

; 2-11 T5 start=105 Endless 해금 새싹×10+뱀×4+새싹×8 [DRAFT]
level_2_11:
    db MASK_MOON2_LATE, 105, 5, 5, 2, 3, 0
    db 0, 2, 15, 0
    db 14, 2, 4, 1
    db 14, 5, 14, 2
    db 1, 5, 4, 1
    db 1, 8, 15, 0
    db 3, 3, 9, 2
    db 3, 6, 9, 2
    WDEF EN_SEEDLING, 10, 36, 0
    WDEF EN_SNAKE, 4, 60, 160
    WDEF EN_SEEDLING, 8, 40, 320

; ────────────────────────── 달3 ─────────────────────────
MASK_MOON3_EARLY = MASK_MOON2_LATE
MASK_MOON3_LATE  = MASK_MOON3_EARLY or (1 shl TW_WIND)

; 3-1 T0 start=130 묘목×6 sp60 [DRAFT]
level_3_1:
    db MASK_MOON3_EARLY, 130, 5, 1, 2, 1, 0
    db 0, 6, 16, 0
    db 2, 4, 12, 1
    db 2, 8, 12, 1
    WDEF EN_SAPLING, 6, 60, 0

; 3-2 T2 start=130 묘목×5 sp60 + 새싹×6 sp40 d180 [DRAFT]
level_3_2:
    db MASK_MOON3_EARLY, 130, 5, 3, 3, 2, 0
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1
    db 6, 5, 4, 3
    db 11, 7, 4, 1
    WDEF EN_SAPLING, 5, 60, 0
    WDEF EN_SEEDLING, 6, 40, 180

; 3-3 T3 start=130 묘목×4 sp70 + 뱀×4 sp50 d150 [DRAFT]
level_3_3:
    db MASK_MOON3_EARLY, 130, 5, 3, 2, 2, 0
    db 0, 3, 8, 0
    db 7, 3, 6, 1
    db 7, 8, 9, 0
    db 1, 5, 5, 4
    db 9, 5, 6, 2
    WDEF EN_SAPLING, 4, 70, 0
    WDEF EN_SNAKE, 4, 50, 150

; 3-4 T5 start=135 묘목×6 sp60 + 새싹×8 sp36 d220 [DRAFT]
level_3_4:
    db MASK_MOON3_EARLY, 135, 5, 5, 2, 2, 0
    db 0, 2, 15, 0
    db 14, 2, 4, 1
    db 14, 5, 14, 2
    db 1, 5, 4, 1
    db 1, 8, 15, 0
    db 3, 3, 9, 2
    db 3, 6, 9, 2
    WDEF EN_SAPLING, 6, 60, 0
    WDEF EN_SEEDLING, 8, 36, 220

; 3-5 T4 start=135 묘목×5 sp60 + 뱀×5 sp50 d160 [DRAFT]
level_3_5:
    db MASK_MOON3_EARLY, 135, 5, 5, 3, 2, 0
    db 0, 2, 6, 0
    db 5, 2, 5, 1
    db 5, 6, 6, 0
    db 10, 6, 5, 1
    db 10, 10, 6, 0
    db 1, 4, 3, 6
    db 6, 8, 3, 2
    db 11, 7, 4, 2
    WDEF EN_SAPLING, 5, 60, 0
    WDEF EN_SNAKE, 5, 50, 160

; 3-6 T1 start=140 사막바람 도입 묘목×6 sp60 [DRAFT]
level_3_6:
    db MASK_MOON3_LATE, 140, 5, 1, 2, 1, 0
    db 0, 9, 16, 0
    db 2, 7, 12, 1
    db 2, 5, 12, 1
    WDEF EN_SAPLING, 6, 60, 0

; 3-7 T2 start=140 묘목×6 sp56 + 뱀×4 sp50 d180 [DRAFT]
level_3_7:
    db MASK_MOON3_LATE, 140, 5, 3, 3, 2, 0
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1
    db 6, 5, 4, 3
    db 11, 7, 4, 1
    WDEF EN_SAPLING, 6, 56, 0
    WDEF EN_SNAKE, 4, 50, 180

; 3-8 T3 start=140 묘목×5 sp60 + 새싹×10 sp32 d200 [DRAFT]
level_3_8:
    db MASK_MOON3_LATE, 140, 5, 3, 2, 2, 0
    db 0, 3, 8, 0
    db 7, 3, 6, 1
    db 7, 8, 9, 0
    db 1, 5, 5, 4
    db 9, 5, 6, 2
    WDEF EN_SAPLING, 5, 60, 0
    WDEF EN_SEEDLING, 10, 32, 200

; 3-9 T2 SPOTS start=135 묘목×6 + 뱀×4 [DRAFT]
level_3_9:
    db MASK_MOON3_LATE, 135, 5, 3, 1, 2, 0   ; SPOTS→build_count=1 (제한 설치)
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 6, 5, 4, 3
    WDEF EN_SAPLING, 6, 56, 0
    WDEF EN_SNAKE, 4, 50, 200

; 3-10 T5 start=145 묘목×6+새싹×10+뱀×4 [DRAFT]
level_3_10:
    db MASK_MOON3_LATE, 145, 5, 5, 2, 3, 0
    db 0, 2, 15, 0
    db 14, 2, 4, 1
    db 14, 5, 14, 2
    db 1, 5, 4, 1
    db 1, 8, 15, 0
    db 3, 3, 9, 2
    db 3, 6, 9, 2
    WDEF EN_SAPLING, 6, 52, 0
    WDEF EN_SEEDLING, 10, 34, 220
    WDEF EN_SNAKE, 4, 50, 420

; 3-11 T4 start=150 Random 해금 묘목×6+뱀×5+새싹×10 [DRAFT]
level_3_11:
    db MASK_MOON3_LATE, 150, 5, 5, 3, 3, 0
    db 0, 2, 6, 0
    db 5, 2, 5, 1
    db 5, 6, 6, 0
    db 10, 6, 5, 1
    db 10, 10, 6, 0
    db 1, 4, 3, 6
    db 6, 8, 3, 2
    db 11, 7, 4, 2
    WDEF EN_SAPLING, 6, 56, 0
    WDEF EN_SNAKE, 5, 50, 180
    WDEF EN_SEEDLING, 10, 34, 360

; ────────────────────────── 달4 ─────────────────────────
MASK_MOON4_EARLY = MASK_MOON3_LATE or (1 shl TW_FOX)
MASK_MOON4_ALL   = MASK_MOON4_EARLY

; 4-1 T0 start=180 여우+허영쟁이 도입 허영×2 sp80 + 새싹×8 sp40 d160 [DRAFT]
level_4_1:
    db MASK_MOON4_EARLY, 180, 5, 1, 2, 2, 0
    db 0, 6, 16, 0
    db 2, 4, 12, 1
    db 2, 8, 12, 1
    WDEF EN_VAIN, 2, 80, 0
    WDEF EN_SEEDLING, 8, 40, 160

; 4-2 T2 start=180 허영×3 sp70 + 묘목×4 sp60 d180 [DRAFT]
level_4_2:
    db MASK_MOON4_EARLY, 180, 5, 3, 3, 2, 0
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1
    db 6, 5, 4, 3
    db 11, 7, 4, 1
    WDEF EN_VAIN, 3, 70, 0
    WDEF EN_SAPLING, 4, 60, 180

; 4-3 T3 start=180 허영×2 sp80 + 뱀×5 sp50 d160 [DRAFT]
level_4_3:
    db MASK_MOON4_EARLY, 180, 5, 3, 2, 2, 0
    db 0, 3, 8, 0
    db 7, 3, 6, 1
    db 7, 8, 9, 0
    db 1, 5, 5, 4
    db 9, 5, 6, 2
    WDEF EN_VAIN, 2, 80, 0
    WDEF EN_SNAKE, 5, 50, 160

; 4-4 T5 start=185 거목×3 sp90 [DRAFT]
level_4_4:
    db MASK_MOON4_ALL, 185, 5, 5, 2, 1, 0
    db 0, 2, 15, 0
    db 14, 2, 4, 1
    db 14, 5, 14, 2
    db 1, 5, 4, 1
    db 1, 8, 15, 0
    db 3, 3, 9, 2
    db 3, 6, 9, 2
    WDEF EN_TREE, 3, 90, 0

; 4-5 T4 start=185 거목×2+새싹×12 [DRAFT — T6 합류 설계지만 엔진 구현 전 T4]
level_4_5:
    db MASK_MOON4_ALL, 185, 5, 5, 3, 2, 0
    db 0, 2, 6, 0
    db 5, 2, 5, 1
    db 5, 6, 6, 0
    db 10, 6, 5, 1
    db 10, 10, 6, 0
    db 1, 4, 3, 6
    db 6, 8, 3, 2
    db 11, 7, 4, 2
    WDEF EN_TREE, 2, 90, 0
    WDEF EN_SEEDLING, 12, 34, 200

; 4-6 T2 start=190 사업가+술꾼 도입 [DRAFT]
level_4_6:
    db MASK_MOON4_ALL, 190, 5, 3, 3, 2, 0
    db 0, 3, 11, 0
    db 10, 3, 7, 1
    db 10, 9, 6, 0
    db 2, 5, 7, 1
    db 6, 5, 4, 3
    db 11, 7, 4, 1
    WDEF EN_BIZ, 3, 70, 0
    WDEF EN_DRUNK, 3, 70, 180

; 4-7 T3 start=190 사업가×4 + 묘목×4 d200 [DRAFT]
level_4_7:
    db MASK_MOON4_ALL, 190, 5, 3, 2, 2, 0
    db 0, 3, 8, 0
    db 7, 3, 6, 1
    db 7, 8, 9, 0
    db 1, 5, 5, 4
    db 9, 5, 6, 2
    WDEF EN_BIZ, 4, 60, 0
    WDEF EN_SAPLING, 4, 60, 200

; 4-8 T5 start=190 술꾼×4 + 거목×2 d180 [DRAFT]
level_4_8:
    db MASK_MOON4_ALL, 190, 5, 5, 2, 2, 0
    db 0, 2, 15, 0
    db 14, 2, 4, 1
    db 14, 5, 14, 2
    db 1, 5, 4, 1
    db 1, 8, 15, 0
    db 3, 3, 9, 2
    db 3, 6, 9, 2
    WDEF EN_DRUNK, 4, 60, 0
    WDEF EN_TREE, 2, 90, 180

; 4-9 T4 start=195 특수4+바오밥 종합 [DRAFT]
level_4_9:
    db MASK_MOON4_ALL, 195, 5, 5, 3, 4, 0
    db 0, 2, 6, 0
    db 5, 2, 5, 1
    db 5, 6, 6, 0
    db 10, 6, 5, 1
    db 10, 10, 6, 0
    db 1, 4, 3, 6
    db 6, 8, 3, 2
    db 11, 7, 4, 2
    WDEF EN_VAIN, 2, 80, 0
    WDEF EN_BIZ, 2, 70, 140
    WDEF EN_DRUNK, 2, 70, 280
    WDEF EN_TREE, 2, 90, 420

; 4-10 T5 HALF start=200 모든 타워/적 [DRAFT]
level_4_10:
    db MASK_MOON4_ALL, 200, 5, 5, 1, 4, 0
    db 0, 2, 15, 0
    db 14, 2, 4, 1
    db 14, 5, 14, 2
    db 1, 5, 4, 1
    db 1, 8, 15, 0
    db 3, 3, 9, 2
    WDEF EN_SAPLING, 4, 56, 0
    WDEF EN_SNAKE, 4, 50, 160
    WDEF EN_VAIN, 2, 80, 300
    WDEF EN_TREE, 3, 90, 420

; 4-11 T5 start=200 최종 보스 [DRAFT]
level_4_11:
    db MASK_MOON4_ALL, 200, 5, 5, 2, 4, 0
    db 0, 2, 15, 0
    db 14, 2, 4, 1
    db 14, 5, 14, 2
    db 1, 5, 4, 1
    db 1, 8, 15, 0
    db 3, 3, 9, 2
    db 3, 6, 9, 2
    WDEF EN_SEEDLING, 10, 36, 0
    WDEF EN_SAPLING, 4, 60, 220
    WDEF EN_TREE, 2, 90, 420
    WDEF EN_TREE_BOSS, 1, 90, 700

; ── 레벨 포인터 테이블 (44개, 0-based) ────────────────
align 4
level_table:
    dd level_1_1,  level_1_2,  level_1_3,  level_1_4
    dd level_1_5,  level_1_6,  level_1_7,  level_1_8
    dd level_1_9,  level_1_10, level_1_11
    dd level_2_1,  level_2_2,  level_2_3,  level_2_4
    dd level_2_5,  level_2_6,  level_2_7,  level_2_8
    dd level_2_9,  level_2_10, level_2_11
    dd level_3_1,  level_3_2,  level_3_3,  level_3_4
    dd level_3_5,  level_3_6,  level_3_7,  level_3_8
    dd level_3_9,  level_3_10, level_3_11
    dd level_4_1,  level_4_2,  level_4_3,  level_4_4
    dd level_4_5,  level_4_6,  level_4_7,  level_4_8
    dd level_4_9,  level_4_10, level_4_11
