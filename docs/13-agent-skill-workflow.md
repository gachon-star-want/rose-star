# Agent Skill Workflow

이 문서는 이 프로젝트에서 사용할 로컬 agent skills를 정리한다. 최종 게임 제출물에는 포함하지 않는다.

## 현재 관찰

2026-05-27 현재 프로젝트 루트에는 `AGENTS.md`, `CONTEXT.md`, `docs/agents/`, `.scratch/`가 있다. Git 저장소와 원격 issue tracker는 아직 확정하지 않았다.

따라서 Matt Pocock 계열 스킬은 로컬 markdown issue tracker 기준으로 바로 사용할 수 있다.

## 우선 설정

추천:

- `setup-matt-pocock-skills`를 먼저 사용한다.
- issue tracker는 초기에는 local markdown으로 둔다.
- triage label은 기본값을 쓴다.
- domain docs는 single-context 구조로 둔다.

권장 구조:

```text
.
├─ AGENTS.md
├─ CONTEXT.md
├─ docs/
│  ├─ agents/
│  │  ├─ issue-tracker.md
│  │  ├─ triage-labels.md
│  │  └─ domain.md
│  └─ adr/
└─ .scratch/
```

GitHub 저장소를 만들고 remote를 연결한 뒤에는 issue tracker를 GitHub Issues로 바꿀 수 있다.

## 권장 스킬

| Skill | 사용 시점 | 이 프로젝트에서의 역할 |
| --- | --- | --- |
| `grill-with-docs` | 설계 결정을 더 밀어붙일 때 | 용어를 `CONTEXT.md`에 고정하고, 되돌리기 어려운 결정은 ADR로 남김 |
| `to-prd` | 현재 문서와 대화를 제품 요구사항으로 묶을 때 | 1.44MB 타워디펜스 PRD 생성 |
| `to-issues` | 구현 작업을 쪼갤 때 | 부팅 화면, 프레임버퍼, 입력, 경로, 웨이브 같은 vertical slice issue 생성 |
| `tdd` | 테스트 가능한 도구/로직을 만들 때 | size-check, level-check, replay-check, deterministic simulation 검증 |
| `diagnose` | 빌드/실행/렌더링/성능 문제가 생겼을 때 | 재현 루프를 먼저 만들고 원인을 좁힘 |
| `prototype` | 밸런스나 상태 모델을 빠르게 검증할 때 | 경제, 웨이브, 타워 수치, 레벨 곡선을 throwaway terminal prototype으로 실험 |
| `improve-codebase-architecture` | 코드가 커져서 모듈 경계가 흔들릴 때 | Win32 platform, renderer, game simulation, data decoder의 seam 점검 |
| `zoom-out` | 코드 구조를 다시 파악해야 할 때 | 관련 모듈과 호출 관계를 한 단계 위에서 정리 |
| `triage` | issue tracker를 운영할 때 | 이슈를 `needs-triage`, `ready-for-agent` 등으로 정리 |
| `handoff` | 긴 작업을 다음 세션으로 넘길 때 | 다음 agent가 바로 이어받을 수 있는 요약 생성 |

## 이 프로젝트에서 특히 중요한 사용법

어셈블리어 게임이라고 해서 모든 검증을 어셈블리에서만 할 필요는 없다. 최종 산출물은 Windows x86 PE `.exe`지만, 개발 중 하네스와 프로토타입은 별도 도구로 만들 수 있다.

권장 분리:

- 최종 게임 코드: FASM, Win32 API, 단일 `.exe`
- 개발 하네스: shell script, small utility, local markdown docs
- 레벨 검증: stage data parser/checker
- 밸런스 실험: throwaway prototype
- 회귀 검증: deterministic replay checksum

단, 개발 도구와 prototype은 최종 제출물에 포함하지 않는다.

## 사용 순서

1. `grill-with-docs`로 도메인 용어와 큰 결정을 정리한다.
2. `to-prd`로 현재 문서를 PRD로 묶는다.
3. `to-issues`로 tracer bullet 구현 이슈를 만든다.
4. 구현 중 새 기능은 `tdd`, 버그는 `diagnose`, 수치 실험은 `prototype`으로 처리한다.
5. 모듈 경계가 복잡해지면 `improve-codebase-architecture`를 사용한다.

## 주의

- 스킬은 최종 게임 크기에 영향을 주지 않는다. 제출물에는 `dist/`만 넣는다.
- 스킬이 만든 문서와 이슈는 개발 품질을 위한 내부 산출물이다.
- 1.44MB exact-fill 목표는 계속 우선한다.
