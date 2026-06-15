# Tower Design

타워 5종은 D054에서 확정. 기존 디지털 명명(Bit Gun 등)은 우화 명명으로 교체(D060).

## 목표

최종 타워는 5종이다.

| 타워 | 역할 |
| --- | --- |
| 🐑양 | 기본 단일 근접 (바오밥 천적) |
| 💡가로등 | 장거리 저격 (느린 강공격) |
| 🌋화산 | 범위 공격(AoE) |
| 🏜️사막바람 | 슬로우 |
| 🦊여우 | 지원/버프 |

1차 구현은 `🐑양`, `🌋화산`, `💡가로등`까지만 만든다(D054 1차 3종).

## 공통 규칙

- 타워는 16x16 타일 한 칸을 차지한다.
- **타워에는 HP가 없다(설계 결정).** 적은 경로만 따르고 타워를 공격하지 않으므로 타워는 파괴되지 않는다 — 방어선은 "배치"로만 무너진다(병목·사거리 실패). 적이 타워에 피해를 주는 메커닉은 1차/최종 모두 없음. (체력 있는 대상은 적과 🌹장미뿐 — 11/12.)
- 경로 위에는 설치할 수 없다.
- 설치 가능 칸은 해넘이 데이터의 `BUILD` 명령으로 정의한다.
- 타워 판매 시 구매가의 60%를 돌려준다.
- 타워 업그레이드는 1차 목표에서 제외한다.
- 모든 타워는 정수 프레임 쿨다운을 사용한다.

## 타겟팅

기본 타겟팅은 **경로 진행도가 가장 높은 적**이다. 즉, 장미에 가장 가까운 적을 먼저 공격한다.

타겟팅 순서:

1. 사거리 안의 적 목록을 찾는다.
2. `path_progress`가 가장 큰 적을 고른다.
3. 동률이면 배열에서 먼저 찾은 적을 고른다.

이 방식은 구현이 쉽고 타워디펜스 감각에도 맞다.

## 사거리

사거리는 타일 단위 정수로 둔다. 1차 구현에서는 사각형 사거리로 충분하다.

```text
in_range = abs(enemy_tile_x - tower_x) <= range
        && abs(enemy_tile_y - tower_y) <= range
```

원형 사거리는 시각적으로 자연스럽지만 계산이 늘어난다. 최종에서도 사각형 사거리를 유지해도 된다.

## 확정 스탯 테이블 (AI 구현 스펙)

수치는 **확정값**이다(설계 결정 — D079). cooldown은 60FPS 프레임 정수라 변환 불필요. 구현자는 그대로 `tower_def` 배열에 넣는다. (비용 단위 = 별빛)

| 타워 | 상수 prefix | Cost | Damage | Cooldown(f) | Range(tile) | flags | 1차 해금 |
| --- | --- | ---: | ---: | ---: | ---: | --- | --- |
| 🐑양 | `TW_SHEEP` | 20 | 1 | 20 | 3 | `DAMAGE` | 1-1 |
| 🌋화산 | `TW_VOLCANO` | 40 | 2 | 55 | 2 | `DAMAGE` `SPLASH` | 2-1 |
| 💡가로등 | `TW_LAMP` | 45 | 5 | 70 | 4 | `DAMAGE` | 2-6 |
| 🏜️사막바람 | `TW_WIND` | 35 | 0 | 45 | 3 | `SLOW` | 3-6 |
| 🦊여우 | `TW_FOX` | 50 | — | — | 1 | `BOOST` | 4-1 |

- **사거리 메트릭 = 체비셰프(정사각형)**: `in_range = max(|ex-tx|, |ey-ty|) <= range`. 양과 동일(1-1 검증분). 원형 안 씀(D 본문).
- **판매 환급 = cost × 60%** (내림): 양 12 / 화산 24 / 가로등 27 / 사막바람 21 / 여우 30. (`SELL_REFUND` 일반화 — 현재 양 12 ✓)
- **🦊여우**: damage/cooldown 없음(공격 안 함). BOOST 오라만(§플래그 알고리즘).
- **🏜️사막바람**: damage 0, 처치 못 함. SLOW 상태만 부여.
- splash 반경·slow 배율·boost 배율 등 효과 상수는 §플래그 알고리즘에 못박는다.
- 업그레이드 없음(D 본문). 타워는 설치/판매만.

## 타워별 디자인

### 🐑양

기본 타워다. 싸고 빠르며 바오밥의 천적이다. 모든 초반 해넘이의 기준이 된다.

용도:

- 🌱싹·🌿새싹 처리
- 🐍뱀 등 빠른 적 견제
- 경로 초반/후반 어디에 둬도 무난

시각:

- 작고 둥근 양, 폴짝 들이받기
- 1~2픽셀 풀잎/먼지 효과
- 흰/크림 팔레트

*(D054: 양에 "장미도 먹을 수 있는" 리스크 메커닉 여지)*

### 💡가로등

느리지만 강하다. 멀리서 한 발씩 정확히 쏜다. 🪴묘목·🌳거목 같은 탱커형 적을 상대한다.

용도:

- 체력 높은 적 처리
- 좁은 경로의 핵심 지점 장거리 방어

시각:

- 긴 가로등 기둥, 빛줄기 저격
- 발사 시 2프레임 점등
- 노랑 불빛 팔레트

### 🌋화산

범위 공격 타워다. 많은 적을 상대한다.

용도:

- 무리 웨이브(다수 🌱싹)
- 밀집한 적 처리

시각:

- 작은 화산, 분화 splash 파동
- 붉은색/주황색 팔레트

### 🏜️사막바람

속도를 늦춘다. 직접 피해는 없거나 매우 낮다.

용도:

- 🐍뱀·🍷술꾼 계열 견제
- 긴 사거리 타워(가로등) 앞에서 적을 묶음

시각:

- 모래 회오리, 베이지/흰색 점멸
- 적에게 슬로우 상태 팔레트 적용

### 🦊여우

주변 타워를 강화한다. 직접 공격하지 않는다.

용도:

- 후반 배치 퍼즐
- 좁은 설치 구역에서 효율 극대화

시각:

- 주변 1타일에 얇은 신호선("길들임"의 끈)
- 주황/흰색 팔레트

## 해금 순서

| 구간 | 해금 |
| --- | --- |
| 1-1 | 🐑양 |
| 2-1 | 🌋화산 |
| 2-6 | 💡가로등 |
| 3-6 | 🏜️사막바람 |
| 4-1 | 🦊여우 |

## 구현 메모

타워는 테이블 기반으로 둔다.

```text
tower_def:
  cost
  damage
  cooldown
  range
  flags
```

## 플래그 알고리즘 (확정 — 효과 상수 못박음)

```text
TOWER_DAMAGE = 0x01   ; 타겟에 damage. 거목 ARMORED·허영쟁이 AURA가 받는 피해 -1 적용(11 §플래그).
TOWER_SPLASH = 0x02   ; 화산. 아래.
TOWER_SLOW   = 0x04   ; 사막바람. 아래.
TOWER_BOOST  = 0x08   ; 여우. 아래.
```

발사 적용 시점은 docs/18 §전체 틱 순서의 `fire_tick`. 타게팅은 위 §타게팅(path_progress 최대) 그대로.

### TOWER_SPLASH (화산) — 주변 피해

주 타겟을 정한 뒤, 주 타겟 **타일 기준 체비셰프 1타일** 안의 모든 적에게 같은 damage를 준다.

```text
fire(tower=화산):
    main = target(tower)                      ; path_progress 최대(사거리 내)
    if main == none: return
    for e in active_enemies:
        if chebyshev_tile(e, main) <= 1:      ; 주 타겟 포함(자신과의 거리 0)
            on_hit(e, tower.damage)           ; ARMORED/AURA 경감은 on_hit 내부
    tower.cooldown = TW_VOLCANO.cooldown
```

splash 반경 상수 = **1타일**. 감쇠 없음(범위 내 동일 damage) — 구현·하네스 단순.

### TOWER_SLOW (사막바람) — 속도 저하 상태

damage를 주지 않고, 사거리 내 적에게 **슬로우 상태**를 건다. 적별 `slow_timer`(틱 카운터)에 기록한다.

```text
SLOW_FACTOR = 128        ; ÷256 = 50% 속도 (eff_speed = speed * 128 / 256)
SLOW_DURATION = 60       ; 틱(=1초). 재적용 시 갱신(연장 아닌 리프레시).

fire(tower=사막바람):
    for e in active_enemies:
        if in_range(e, tower):
            e.slow_timer = SLOW_DURATION       ; 사거리 내 전부(쿨다운마다 리프레시)
    tower.cooldown = TW_WIND.cooldown

apply_slow(enemy, speed, resist=false):        ; move_tick에서 호출(11 §EVADE/ERRATIC)
    if enemy.slow_timer > 0:
        factor = SLOW_FACTOR
        if resist: factor = (SLOW_FACTOR + 256) / 2   ; 술꾼: 50%와 100%의 중간=75%
        speed = speed * factor / 256
    return speed
; slow_timer 감소는 틱 끝 decay 단계(docs/18)에서 일괄 dec.
```

스택 안 함(여러 사막바람이어도 50% 1단). 술꾼은 `resist`로 75%만.

### TOWER_BOOST (여우) — 주변 타워 강화

공격하지 않고, **체비셰프 1타일** 안 타워의 쿨다운을 빠르게 돌린다. AURA처럼 매 틱 재계산(영구 변형 금지·시점 고정).

```text
BOOST_FACTOR = 218       ; ÷256 ≈ 85% (쿨다운을 0.85배 길이로 == 발사 더 잦음)

boost_recompute():                 ; 틱 순서상 fire 전 (docs/18)
    for t in towers: t.boosted = 0                 ; 매 틱 리셋
    for f in towers where f.flags & TOWER_BOOST:
        for t in towers where t != f and t.flags & TOWER_DAMAGE:
            if chebyshev_tile(t, f) <= 1:
                t.boosted = 1                       ; 불리언, 스택 안 함
; 효과: fire에서 쿨다운 설정 시 boosted면 단축
set_cooldown(tower):
    cd = tower.def.cooldown
    if tower.boosted: cd = cd * BOOST_FACTOR / 256
    tower.cooldown = cd
```

여우끼리·여우 자신은 BOOST 못 받음(DAMAGE 타워만 대상). 스택 없음 → "어디에 여우를 두나" 배치 퍼즐만 남고 폭주 없음.

## 구현 메모 (추가)

- 모든 effect 상수(`SLOW_FACTOR=128`, `SLOW_DURATION=60`, `BOOST_FACTOR=218`, splash 반경 1)는 `game_defs.inc`에 named 상수로 둔다(매직넘버 금지).
- AURA(적)·BOOST(타워)·SLOW decay는 모두 docs/18 §전체 틱 순서가 정한 단일 지점에서만 변형 → PE와 헤드리스 하네스가 비트 단위로 일치.
