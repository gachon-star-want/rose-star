# 도트 시안 (example/)

「어린왕자: 장미의 별」의 **밤하늘 동화** 도트 에셋이다(D070/D072). 제작 방식은 **AI 생성 베이스 + 정리**(D075, 도구=pixelforge). 어린왕자 캐논(`ai/prince-iconic.png`, D077)을 스타일 레퍼런스로 전 캐스트를 통일 생성하고, 우리 사이즈·가독성으로 다듬었다.

> `ai/render.py`(손작성 16×16 그리드)는 **폐기**됨(D073→D075). 무시할 것.

## 폴더

- **`ai/cast/`** — 48px 마스터(생성 원본, 보존). 캐릭터/적/타워 + 16px 타일 2 + 24px HUD 2.
- **`ai/cast/cast24/`** — **게임용 24px 세트**(48→24 정수 2× BOX 다운스케일, D076/D078). 엔진이 쓸 최종 사이즈.
- 원본 백업: `enemy4-tippler_raw.png`(보강 전 술꾼).

## 먼저 볼 것

- **`ai/cast/_scene24.png`** — 24px 네이티브 인게임 목업. 밤하늘 + 모래 경로 + 빌드존에 적·타워·HUD 배치. **가독성 실증**(D072)의 기준 그림.
- **`ai/cast/_castsheet2.png`** — 적4·타워5 대조 시트(×10 확대).

## 캐스트 (14)

| 분류 | 파일 |
| --- | --- |
| 코어/플레이어 | `rose` / `ai/prince-iconic`(캐논) |
| 바오밥 4성장 | `baobab1-sprout` → `baobab2-seedling` → `baobab3-sapling` → `baobab4-giant` |
| 특수 적 4 | `enemy1-snake` 🐍 / `enemy2-vain` 🎩 / `enemy3-businessman` 💼 / `enemy4-tippler` 🍷 |
| 타워 5 | `tower1-sheep` 🐑 / `tower2-volcano` 🌋 / `tower3-lamp` 💡 / `tower4-wind` 🏜️ / `tower5-fox` 🦊 |
| 타일 2 | `tile-path-sand`(모래 경로) / `tile-ground`(행성 지면) — 16px |
| HUD 2 | `hud-starlight` ✨ / `hud-petal` 🌹 — 24px |

## 디자인 수정하는 법

1. 특정 스프라이트를 다시 뽑고 싶으면 그냥 말해줘("여우 더 크게" "사막바람 색을 더 밝게"). pixelforge로 재생성한다.
   - 주의: **오브젝트 타워(화산/가로등/바람)는 prince 레퍼를 빼고** 바오밥 레퍼로 생성해야 꼬마 왕자가 안 섞인다(레퍼런스 누수 전례).
2. 정리 스크립트를 다시 돌리면 cast24 갱신: `python3 example/ai/cast/_cleanup.py`
3. 목업 재확인: `python3 example/ai/cast/_scene24.py`

## 아직 안 한 것

- **팔레트 통일(하드 양자화)** — asm 렌더러 팔레트 포맷이 결정될 때 명시적 hex 리스트로 박고 그때 양자화(D078).
- 타이틀/결과 화면, 보너스 모드 스킨(점등인의 별/사막의 신기루), 애니메이션 프레임.
