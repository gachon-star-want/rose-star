# Technical Strategy

## 현재 추천안

**FASM으로 Windows x86 PE `.exe` 단일 실행 파일을 만들고, OS 기본 API만 사용한다.**

엔진, 프레임워크, 대형 런타임, 외부 리소스는 배제한다. 1.44MB 제한에서는 기술 선택 자체가 게임의 일부다.

## 플랫폼 후보

| 후보 | 장점 | 단점 | 현재 판단 |
| --- | --- | --- | --- |
| Windows PE `.exe` | 독립 실행 파일 조건을 가장 안전하게 설명 가능, 심사자 실행 가능성이 높음 | macOS에서 직접 실행 검증이 어려울 수 있음 | 확정 |
| Windows x86 PE | 더 작은 바이너리 가능성, Win32 자료 많음, FASM 예제가 많음 | 주최 측이 64비트만 요구하면 전환 필요 | 확정 |
| Windows x64 PE | 현대 Windows 네이티브 환경에 직접 대응 | 코드/호출 규약이 더 길어질 수 있음, 크기 이점은 적음 | 백업 |
| DOS `.com` | 극도로 작음, 레트로 감성 강함 | DOSBox 필요 시 독립 실행 조건이 애매함 | 비추천 |
| macOS Mach-O | 현재 개발 머신에서 실행 가능 | 심사 환경 불확실, 서명/보안 이슈 가능 | 보류 |
| Linux ELF | 작게 만들 수 있음 | 심사 환경 불확실 | 보류 |

## 권장 아키텍처

```text
main.asm
  ├─ platform layer
  │   ├─ window creation
  │   ├─ input polling
  │   ├─ timing
  │   ├─ audio output
  │   └─ file/exit handling
  ├─ renderer
  │   ├─ framebuffer
  │   ├─ tile draw
  │   ├─ sprite draw
  │   └─ text draw
  ├─ game
  │   ├─ cursor/input state
  │   ├─ tower placement
  │   ├─ path following
  │   ├─ waves/enemies
  │   ├─ tower targeting/fire
  │   ├─ economy
  │   ├─ level state
  │   └─ win/fail/reset
  ├─ data
  │   ├─ palettes
  │   ├─ tile bit patterns
  │   ├─ levels
  │   └─ tiny music patterns
  └─ debug/test hooks
      ├─ deterministic frame step
      ├─ replay input
      └─ state checksum
```

## 렌더링 전략

추천 내부 해상도:

- 256x192 (4:3): 확정(D046). 16x16 타일 기준 16x12 화면(상단 UI 1행 제외 플레이 영역 16x11), 창 모드 전용 + 정수배 4배(1024x768). 기존 256x144/16:9(D008)를 대체.
- 256x144 / 320x180: 이전 후보, D046으로 폐기

추천 방식:

- 메모리 프레임버퍼를 직접 채움
- 타일/스프라이트는 비트패턴 또는 절차적 생성
- OS API로 화면에 확대 출력
- 이미지 파일은 사용하지 않거나 극소수만 허용

Windows PE 기준 추천:

- 창 생성: Win32 API
- 화면 출력: `StretchDIBits`
- 입력: `GetAsyncKeyState` 또는 메시지 기반 키 상태
- 타이머: `QueryPerformanceCounter`

## x86 PE 선택 근거

x86 PE를 먼저 확정하는 이유는 다음과 같다.

- 32비트 Windows GUI 프로그램은 x64 Windows에서 WOW64를 통해 실행된다.
- 어셈블리어 예제와 Win32 호출 관례 자료가 많다.
- 포인터와 호출 규약이 x64보다 단순해 코드 크기와 개발 난이도에 유리하다.
- 이 게임은 2GB 이상 주소 공간이나 64비트 연산이 필요하지 않다.
- 공모전의 핵심은 초소형 독립 실행 파일이므로 네이티브 64비트보다 작은 완성 가능성이 더 중요하다.

x64 PE로 전환하는 조건:

- 주최 측이 64비트 실행 파일만 허용한다고 명시한다.
- 실제 심사/테스트 Windows 환경에서 x86 실행이 막혀 있다.
- x86에서 필요한 API나 오디오/그래픽 경로가 예상 밖으로 불리하다.

## 오디오 전략

샘플 파일을 넣지 않는다. 코드를 통해 파형을 생성한다.

1차 목표는 효과음만 포함한다. 음악은 2차 목표이며, 게임 루프와 배치/웨이브 피드백이 완성된 뒤 추가한다.

추천 사운드:

- 타워 설치: 짧은 클릭/펄스
- 공격/명중: 짧은 사각파
- 적 도달/피해: 하강 노이즈
- 클리어: 4~6음 짧은 멜로디

Windows 기준 후보:

- `waveOutOpen`
- `waveOutPrepareHeader`
- `waveOutWrite`

초기 버전에서는 오디오를 후순위로 둔다. 먼저 배치, 웨이브, 승패 루프가 완성되어야 한다.

1차 효과음 구현 범위:

| 이벤트 | 사운드 |
| --- | --- |
| 타워 설치 | 50~80ms 클릭/펄스 |
| 공격/명중 | 20~60ms 짧은 사각파 |
| 적 도달 | 150~250ms 하강 노이즈/저음 |
| 클리어 | 3~4음 짧은 상승 멜로디 |

2차 음악 구현 범위:

- 짧은 루프 패턴
- 음계/길이/파형만 데이터로 저장
- exact-fill 단계에서 남는 바이트를 음악 패턴, 변주, 숨겨진 트랙으로 채움

## 데이터 전략

데이터는 사람이 읽기 쉬운 형태로 시작하되, 최종 빌드 전 압축한다.

레벨은 오브젝트 명령 기반으로 저장한다. 전체 타일맵을 그대로 저장하지 않고, 경로/설치 가능 칸/웨이브/시작 자원/특수 규칙을 명령 배열로 둔다. RLE와 nibble packing은 보조 압축으로 사용한다.

추천 순서:

1. 개발 중: 사람이 읽기 쉬운 stage DSL 또는 바이트 배열
2. 중간: 오브젝트 명령 배열
3. 최종: command stream + RLE + nibble-packed fields

예시 타일 ID:

| ID | 의미 |
| --- | --- |
| 0 | 빈칸 |
| 1 | 경로 |
| 2 | 설치 가능 |
| 3 | 설치 불가 장식/벽 |
| 4 | 시작 지점 |
| 5 | 도착 지점 |
| 6 | 특수 효과 칸 |
| 7 | 예약 |

예시 stage command:

| 명령 | 의미 |
| --- | --- |
| `PATH x y len dir` | 경로 구간 생성 |
| `BUILD x y w h` | 설치 가능 영역 생성 |
| `BLOCK x y w h` | 설치 불가 영역 생성 |
| `WAVE type count spacing delay` | 웨이브 정의 |
| `START bytes lives` | 시작 자원/생명 정의 |
| `RULE id value` | 스테이지 특수 규칙 |

권장 stage data layout:

```text
stage_header:
  id              db  ; 0..43
  moon            db  ; 1..4 (달)
  sunset          db  ; 1..11 (해넘이)
  tower_mask      db  ; 사용 가능 타워 bitmask
  start_light     dw  ; 시작 별빛
  petals          db  ; 장미 꽃잎/생명
  easy_income     db  ; Easy Mode 초당 별빛
  kill_reward_mul db  ; 처치 보상 배율
  path_count      db
  build_count     db
  wave_count      db
  flags           db

path_commands:
  ; PATH x y len dir

build_zones:
  ; BUILD x y w h

waves:
  ; WAVE type count spacing delay
```

44개 해넘이는 같은 포맷을 공유하고, Easy/Hard는 별도 맵을 복제하지 않는다. 같은 해넘이 데이터를 두 모드가 공유하고 `easy_income` 적용 여부만 달라진다. 따라서 콘텐츠 체감은 88개 챌린지지만 데이터 크기는 44개 해넘이 수준으로 유지된다.

## 툴체인 상태

현재 로컬 관찰:

- `clang`: 있음
- `fasm`: 없음. 프로젝트 표준 어셈블러로 확정
- `nasm`: 없음
- `wine`: 없음

툴체인 후보:

| 후보 | 장점 | 단점 |
| --- | --- | --- |
| FASM | PE 파일 직접 생성이 쉬움, 단일 파일 개발에 좋음 | 확정. 현재 미설치 |
| NASM + linker | 자료 많고 일반적 | 링커 설정 필요, 현재 미설치 |
| clang assembler | 로컬에 있음 | PE 단일 exe 목표라면 추가 설정 필요 |

## FASM 확보 원칙

FASM은 프로젝트 로컬 고정 툴체인으로 관리한다. 시스템에 우연히 설치된 `fasm`을 자동으로 신뢰하지 않는다.

권장 관리 방식:

- `tools/toolchain/fasm/` 아래에 사용 버전을 둔다.
- `tools/toolchain/README.md`에 FASM 버전, 다운로드 출처, SHA256, 확보 날짜를 기록한다.
- `make toolchain-check` 또는 동등한 검증 명령이 실제 사용 중인 FASM 경로와 해시를 출력하게 한다.
- 최종 제출물에는 `tools/`, FASM 바이너리, 문서 파일을 넣지 않는다.

macOS에서 로컬 FASM 실행이 어렵다면 Linux/Windows VM 또는 Docker 빌드 경로를 보조 경로로 둔다. 그래도 프로젝트의 표준 빌드 입력은 같은 `src/main.asm`과 같은 FASM 버전이어야 한다.

## FASM 선택 근거

FASM을 확정하는 이유:

- `format PE GUI 4.0` 같은 방식으로 PE 실행 파일을 직접 만들 수 있다.
- 별도 링커 설정을 최소화할 수 있다.
- Win32 import table을 어셈블리 소스 안에서 관리하기 쉽다.
- 단일 파일에서 시작해 필요한 시점에 include/module로 나누기 좋다.
- x86 Win32 예제가 많아 창 생성, 메시지 루프, GDI 출력까지 빠르게 검증할 수 있다.

FASM 사용 원칙:

- 초기 프로토타입은 `src/main.asm` 단일 파일로 시작한다.
- 코드가 커지면 `src/platform_win32.asm`, `src/render.asm`, `src/game.asm`, `src/data.asm`로 분리한다.
- 최종 릴리즈에서는 include 구조가 최종 exe 크기에 영향을 주지 않게 한다.
- import는 필요한 Win32 API만 최소화한다.

## 기술 원칙

- 실행 파일 하나로 끝내는 것을 기본값으로 둔다.
- 외부 DLL은 OS 기본 제공 DLL만 사용한다.
- 리소스 파일보다 코드와 압축 데이터를 우선한다.
- 디버그 편의보다 최종 크기를 우선하되, 개발용 하네스는 별도 유지한다.
- 크기 최적화는 마지막에 몰아서 하지 않고 매 빌드마다 확인한다.
