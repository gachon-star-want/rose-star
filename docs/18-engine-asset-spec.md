# Engine & Asset Build Spec

10/11/12가 "무엇을(적·타워·맵·웨이브)"이라면, 이 문서는 **교차로 걸치는 엔진/에셋 스펙**이다 — 어디 한 곳에 속하지 않지만 여러 곳이 의존하는 것들. 전부 **설계 결정(확정값, D079)**이라 시뮬 없이 지금 박는다. (밸런스 결과인 웨이브 숫자만 12에서 draft/locked로 관리.)

목차: ①전체 틱 순서 ②결정론적 PRNG ③비트맵 폰트 ④스프라이트·팔레트·blit ⑤SFX ⑥기록 코드 ⑦공유 카드 BMP ⑧정확히 1,474,560B 채우기.

---

## 1. 전체 틱 순서 (단일 진실의 원천)

`update()`는 프레임당 1회(60Hz 목표). **모든 상태 변형은 아래 순서의 정해진 지점에서만** 일어난다 → PE와 헤드리스 하네스가 비트 단위로 일치, 리플레이 재현 보장. 10/11이 "fire_tick에서"·"move 전에"라고 한 건 전부 이 순서를 가리킨다.

### 게임 상태(game_state) 열거

| 상수 | 값 | 의미 | 시뮬 |
| --- | ---: | --- | --- |
| `GS_TITLE` | 0 | 타이틀(이어하기/새 별 선택, 06 §기록 저장) | 동결 |
| `GS_PLAY` | 1 | 진행 중 | 가동 |
| `GS_PAUSE` | 2 | 일시정지(`[P]`) | 동결 |
| `GS_WON` | 3 | 승리 | 동결 |
| `GS_LOST` | 4 | 패배 | 동결 |

`GS_PLAY`만 시뮬을 돌린다. 나머지는 update가 즉시 반환(엣지 소거)하므로 일시정지·승/패 화면은 결정론에 영향 없음(리플레이는 GS_PLAY 틱만 카운트).

### 프레임 외곽 입력(시뮬 무관 토글)

아래는 `input_apply`(시뮬 내부)가 아니라 update **진입 전** 프레임 핸들러에서 처리한다 — game_state 전이·렌더 옵션이라 리플레이 틱 스트림을 건드리지 않는다.

| 키 | 동작 |
| --- | --- |
| `[P]` | GS_PLAY ↔ GS_PAUSE 토글 |
| `[F]` | 게임 속도 토글 1×↔2×(`speed_mul` 1/2 — 한 프레임에 update를 1회 또는 2회 호출. 시뮬은 그대로, 체감만 빨라짐) |
| `[M]` | 음소거 토글(SFX) |

`speed_mul`은 update 호출 *횟수*만 바꾼다(틱 내부 수치 불변) → 2× 클리어도 리플레이·하네스 결과 동일.

```text
frame():                               ; 60Hz 렌더 루프, update의 바깥
    handle_toggles()                   ; [P]/[F]/[M] — game_state·speed_mul·muted
    if game_state == GS_PLAY:
        repeat speed_mul times: update()
    render()

update():                              ; 1 논리 틱(여기부터 시뮬·리플레이 단위)
    if game_state != GS_PLAY:
        in_place=0; in_sell=0; in_callwave=0   ; 입력 엣지 소거
        return                                 ; (도달 안 함 — frame이 GS_PLAY만 호출)

    1. input_apply       ; in_place→do_place, in_sell→do_sell,
                         ;   in_callwave→call_wave_early() (엣지 셋 다 0으로 리셋)
    2. spawn_tick        ; wave_timer 감소; 0 도달 시 다음 WAVE 큐잉 → free 슬롯에 새 적
    3. aura_recompute    ; 적 AURA: 전 적 aura_buff=0 후 재계산 (11 §AURA)
    4. boost_recompute   ; 타워 BOOST: 전 타워 boosted=0 후 재계산 (10 §BOOST)
    5. move_tick         ; 슬롯 0..MAX-1 순회: eff_speed = base
                         ;   ENEMY_EVADE: 사거리 안이면 ×3/2 (11 §EVADE)
                         ;   ENEMY_ERRATIC: prng_next 1회로 jitter (11 §ERRATIC)
                         ;   apply_slow(slow_timer, resist=ERRATIC) (10 §SLOW)
                         ;   pos += eff_speed; pos>=PATH_END_FX → leak(꽃잎-leak, 슬롯 비움)
    6. fire_tick         ; 타워 슬롯 순회: cooldown>0면 dec & skip; 아니면 fire()
                         ;   DAMAGE/SPLASH/SLOW 효과(10 §플래그), set_cooldown(boosted)
    7. boss_tick         ; is_boss HP 임계 → 호위 spawn_burst (11 §보스)
    8. decay_tick        ; 전 적 slow_timer>0면 dec (cooldown dec는 6에서 처리)
    9. winloss_tick      ; petals<=0 → GS_LOST; 모든 WAVE 소진 && active_count==0 → GS_WON
```

핵심 불변식:
- **일시정지/속도/음소거는 update 바깥**(frame)에서 처리 → 리플레이 틱 스트림 불변. 2×는 update를 2번 부를 뿐이라 결과 동일.
- **aura/boost를 move·fire 앞**에 둬서 이 틱의 버프가 이 틱 전투에 반영된다.
- **버프는 매 틱 리셋 후 재계산**(영구 누적·잔류 금지) → 소스(허영쟁이·여우)가 죽으면 다음 틱 즉시 소멸.
- **prng_next 호출 순서 = 슬롯 인덱스 순서**(move_tick 내 ERRATIC 적만, 슬롯 오름차순). 호출 횟수·순서가 고정이라 스트림 결정론 유지.
- 현 1-1 구현(spawn→move→fire→winloss)은 이 순서의 부분집합 — aura/boost/boss/decay는 해당 메커닉 도입 시 끼워 넣는다(순서만 지키면 됨).

### spawn_tick — 웨이브 큐 + 다음 무리 앞당기기(`call_wave_early`)

한 해넘이는 WAVE 줄들의 큐다(12 §44 표). 각 WAVE는 `delay`(앞 WAVE 시작 기준 틱) 뒤에 풀린다. `wave_timer`가 그 카운트다운.

```text
WAVE_BONUS_DIV = 30           ; 남은 대기 틱 / 30 = 보너스 별빛(0.5초당 1)

spawn_tick():
    if active_wave has pending spawns:        ; 현재 웨이브 분사 중
        spawn_cd--; if 0: emit one enemy; reset spawn_cd = spacing
        return
    if next_wave exists:
        wave_timer--
        if wave_timer <= 0: begin_wave(next_wave)   ; 다음 WAVE 활성화

call_wave_early():                            ; [N] 엣지, input_apply에서
    if next_wave exists and wave_timer > 0:
        starlight += wave_timer / WAVE_BONUS_DIV    ; 남은 대기에 비례한 보너스
        wave_timer = 0                              ; 즉시 풀림(다음 spawn_tick이 begin_wave)
    ; 풀 웨이브가 없으면(마지막 웨이브 분사 중/종료) 무시 — 안전
```

- 보너스는 **앞당긴 대기 시간에만 비례** → 일찍 부를수록 더 받는 위험·보상. Hard에서 초당 별빛이 없는 만큼 능동적 자원 획득 수단(12 §경제와 연결). 결정론: 입력 틱이 리플레이에 기록되므로 동일 재현.
- 동시 화면 적 ≤ MAX_ENEMIES(16)는 그대로 — [N] 남발로 슬롯 초과 시 begin_wave가 free 슬롯에만 투입(나머지는 spawn_cd로 대기, 보스 호위와 동일 규칙).

---

## 2. 결정론적 PRNG

진짜 난수 금지(리플레이·하네스가 깨짐). **xorshift32** 1개 스트림.

```text
prng_state dd ?                 ; 32-bit, 0 금지

prng_seed(stage_id, mode):      ; init_level에서 호출
    prng_state = (stage_id*2654435761 + mode*40503 + 0x9E3779B9) | 1   ; 0 회피

prng_next() -> u32:
    x = prng_state
    x ^= x << 13
    x ^= x >> 17
    x ^= x << 5
    prng_state = x
    return x
```

- **시드 = f(stage_id, mode)** 뿐 → 같은 해넘이는 매 시도 동일 전개(퍼즐로서 학습 가능, 리플레이는 stage+mode+입력만으로 재현). 기록 코드에 시드를 따로 담을 필요 없음.
- 현재 유일 소비자 = ERRATIC(술꾼) move jitter. Random 모드(절차적 맵)도 **이 PRNG를 별도 인스턴스**로 쓴다(맵 생성 시드는 그 모드 진입 시각 기반 1회). 캠페인 스트림과 섞지 않는다.
- 하네스도 동일 `prng_seed`/`prng_next`를 호출 → 시뮬 결과 동일.

---

## 3. 비트맵 폰트 서브셋

게임에 나오는 글자만 글리프로 갖는다(서브셋). **글리프 = docs/09 인게임 문자열의 고유 문자 집합.**

### 서브셋 인벤토리 (추출 결과)

| 구분 | 수 | 비고 |
| --- | ---: | --- |
| 한글 — UI 문자열 표(확정) | **80** | `LOCKED` 서브셋 |
| 한글 — 서사 포함(09 서사는 초안) | **153** | 서사 확정 전까진 `PROVISIONAL` |
| ASCII | ~50 | 숫자 0-9, 대문자 A-Z, 소문자 c·s, 기호 `[ ] : - / 공백` |

UI 확정 80음절:
```
가겼기꽃나남넘네누늘다달도돌두드라려로롭르른를리린막매면모못무물미바밥별복빛
사손쉬시아았어었오왔왕움워위으은을의이잎자작장저졌지직진차천카켰코키타태판하한해했히
```
서사 추가 73음절(초안, 09 §서사 확정 시 lock):
```
갠거걸것겐겠곁고그금길까끝난날내느는니돋되된됨든들땐떠래러럼림마만멀며목밤뱀보본볼뽑뿌
새서셈송수싹쓰씨에엔여올용우웃인일임있제조준쪼책처취피함헛황
```

> 의존성: 폰트 서브셋은 **09 §UI 표가 확정**돼야 80자가 잠긴다(이미 확정). 서사 73자는 09 §서사가 초안이라 **provisional** — 서사 카피 확정 후 재추출해 합집합 153자로 lock. 추출은 빌드 스크립트(`tools/font-subset.py`: 09에서 고유 음절 파싱)로 자동화해 표류 방지.
>
> **구현 완료(2026-06-15, D083).** `font-subset.py`가 더미 글리프(초중종성 추상 패턴)에서 **AppleSDGothicNeo TTF 12×12 실제 래스터화**로 교체됨(이전엔 전 텍스트가 깨진 블록). UI 표 + 서사(`gen_story.py` STORY_SCENES, 단일 소스) 음절 = 200글리프 → `src/font_data.inc`(자동 생성). draw_char 규약(행 12비트, byte0=v>>4, byte1=(v<<4)&0xF0) 일치.
>
> ⚠️ **(과거 노트) 폰트 재추출 필요 (D080)**: docs/09에 신규 UI 문자열이 추가됨 — `[N] 다음 무리`, `멈췄다`, `새로 시작`, `이어하기`, `기록 코드를 넣으면 이어갈 수 있다`, `코드가 맞지 않는다`, `N달 N해넘이까지 클리어한 기록`, `별빛이 N 왔다`. 신규 음절 후보: **다·음·무·리·멈·췄·새·이·어·하·기·넣·맞·않** 등. 서사 확정 전까지는 PROVISIONAL 풀에서 재추출. 09 UI 표 최종 확정 시 `tools/font-subset.py` 재실행 후 80+α LOCKED 값으로 갱신.

### 글리프 포맷

- 셀 **12×12 픽셀, 1bpp**(전경/투명). 행당 2바이트(12비트 + 패딩) × 12행 = **24바이트/글리프**.
- 한글·ASCII 동일 12×12 셀(고정폭, 렌더 단순). ASCII는 좌측 정렬, 빈 우측은 0.
- 렌더: 1비트=전경색(팔레트 인덱스), 0=건너뜀(배경 보존). HUD/카드/서사에서 동일 루틴.
- 글리프 테이블 = `[코드포인트 u16][비트맵 24B]` 정렬 배열, 코드포인트 이진탐색.
- 바이트 비용: 153+50 ≈ 200글리프 × 24B ≈ **4.8KB** (1,474,560 대비 무시 가능, exact-fill엔 도움).

---

## 4. 스프라이트 · 팔레트 · blit

폰트 포맷을 픽셀 한 점 안 찍고 확정했듯, **스프라이트 저장 포맷·팔레트 표현·blit 루틴도 설계 결정**이라 지금 박는다(베이크된 hex 바이트만 cast24 PNG가 있어야 하므로 후속). 이로써 D078이 "asm 렌더러 팔레트 포맷 결정 시점으로" 미뤘던 양자화의 *대상 포맷*이 확정된다 → 양자화는 이제 로컬 변환(이미지 생성 API 0원, 비용 무관 — [[api-cost-image-gen-approval]]).

### 저장 포맷 (확정)

- **4bpp 인덱스 스프라이트, 인덱스 0 = 투명.** 픽셀당 4비트(반바이트). 24×24 = 행당 12B × 24 = **288B/스프라이트**(D076 산정치와 일치). 16×16 타일 = 8B×16 = 128B.
- **팔레트 = palette_id별 16색 서브팔레트.** `palette[palette_id][0..15]` 각 엔트리 `0x00RRGGBB`(프레임버퍼와 동일 색공간). 인덱스 0 엔트리는 미사용(투명).
- 스프라이트는 *색이 아니라 인덱스*를 담는다 → palette_id만 바꿔 같은 비트맵을 다르게 칠함.

### palette_id의 역할 (10/11 표와 연결)

- **바오밥 4단계 = body_id 0(단일 바디 스프라이트) + palette_id 0/1/2/3**(연두/초록/갈색/짙은갈색). 새 스프라이트 아님 → 리소스 원칙(바디 3개) 충족.
- **Easy/Hard = 팔레트 차원 하나 더**(D070: 새 이미지 아닌 팔레트 변형). `palette[mode][palette_id][idx]` 또는 Hard를 Easy에서 변환(명도↓·위험색 강조). UI 텍스트·기록코드 팔레트는 모드 무관 고정(09 가독성).
- 모든 서브팔레트의 hex는 **명시적 리스트로 박는다**(스크립트가 몰래 굽지 않음 — D078 정신). 첫 결정 시 docs/18에 hex 표 추가 + D번호.

### blit 루틴 (확정)

```text
blit_sprite(sprite, palette_id, tile_col, tile_row, mode):
    pal = palette[mode][palette_id]
    ; 24px 스프라이트를 16px 논리셀에 아래-중앙 앵커, 위로 오버플로우(TD 표준, D076)
    base_x = tile_col*16 + 8 - 12          ; 가로 중앙: 셀중앙 - 스프라이트폭/2
    base_y = tile_row*16 + 16 - 24         ; 세로 바닥: 셀바닥 - 스프라이트높이
    for sy in 0..23:
        for sx in 0..23:
            idx = sprite_pixel(sprite, sx, sy)     ; 4bpp 언팩
            if idx == 0: continue                  ; 투명
            px = base_x+sx; py = base_y+sy
            if px<0 or px>=FB_W or py<0 or py>=FB_H: continue   ; 클립(HUD행 침범 방지)
            framebuffer[py*FB_W + px] = pal[idx]
```

- 현 `fill_rect` 기반 도형 렌더(1-1)는 이 blit으로 교체된다. game.inc은 여전히 OS-free(인덱스→색 계산만, int 0x80 없음) → 헤드리스 하네스에서도 그대로 렌더·검증.
- 렌더 순서(겹침): 타일 → 빌드존 → 타워 → 적(path_progress 순) → HUD → 토스트. 적은 위로 오버플로우하므로 뒤쪽(작은 y) 먼저.

### cast24 PNG → asm 파이프라인

```
tools/png-to-sprite.py:
    입력: example/ai/cast/cast24/*.png (D078 산출, 24px)
    1. 각 스프라이트를 16색으로 양자화 → 그 스프라이트의 palette_id 서브팔레트 hex 도출
    2. 투명 픽셀(알파 0 또는 지정 키컬러) → 인덱스 0
    3. 4bpp 패킹 → src/assets/*.inc (db 바이트열) + 팔레트 hex 표
    4. 밤하늘 가독성(D072) 자동 점검: 적 외곽 vs 배경 명도차 임계 이상인지 assert
```

양자화는 기존 PNG에 대한 로컬 연산(생성 0회). 결과 팔레트 hex는 사람이 확인 후 docs/18에 박고 D번호로 lock.

> **구현 완료(2026-06-15, D082).** `tools/png-to-sprite.py` 구현 → `src/sprites.inc`(18 스프라이트, 자동 생성). 팔레트는 스프라이트별 16색을 결정론적으로 도출해 `sprites.inc`에 박힘 = **lock 산출물**(288 hex를 docs에 중복 기재하지 않고 sprites.inc를 단일 진실원으로; ship-once D074). `blit_sprite`/`plot_px` game.inc 구현, draw_towers/enemies/장미 24px blit 교체 완료. 가독성(D072): 전 적 maxL ≫ 배경(34) 확인.

## 5. SFX (절차 생성, 외부 파일 0 — docs/04)

오디오 파일을 넣지 않는다. **이벤트 시점에 사각파 PCM을 즉석 생성**해 재생.

- API: `winmm.dll` `waveOutOpen`/`waveOutPrepareHeader`/`waveOutWrite`(비차단). import 추가 필요(winmm).
- 포맷: **PCM 11025Hz, 8-bit, 모노**. 작은 출력 버퍼 1~2개 순환.
- 파형: 사각파(주파수=토글 주기), 선형 감쇠 엔벨로프. 생성 코드만 들어가고 샘플 데이터는 런타임 합성.

### sfx_def 테이블 (id → freq, dur, 감쇠)

| 이벤트 | freq(Hz) | dur(ms) | 비고 |
| --- | ---: | ---: | --- |
| 커서 이동 | 880 | 30 | 짧은 틱 |
| 타워 설치 | 523 | 90 | 맑게 |
| 타워 판매 | 392 | 90 | 낮게 |
| 별빛 부족(거부) | 196 | 120 | 부저 |
| 적 처치 | 1047 | 60 | 밝게 |
| 꽃잎 감소(leak) | 147 | 180 | 묵직/경고 |
| 승리 | 659→988 | 400 | 2음 상행 |
| 패배 | 330→165 | 500 | 2음 하행 |

값은 사람이 듣고 ±조정 가능한 *시작점*(밸런스 결과 아님, 비차단이라 게임플레이 무관). 음소거 토글 `M` 권장.

---

## 6. 기록 코드 (Record Code) — 결과 기록 + 진행 저장 이중 역할

자랑/공유용 **결과 코드**이자, **외부 세이브 파일 없이 진행을 이어가는 수단**이다(전체 입력 리플레이 아님 — 09 톤). 포맷:

```
RS-{달}-{해넘이}-{모드}-{HEX4}
예) RS-4-11-H-A91F
```

- `RS` 고정 접두(Rose Star). 달 1~4, 해넘이 1~11, 모드 `E`/`H`(ASCII, 09 D062).
- `HEX4` = 16비트 페이로드를 4 hex 대문자로.

### 16비트 페이로드 비트 배치

| 비트 | 필드 | 범위 |
| --- | --- | --- |
| 0–2 | 남은 꽃잎 | 0–5 (3b) |
| 3–7 | 타워 설치 수 | 0–31 (5b) |
| 8–12 | 판매 횟수 | 0–31 (5b) |
| 13–15 | 체크(아래) | 3b |

체크 = `(꽃잎 + 타워수 + 판매수 + 달 + 해넘이) & 7`. 디코드 시 불일치면 `잘못된 코드`로 거부(가벼운 위변조 방지). 달·해넘이·모드는 접두에서 읽으므로 페이로드에 중복 안 함.

- 인코딩/디코딩 루틴은 양방향(코드 표시 + 붙여넣기 검증). 화면 표기는 한글 텍스트와 함께, 코드 자체는 ASCII(09).

### 이어하기 — 기록 코드가 세이브를 대체 (docs/04 외부 파일 0 정합)

`RS-{달}-{해넘이}-{모드}-{HEX4}`는 "그 해넘이를 클리어한 기록"이므로, 접두(`달/해넘이/모드`)만으로 **어느 스테이지까지 클리어했는지**를 인코딩한다. 이를 타이틀 `이어하기`에서 입력받으면 **별도 세이브 파일 없이 진행을 복원**할 수 있다 — 외부 레지스트리/파일을 쓰지 않아 docs/04 "외부 파일 0" 정신과 완전히 정합.

```text
progress_from_code(code):
    parse RS-{m}-{s}-{mode}-{HEX4}       ; 달 m, 해넘이 s, 모드 mode
    if check_fails: reject("코드가 맞지 않는다")
    ; 해금 정책: 스테이지는 순차 해금이라 (m-1)*11 + s까지가 모두 클리어된 것과 동등
    unlock_up_to(m, s, mode)              ; m-s 이전 스테이지 + 보너스 모드(2-11→Endless, 3-11→Random) 해금
    set_resume(m, s, mode)               ; 타이틀의 달/해넘이 선택 커서를 (m, s)로 초기화
```

- **새로 시작**: 해금 0, 1-1 Easy만 선택 가능.
- **이어하기**: 코드 입력 → `unlock_up_to` → 달/해넘이 선택 메뉴 진입(모든 해금된 스테이지가 선택 가능).
- 코드는 복수 시작점을 지원한다(Easy 최신 코드와 Hard 최신 코드를 각각 입력해 양쪽 진행 복원 가능).
- **세이브 타이밍**: 스테이지 클리어 결과 화면에서 기록 코드를 표시 → 플레이어가 적으면 됨. 게임이 암묵적으로 파일을 쓰지 않는다.
- 메모리상 해금 테이블(`unlock_table`: 44비트 = 6B Easy + 6B Hard, 비트셋)은 런타임에 유지하고 타이틀→선택 화면에서 참조한다. 프로세스 종료 후 소멸 — 이것이 "파일 0" 설계의 의도적 트레이드오프(매 세션 코드 입력 = 게임의 아날로그 감성과 맞음).

#### 세이브/이어하기 흐름 (GS_TITLE, docs/18 §1 게임 상태)

```text
title_screen():
    선택지 1: "새로 시작"  → unlock_table 초기화 → 달/해넘이 선택(1-1 Easy만)
    선택지 2: "이어하기"   → 기록 코드 입력 UI 진입
        입력 완료 → progress_from_code(code):
            성공: "N달 N해넘이까지 클리어한 기록" 표시 → 확인 → 달/해넘이 선택(해금분)
            실패: "코드가 맞지 않는다" → 입력 화면으로
```

문자열은 09 §이어하기에 확정(한글 텍스트). 코드 입력창은 ASCII 직접 입력(`[A-Z0-9-]` 만 허용, 나머지 무시).

---

## 7. 공유 카드 BMP

결과를 담은 픽셀 포스터를 파일로 저장(09 §공유 카드). 외부 라이브러리 없이 **무압축 BMP 직접 기록**.

- 파일: `share/RS-{달}-{해넘이}-{모드}-{HEX4}.bmp` (코드와 동일 식별자). `CreateDirectory`(share) + `CreateFile`/`WriteFile`(kernel32).
- 크기: **480×480, 24bpp 무압축**(BI_RGB). 행 패딩 4바이트 정렬(480×3=1440, 이미 4의 배수라 패딩 0). 바텀업.
- 헤더: `BITMAPFILEHEADER`(14B, `BM`, 파일크기, offBits=54) + `BITMAPINFOHEADER`(40B, w=480,h=480,planes=1,bpp=24,comp=0).
- 픽셀 데이터 = **메모리에서 합성**: 중앙에 게임 프레임버퍼(256×192) nearest로 확대 배치, 둘레는 배경 팔레트, 상/하단에 §3 폰트로 텍스트 기록(09 §카드 안의 글자: 제목/해넘이 N-N/모드/꽃잎 N/타워 N/판매 N/코드 RS-...).
- 데이터는 BGR 순(BMP는 하위 바이트가 B). 프레임버퍼(0x00RRGGBB)에서 채널 재배열해 기록.

---

## 8. 정확히 1,474,560 바이트 채우기 (D004)

최종 .exe는 **정확히 1,474,560바이트**(3.5" HD 플로피 1장). 줄이는 게 아니라 *정확히 맞춘다*.

### 전략

1. 링크 후 실제 크기 `N`을 측정한다(`tools/verify-pe.py` 확장).
2. `N < 1,474,560`이면 부족분 `P = 1,474,560 - N`만큼 **전용 패드 섹션**을 PE 끝에 추가한다.
3. `N > 1,474,560`이면 빌드 실패 처리 — 코드/데이터를 줄여야 함(폰트·레벨 데이터는 이미 작아 여유 큼).
4. 빌드 후 재검증: 산출물이 정확히 1,474,560인지 byte 단위 assert(`make size-check`).

### 패드 내용 (의미 있는 채움)

0으로 채우지 않는다 — "1바이트도 안 남기는" 미학에 맞게 **의미를 채운다**:

- `docs/16-lore-bible.md`(심층 lore)와 크레딧을 UTF-8 평문으로 패드 섹션에 임베드(읽을 수 있는 이스터에그 — 헥스 에디터로 열면 이야기가 나옴).
- lore가 P보다 작으면 나머지는 결정론적 패턴(예: `RS` 반복 또는 0x00)으로 정확히 채운다.
- 패드는 코드/데이터에서 참조되지 않으므로 실행에 영향 없음.

### 빌드 단계

```
make build      → 링크 → 크기 N 측정
                → tools/pad-to-size.py: P 계산, lore+크레딧+패턴으로 패드 생성, 섹션 append
                → 재측정 == 1,474,560 assert (아니면 fail)
make size-check → dist/rose-star.exe 가 정확히 1,474,560B 인지 단독 검증
```

> 주의: 패드는 **최종 릴리즈 단계**에서만(원샷 출시 D074). 개발 중 빌드는 패드 없이 작은 exe로 빠르게 돈다. exact-fill은 릴리즈 게이트.
