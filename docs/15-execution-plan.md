# Execution Plan

이 문서는 「어린왕자: 장미의 별」을 실제 완성 가능한 순서로 만들기 위한 실행 계획이다. 구현은 아직 시작하지 않고, 먼저 작업 단위와 검증 기준을 고정한다.

## 계획 원칙

- 첫 목표는 "44해넘이 전체"가 아니라 "1해넘이가 진짜 게임처럼 작동하는 tracer bullet"이다.
- 기술 리스크는 콘텐츠보다 먼저 제거한다.
- 각 단계는 빌드 산출물, 크기 기록, 검증 명령을 남긴다.
- 최종 제출물에는 개발 문서, 하네스, 툴체인을 포함하지 않는다.

## Track A: Project Skeleton

목표:

- Git 저장소 여부 결정
- `src/`, `tools/`, `build/`, `dist/` 구조 생성
- Makefile 공개 명령 확정
- FASM 로컬 툴체인 확보 방식 문서화

완료 조건:

- `make toolchain-check` 명령의 기대 동작이 구현 또는 명확히 stub 처리됨
- `make size`가 `dist/` 크기를 측정함
- `tools/toolchain/README.md`가 FASM 버전/SHA256 기록 위치를 정의함

## Track B: Win32 Framebuffer Tracer Bullet

목표:

- Windows x86 PE 빈 창
- 256x192 프레임버퍼(D046)
- 확대 출력
- 키 입력과 종료

완료 조건:

- `dist/rose-star.exe`가 생성됨
- 프레임 카운터 또는 색상 변화로 화면 갱신을 확인함
- `Esc`로 종료 가능
- 파일 크기가 기록됨

## Track C: Playable One-Sunset Loop

목표:

- 해넘이 1-1 데이터
- 커서 이동
- Build Zone 하이라이트
- 🐑양 설치
- 🌱싹 적 이동
- 승패/재시작

완료 조건:

- 60초 안에 한 판이 끝남
- 실패 후 `R`로 재시작 가능
- Easy Mode와 Hard Mode의 별빛 규칙 차이가 보임

## Track D: Data And Harness

목표:

- 해넘이 command stream 초안
- Level checker
- Replay/checksum 검증
- 크기 예산 추적

완료 조건:

- 해넘이 data가 코드와 분리됨
- Level checker가 Path/Build Zone/Wave 기본 오류를 잡음
- 같은 replay가 같은 checksum을 냄

## Track E: Content Ramp

목표:

- 11개 해넘이 alpha(달1)
- 22개 해넘이 beta(달1~2)
- 44개 해넘이 release(4달×11)
- 타워 5종/적 8종 순차 해금

완료 조건:

- 각 달이 하나의 바오밥 성장단계/학습 테마를 가짐
- 각 해넘이가 하나의 아이디어를 가짐
- Hard Mode도 수동 클리어 가능한 상태로 검증됨

## Track F: Release Hardening

목표:

- 효과음
- Record Code
- BMP Share Card
- exact-fill release
- Windows 실기기/VM 검증

완료 조건:

- `dist/` 총합이 정확히 1,474,560바이트
- 깨끗한 Windows 환경에서 인터넷 없이 실행됨
- 해넘이 1-1과 4-11(거목 보스) 클리어 확인

## Next Decision

Q026. 다음 산출물은 무엇으로 할 것인가?

추천: `to-prd`로 전체 PRD를 만들기 전에, `.scratch/tracer-bullet/` 아래에 첫 playable slice 이슈 세트를 먼저 만든다. 이유는 현재 프로젝트의 가장 큰 리스크가 "아이디어 부족"이 아니라 "FASM/Win32/프레임버퍼/입력/빌드가 실제로 닫히는가"이기 때문이다.
