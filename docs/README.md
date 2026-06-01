# 1.44MB Assembly Game 개발 문서

이 폴더는 1.44MB GAME_DEV CONTEST 제출작을 만들기 위한 개발 문서의 기준점이다. 문서는 토론하면서 계속 갱신한다. 결정된 내용, 보류된 내용, 검증 방법을 분리해 관리한다.

## 현재 방향

- 공모전: 1.44MB 이하 독립 실행 게임 개발 공모전
- 핵심 제약: 압축 해제 후 전체 제출물 1,474,560바이트 이하
- 현재 게임 방향: 「어린왕자: 장미의 별」 — 초기 Mario식 압축/재사용 철학을 참조한 44해넘이 초소형 타워디펜스(순수 어린왕자 우화, D048/D049)
- 현재 기술 방향: FASM 기반 어셈블리어, 무엔진, Windows x86 PE `.exe` 단일 실행 파일
- 현재 문서 상태: 초기 설계 초안. 플랫폼/아키텍처/어셈블러는 확정, FASM은 프로젝트 로컬 고정 툴체인으로 확보 예정

## 문서 목록

| 문서 | 목적 |
| --- | --- |
| [00-product-brief.md](00-product-brief.md) | 게임의 목적, 감성, 핵심 재미, 비목표 |
| [01-contest-constraints.md](01-contest-constraints.md) | 공모전 규칙과 제출 제약 |
| [02-technical-strategy.md](02-technical-strategy.md) | 어셈블리 개발 전략, 플랫폼 후보, 런타임 전략 |
| [03-game-design.md](03-game-design.md) | 타워디펜스 규칙, 해넘이, 타워, 적, 경제 |
| [04-size-budget.md](04-size-budget.md) | 1.44MB 바이트 예산과 절감 전략 |
| [05-build-and-harness.md](05-build-and-harness.md) | 빌드, 용량 측정, 실행 검증, 테스트 하네스 |
| [06-decision-log.md](06-decision-log.md) | 확정/보류 의사결정 기록 |
| [07-roadmap.md](07-roadmap.md) | 개발 순서와 마일스톤 |
| [08-risk-register.md](08-risk-register.md) | 주요 리스크와 대응 |
| [09-world-concept.md](09-world-concept.md) | 세계관, 톤, 명명 규칙, 시각 방향 |
| [10-tower-design.md](10-tower-design.md) | 타워 5종 역할, 수치, 해금, 구현 플래그 |
| [11-enemy-design.md](11-enemy-design.md) | 적 8종 역할, 수치, 리소스 재사용 전략 |
| [12-level-design.md](12-level-design.md) | 44해넘이 구조(4달×11), 달별 목표, 밸런스 체크 |
| [13-agent-skill-workflow.md](13-agent-skill-workflow.md) | 로컬 agent skills 사용 전략과 Matt Pocock 계열 스킬 운영 방식 |
| [14-resume-session.md](14-resume-session.md) | 재부팅/새 세션 뒤 바로 이어가기 위한 터미널 명령과 시작 프롬프트 |
| [15-execution-plan.md](15-execution-plan.md) | 구현 전 실행 트랙, 완료 조건, 다음 산출물 결정 |

## 문서 운영 규칙

- 추측은 `추천안` 또는 `가정`으로 표시한다.
- 확정된 내용은 `결정`으로 옮긴다.
- 공모전 원문과 충돌하는 내용은 원문 우선으로 판단하되, 모순점은 별도 기록한다.
- 구현 전에 항상 다음 세 가지를 확인한다.
  - 용량 제한을 깨지 않는가?
  - 독립 실행 파일 조건을 만족하는가?
  - 실제 완성 가능한 범위인가?

## 현재 로컬 환경 관찰

2026-05-27 현재 이 폴더에는 `CONTEST.md`, `AGENTS.md`, `CONTEXT.md`, `docs/`, `.scratch/`가 있다. 로컬에서 확인한 도구 상태는 다음과 같다.

| 도구 | 상태 |
| --- | --- |
| `clang` | 사용 가능: `/usr/bin/clang` |
| `fasm` | 미설치. 프로젝트 표준 어셈블러로 확정 |
| `nasm` | 미설치 |
| `wine` | 미설치 |

따라서 구현을 시작하려면 FASM 설치 또는 프로젝트 로컬 바이너리 확보가 필요하다. 공식 빌드는 프로젝트 로컬 고정 툴체인과 버전/SHA256 기록을 기준으로 한다.
