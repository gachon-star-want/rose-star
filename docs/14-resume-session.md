# Resume Session

컴퓨터를 껐다가 다시 켠 뒤, 이 프로젝트를 바로 이어가기 위한 명령어와 시작 프롬프트다.

## Terminal Command

터미널을 열고 아래 명령을 실행한다.

```sh
cd /Users/lee_wonyoung/Developer/game/1.44mb
claude
```

## First Prompt

새 세션이 시작되면 아래 프롬프트를 그대로 붙여넣는다.

```text
이 프로젝트 이어서 진행하자. 작업 디렉터리는 /Users/lee_wonyoung/Developer/game/1.44mb 이다.

먼저 다음을 읽고 현재 상태를 파악해라:
- docs/06-decision-log.md 의 D045~D060 (특히 D055~D060)
- docs/14-resume-session.md, CONTEXT.md
- 메모리: little-prince-td-pivot

현재 상태: 1.44MB GAME_DEV CONTEST 제출작을 「어린왕자: 장미의 별」(순수 우화 타워디펜스)로 피벗 완료. 한국 한정 제출 법적 안전 확인됨(D048). 디지털 명명(Floppy Defense/BYTES/Disk/Sector/Bit/Glitch)을 우화 명명으로 일괄 리네이밍 + 레벨 구조 64→44 개편까지 모든 docs+CONTEXT.md에 반영 완료.

이미 확정한 것 (docs/06-decision-log.md):
- 제목: 「어린왕자: 장미의 별」 (D058)
- 표시: 4:3 / 256x192 / 창모드 전용 / 정수배 4배 (D046)
- 톤: 순수 우화, 레트로는 비주얼만, 표기 "바오밥" (D049)
- 코어=🌹장미 / 플레이어=어린왕자 (D050)
- 주적 바오밥 4성장단계 (D051)
- 레벨: 44해넘이 = 4달 × 11 (달=막=바오밥단계), 44번째=거목보스 (D059)
- 적 8종 = 바오밥4 + 특수4(뱀/허영쟁이/사업가/술꾼) (D053)
- 타워 5종 = 양/화산/가로등/사막바람/여우 (D054)
- 자원: ✨별빛(Starlight), 생명: 🌹장미 꽃잎 5장 고정 (D055/D050/D063)
- 보너스: Endless=🏮점등인의 별(22번째 해금) / Random=🏜️사막의 신기루(33번째 해금) (D057/D059)
- 리네이밍 사전 + 파생숫자(128→88 등) (D060)
- UI: 한글 기본 + 사용음절 서브셋 비트맵 폰트, 모드 쉬움/어려움, 승="별을 지켰다"/패="별이 어두워졌다", 전체 문자열은 09 "UI 문자열 표"(사람 말투, AI틱 금지) (D061/D062)
- 경제: 시작 별빛 달1~4=60/90/130/180, Easy 초당+2/Hard 0, 처치보상 소량, 판매 60% (D064); 밸런스 기준값은 12 "밸런스 기준값"
- 입력: 키보드+마우스 둘 다 기본(좌클릭 설치/우클릭 판매) (D066)
- 튜토리얼 없음: 마리오 1-1식 플레이 중 자연 학습, 달1이 학습 곡선 (D065)
- 서사: 빙산형 — 깊은 백그라운드 lore는 docs/16-lore-bible.md(소설 수준 OK), 게임 속엔 최소한만(개연성만), 09에 표면 초안 (D068/D071). 떠나는 대신 남아서 장미 지킨다로 재해석
- 난이도: 정통 TD답게 도전적, Hard 빡세되 "Hard에서도 클리어 가능" 공정성 유지 (D069)
- 아트: "밤하늘 동화" 제한 팔레트, 원작 PD 삽화 기반 도트(영화판 금지, D070), **가독성 최우선**(어두워도 적·타워·경로 고대비, 바오밥이 남청에 안 묻히게 — D072). **에셋은 Executor가 16×16 데이터로 직접 작성, 외부 AI 안 씀**(D073)
- 운영: **원샷 출시** — 버전관리·업데이트 없음, 처음부터 완벽하게 한 방 (D074). git은 백업 스냅샷일 뿐 워크플로 아님
- 아트 제작: **AI 베이스+정리**(내 손작성 폐기 D075). 도구=연결된 **pixelforge**(Gemini), pixellab/higgsfield/직접작성 제외. **어린왕자 캐논=example/ai/prince-iconic.png(D077)** 를 레퍼런스로 전 캐스트 통일, 48px 생성→24px+밤하늘 팔레트 정리. **진행: 전체 캐스트+타일+HUD 생성 완료(example/ai/cast/)** — 장미+바오밥4+특수적4(뱀/허영쟁이/사업가/술꾼)+타워5(양/화산/가로등/사막바람/여우)+타일2(모래경로/지면)+HUD2(별빛/꽃잎). 인게임 목업 _scene.png 가독성 검증됨. **정리 패스 완료(D078, raw 승인 후)**: 술꾼 보강 + 48→24px BOX 다운스케일 → **example/ai/cast/cast24/(게임용)**, 48px 마스터 cast/ 보존, 24px 목업 _scene24.png 가독성 검증됨. 팔레트 하드 양자화만 asm 렌더러 팔레트 결정 시점으로 연기. **아트 트랙 사실상 완료** → 다음 작업축은 구현 트랙. 미리보기 example/ai/cast/_scene24.py·_castsheet2.py, 정리 _cleanup.py

a(문자열)·b(밸런스) 완료. **구현 트랙 Track A·B(이슈 02·03) 완료(2026-06-02):**
- **Track A 툴체인**: FASM 1.73.32(Linux ELF32, `tools/toolchain/fasm`)를 `linux/386` Docker 래퍼(`tools/toolchain/fasm-docker.sh`)로 macOS arm64에서 실행 → Windows PE 빌드. INCLUDE는 Windows zip(`fasmw17332.zip`)에서 추출(`tools/toolchain/include`), APFS 대소문자 무관이라 소문자 include OK. `make toolchain-check`/`make smoke`(→`tools/verify-pe.py`) 통과. **타깃(Windows PE)·어셈블러(FASM) 재론 안 함 — 빌드 경로만 Docker로 해결**(advisor 확인).
- **Track B 창**: `src/main.asm` = 256×192 top-down DIB를 `StretchDIBits` nearest로 1024×768(AdjustWindowRect)에 4배, PeekMessage 루프 + ESC/WM_PAINT/WM_DESTROY, 비대칭 테스트 패턴. `make build`→`dist/rose-star.exe`(3072B) 유효 PE(gdi32/user32/kernel32 import). **런타임은 Windows 환경(UTM/Parallels) 대기 — 맥 arm에서 x86 exe 실행 불가.**

**Track C 진행 중(Playable One-Sunset Loop, 4단계):**
- **seam 분리**: `src/game.inc`(게임 로직, Win32-free)+`src/game_defs.inc`(상수), `src/main.asm`=플랫폼만.
- **stage1 맵 렌더 완료**: 1-1 맵(경로 가로직선·장미 우측·빌드존4칸), `fill_rect` primitive.
- **★헤드리스 검증 파이프라인**: `make render-test` → `src/_render_harness.asm`(ELF)이 game.inc 돌려 framebuffer 덤프 → `tools/fb-to-png.py` → `build/render-test.png`. **맥에서 게임화면을 눈으로 확인 가능**(Windows 없이). 색·방향 검증됨.
- 4단계: ①맵✓ ②입력+커서+🐑양 설치✓ ③🌱싹 스폰/경로 이동 ④양 공격+별빛 경제+꽃잎/승패/재시작(=한 판 성립). 1-1 값=별빛60·꽃잎5·양20·싹(보상1/leak1).
- **stage2 완료**: 플랫폼이 마우스(÷64)/키보드(방향+Space+S)를 `in_cursor_cx/cy`·`in_place`·`in_sell`(edge)로 정규화(D066), game은 입력원 무관. 설치(빌드존&빈칸&별빛≥20)/판매(+12)·HUD 별빛막대·커서 테두리·양 렌더, 헤드리스로 경제(60→12) 검증.
- **stage3+4 완료(한 판 클리어 가능 성립)**: 적 spawn/move(누수→꽃잎-1)/fire(per-tower 쿨다운·체비셰프3·pos최대타깃·처치→별빛+1)/winloss, update에 game_state 게이트+틱순서(입력→spawn→move→fire→winloss). 렌더에 적(연두6×6)·꽃잎 pip·승(초록)/패(적색) 4px 테두리 프레임. main.asm에 R재시작+Sleep16. **검증=숫자 단언**: `_render_harness_body.inc`(SCENARIO 0/1) 850틱 후 상태헤더 덤프→`tools/check-state.py`. WIN=WON/petals5/starlight8/active0/to_spawn0, LOSS=LOST/petals0/starlight60/active3 둘 다 ✓ (`make render-test`/`render-test-loss`). 텍스트 결과문구(D062)는 서브셋폰트 후, 입력/창/Sleep 실동작·QPC는 Windows VM 대기.
- **다음 = 다음 해넘이(웨이브) 또는 스프라이트 PNG→asm+팔레트(D078).** 해넘이별 웨이브는 12 체크리스트로. asm 규율: fixed-point pos(×256)·클릭매핑(÷4÷16, 상단1행 UI)·init_level로 R재시작.
한국어로 한 번에 하나씩, 추천안 함께, 확정 시 06에 D번호 기록. 인프라/툴체인은 Executor 자율(D045). docs는 AI 구현 스펙이라 구체값까지(메모리 docs-are-ai-build-spec), 카피는 사람 말투(메모리 copy-voice-human-not-ai).
```

## 현재까지의 큰 그림 요약

| 항목 | 결정 |
| --- | --- |
| 제목 | 「어린왕자: 장미의 별」 (D058) |
| 장르 | 고정 경로 마이크로 타워디펜스 (어셈블리/FASM, Windows x86 PE .exe) |
| 세계관 | 어린왕자 순수 우화 (한국 PD, D048/D049), 무대 B-612 정주 방어 (D056) |
| 표시 | 4:3 256x192, 창모드 전용, 정수배 4배 (D046) |
| 코어 / 플레이어 | 🌹 장미 / 👦 어린왕자 (D050) |
| 주적 | 🌳 바오밥 4성장단계 (D051) |
| 레벨 구조 | 44해넘이 = 4달 × 11 (달=막=바오밥단계), 44번째=거목보스 (D059) |
| 적 8종 | 바오밥4 + 🐍뱀·🎩허영쟁이·💼사업가·🍷술꾼 (D053) |
| 타워 5종 | 🐑양·🌋화산·💡가로등·🏜️사막바람·🦊여우 (D054) |
| 자원 / 생명 | ✨별빛(Starlight) / 🌹장미 꽃잎 5장 고정 (D055/D050/D063) |
| 보너스 모드 | Endless=🏮점등인의 별(22번째 해금) / Random=🏜️사막의 신기루(33번째 해금) (D057/D059) |
| UI/문자열 | 한글+서브셋 폰트, 쉬움/어려움, 승="별을 지켰다"/패="별이 어두워졌다", 사람 말투 (D061/D062, 표는 09) |
| 경제 | 시작별빛 60/90/130/180, Easy +2/초·Hard 0, 판매 60% (D064, 기준값 12) |
| 입력 | 키보드+마우스 둘 다 기본 (D066) |
| 튜토리얼 | 없음 — 마리오 1-1식 자연 학습 (D065) |
| 리네이밍 | 디지털→우화 사전 + 파생숫자 일괄 반영 완료 (D060) |
| 구현 | Track A 스켈레톤 done(git+Makefile+dist/rose-star.exe, D067) |
| 다음 | 이슈 02 FASM 확보 → 이슈 03 Win32 PE 창 |
