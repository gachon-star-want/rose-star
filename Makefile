# 어린왕자: 장미의 별 — 빌드 인터페이스 (D021/D022)
# 공개 명령은 얇게 유지하고, 복잡한 로직은 tools/ 스크립트로 분리한다.
# 최종 제출물에는 Makefile/tools/를 포함하지 않는다.

# ── 설정 ───────────────────────────────────────────────
# DIST_LIMIT: 3.5" HD 플로피 물리 용량(바이트), exact-fill 목표 (D004)
# SAFE_LIMIT: 개발 중 안전 상한 (04-size-budget)
# FASM: 프로젝트 로컬 고정 툴체인 (D021). 전역 fasm 사용 금지.
#   바이너리 자체는 Linux ELF32(tools/toolchain/fasm)이고, 맥(arm64)·리눅스 어디서나
#   동일하게 Windows PE를 뽑기 위해 linux/386 Docker 컨테이너 래퍼로 호출한다 (이슈 02).
DIST_LIMIT := 1474560
SAFE_LIMIT := 1400000
EXE        := dist/rose-star.exe
SRC_MAIN   := src/main.asm
FASM       := ./tools/toolchain/fasm-docker.sh

.PHONY: all toolchain-check build smoke render-test render-test-loss \
        harness-1-2 harness-1-3 harness-1-4 harness-1-5 \
        harness-moon1 harness-moon2 harness-moon3 harness-moon4 harness-all \
        harness-code harness-speed \
        harness-aura harness-boost harness-boost-cd harness-callwave \
        harness-boss harness-slow harness-mech \
        harness-evade harness-erratic harness-boss-cascade \
        release size clean release-check help

help:
	@echo "어린왕자: 장미의 별 — 빌드 명령"
	@echo "  make toolchain-check   프로젝트 로컬 FASM(Docker) 존재/유효 확인"
	@echo "  make smoke             FASM이 유효한 Windows PE를 방출하는지 검증"
	@echo "  make render-test       WIN 시나리오 헤드리스 실행 → 상태단언 + build/render-test.png"
	@echo "  make render-test-loss  LOSS 시나리오 헤드리스 실행 → 상태단언 + build/render-loss.png"
	@echo "  make build             $(EXE) 빌드 (FASM 필요, 패딩 없음)"
	@echo "  make release           빌드 + exact-fill 패딩 → 정확히 $(DIST_LIMIT) B 제출물"
	@echo "  make harness-all       44해넘이 전체 Hard 클리어 검증"
	@echo "  make harness-mech      전투 메커닉 10종 검증"
	@echo "  make size              dist/ 총 바이트를 $(DIST_LIMIT) 와 비교"
	@echo "  make release-check     exact-fill( == $(DIST_LIMIT) ) 검증"
	@echo "  make clean             build/ dist/ 산출물 정리"

all: build

# ── 툴체인 ─────────────────────────────────────────────
# 전역 어셈블러를 조용히 쓰지 않는다. 반드시 프로젝트 로컬 FASM(Docker 래퍼)만 인정한다.
toolchain-check:
	@if [ -x "$(FASM)" ]; then \
		echo "[toolchain] OK: $(FASM)"; \
		"$(FASM)" 2>/dev/null | head -n 1 || true; \
	else \
		echo "[toolchain] MISSING: $(FASM)"; \
		echo "  → tools/toolchain/README.md 의 확보 절차를 따르세요 (이슈 02)."; \
		exit 1; \
	fi

# ── 스모크: FASM→Windows PE 파이프라인 검증 (이슈 02) ──
# 동작 게임이 아니라 최소 PE 스텁을 어셈블해 헤더(MZ/PE/i386/섹션)가 유효한지 본다.
smoke: toolchain-check
	@mkdir -p build
	$(FASM) tools/toolchain/_pe-smoke.asm build/_pe-smoke.exe
	@python3 tools/verify-pe.py build/_pe-smoke.exe
	@echo "[smoke] FASM→PE 파이프라인 검증 완료"

# ── 헤드리스 렌더+상태 검증 (맥에서 게임 화면 확인 & 숫자 단언) ──
# 게임 로직(game.inc)을 ELF로 감싸 linux/386 컨테이너에서 SIM_TICKS 회 update→
# 상태헤더(24B)+framebuffer 덤프. check-state.py 가 숫자 단언, fb-to-png.py 가 PNG(offset 24).
render-test: toolchain-check
	@mkdir -p build
	$(FASM) src/_render_harness.asm build/render-harness
	@chmod +x build/render-harness
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/render-harness > build/fb.raw
	@python3 tools/check-state.py build/fb.raw win
	@python3 tools/fb-to-png.py build/fb.raw build/render-test.png 3 24
	@echo "[render-test] build/render-test.png 생성"

harness-1-2: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_1_2.asm build/harness-1-2
	@chmod +x build/harness-1-2
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-1-2 > build/fb-1-2.raw
	@python3 tools/check-state.py build/fb-1-2.raw 1-2-win
	@python3 tools/fb-to-png.py build/fb-1-2.raw build/render-1-2.png 3 24
	@echo "[harness-1-2] OK"

harness-1-3: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_1_3.asm build/harness-1-3
	@chmod +x build/harness-1-3
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-1-3 > build/fb-1-3.raw
	@python3 tools/check-state.py build/fb-1-3.raw 1-3-win
	@python3 tools/fb-to-png.py build/fb-1-3.raw build/render-1-3.png 3 24
	@echo "[harness-1-3] OK"

harness-1-4: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_1_4.asm build/harness-1-4
	@chmod +x build/harness-1-4
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-1-4 > build/fb-1-4.raw
	@python3 tools/check-state.py build/fb-1-4.raw 1-4-win
	@python3 tools/fb-to-png.py build/fb-1-4.raw build/render-1-4.png 3 24
	@echo "[harness-1-4] OK"

harness-1-5: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_1_5.asm build/harness-1-5
	@chmod +x build/harness-1-5
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-1-5 > build/fb-1-5.raw
	@python3 tools/check-state.py build/fb-1-5.raw 1-5-win
	@python3 tools/fb-to-png.py build/fb-1-5.raw build/render-1-5.png 3 24
	@echo "[harness-1-5] OK"

harness-moon1: render-test harness-1-2 harness-1-3 harness-1-4 harness-1-5
	@mkdir -p build
	@$(call HARNESS_CMD,1_6,1-6)
	@$(call HARNESS_CMD,1_7,1-7)
	@$(call HARNESS_CMD,1_8,1-8)
	@$(call HARNESS_CMD,1_9,1-9)
	@$(call HARNESS_CMD,1_10,1-10)
	@$(call HARNESS_CMD,1_11,1-11)
	@echo "[harness-moon1] 달1 전체 (1-1~1-11) LOCKED"

harness-speed: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_speed.asm build/harness-speed
	@chmod +x build/harness-speed
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-speed > build/fb-speed.raw
	@python3 tools/check-state.py build/fb-speed.raw speed
	@echo "[harness-speed] EN_SPROUT/EN_SNAKE 속도 룩업 OK"

harness-aura: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_aura.asm build/harness-aura
	@chmod +x build/harness-aura
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-aura > build/fb-aura.raw
	@python3 tools/check-state.py build/fb-aura.raw aura
	@echo "[harness-aura] EN_VAIN AURA 체비셰프 2타일 OK"

harness-boost: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_boost.asm build/harness-boost
	@chmod +x build/harness-boost
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-boost > build/fb-boost.raw
	@python3 tools/check-state.py build/fb-boost.raw boost
	@echo "[harness-boost] TW_FOX BOOST 체비셰프 1타일 OK"

harness-boss: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_boss.asm build/harness-boss
	@chmod +x build/harness-boss
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-boss > build/fb-boss.raw
	@python3 tools/check-state.py build/fb-boss.raw boss
	@echo "[harness-boss] boss_tick HP 임계 소환 OK"

harness-slow: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_slow.asm build/harness-slow
	@chmod +x build/harness-slow
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-slow > build/fb-slow.raw
	@python3 tools/check-state.py build/fb-slow.raw slow
	@echo "[harness-slow] SLOW 50% + decay_tick OK"

harness-evade: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_evade.asm build/harness-evade
	@chmod +x build/harness-evade
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-evade > build/fb-evade.raw
	@python3 tools/check-state.py build/fb-evade.raw evade
	@echo "[harness-evade] EVADE 양성 경로(218*3/2=327) OK"

harness-erratic: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_erratic.asm build/harness-erratic
	@chmod +x build/harness-erratic
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-erratic > build/fb-erratic.raw
	@python3 tools/check-state.py build/fb-erratic.raw erratic
	@echo "[harness-erratic] EN_DRUNK ERRATIC jitter OK"

harness-boss-cascade: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_boss_cascade.asm build/harness-boss-cascade
	@chmod +x build/harness-boss-cascade
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-boss-cascade > build/fb-boss-cascade.raw
	@python3 tools/check-state.py build/fb-boss-cascade.raw boss-cascade
	@echo "[harness-boss-cascade] 3단계 연쇄 소환 OK"

harness-boost-cd: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_boost_cd.asm build/harness-boost-cd
	@chmod +x build/harness-boost-cd
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-boost-cd > build/fb-boost-cd.raw
	@python3 tools/check-state.py build/fb-boost-cd.raw boost-cd
	@echo "[harness-boost-cd] TW_FOX CD 감소 17←20 OK"

harness-callwave: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_callwave.asm build/harness-callwave
	@chmod +x build/harness-callwave
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-callwave > build/fb-callwave.raw
	@python3 tools/check-state.py build/fb-callwave.raw callwave
	@echo "[harness-callwave] 조기소환 starlight=13/10 OK"

harness-mech: harness-aura harness-boost harness-boost-cd harness-callwave \
              harness-boss harness-slow harness-speed \
              harness-evade harness-erratic harness-boss-cascade
	@echo "[harness-mech] 전투 메커닉 10종 전체 OK"

harness-code: toolchain-check
	@mkdir -p build
	$(FASM) src/_harness_code.asm build/harness-code
	@chmod +x build/harness-code
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness-code > build/fb-code.raw
	@python3 tools/check-state.py build/fb-code.raw code
	@echo "[harness-code] decode_record 4케이스 PASS"

HARNESS_CMD = $(FASM) src/_harness_$(1).asm build/harness_$(1) && \
	chmod +x build/harness_$(1) && \
	docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/harness_$(1) > build/fb_$(1).raw && \
	python3 tools/check-state.py build/fb_$(1).raw $(2)-win && \
	python3 tools/fb-to-png.py build/fb_$(1).raw build/render_$(1).png 3 24 && \
	echo "[harness-$(2)] OK"

harness-moon4: toolchain-check
	@mkdir -p build
	@$(call HARNESS_CMD,4_1,4-1)
	@$(call HARNESS_CMD,4_2,4-2)
	@$(call HARNESS_CMD,4_3,4-3)
	@$(call HARNESS_CMD,4_4,4-4)
	@$(call HARNESS_CMD,4_5,4-5)
	@$(call HARNESS_CMD,4_6,4-6)
	@$(call HARNESS_CMD,4_7,4-7)
	@$(call HARNESS_CMD,4_8,4-8)
	@$(call HARNESS_CMD,4_9,4-9)
	@$(call HARNESS_CMD,4_10,4-10)
	@$(call HARNESS_CMD,4_11,4-11)
	@echo "[harness-moon4] 달4 전체 OK"

harness-all: harness-moon1 harness-moon2 harness-moon3 harness-moon4
	@echo "[harness-all] 44해넘이 전체 LOCKED"

harness-moon3: toolchain-check
	@mkdir -p build
	@$(call HARNESS_CMD,3_1,3-1)
	@$(call HARNESS_CMD,3_2,3-2)
	@$(call HARNESS_CMD,3_3,3-3)
	@$(call HARNESS_CMD,3_4,3-4)
	@$(call HARNESS_CMD,3_5,3-5)
	@$(call HARNESS_CMD,3_6,3-6)
	@$(call HARNESS_CMD,3_7,3-7)
	@$(call HARNESS_CMD,3_8,3-8)
	@$(call HARNESS_CMD,3_9,3-9)
	@$(call HARNESS_CMD,3_10,3-10)
	@$(call HARNESS_CMD,3_11,3-11)
	@echo "[harness-moon3] 달3 전체 OK"

harness-moon2: toolchain-check
	@mkdir -p build
	@$(call HARNESS_CMD,2_1,2-1)
	@$(call HARNESS_CMD,2_2,2-2)
	@$(call HARNESS_CMD,2_3,2-3)
	@$(call HARNESS_CMD,2_4,2-4)
	@$(call HARNESS_CMD,2_5,2-5)
	@$(call HARNESS_CMD,2_6,2-6)
	@$(call HARNESS_CMD,2_7,2-7)
	@$(call HARNESS_CMD,2_8,2-8)
	@$(call HARNESS_CMD,2_9,2-9)
	@$(call HARNESS_CMD,2_10,2-10)
	@$(call HARNESS_CMD,2_11,2-11)
	@echo "[harness-moon2] 달2 전체 OK"

render-test-loss: toolchain-check
	@mkdir -p build
	$(FASM) src/_render_harness_loss.asm build/render-harness-loss
	@chmod +x build/render-harness-loss
	@docker run --rm --platform linux/386 -v "$(CURDIR):/work" -w /work \
		alpine:latest /work/build/render-harness-loss > build/fb-loss.raw
	@python3 tools/check-state.py build/fb-loss.raw loss
	@python3 tools/fb-to-png.py build/fb-loss.raw build/render-loss.png 3 24
	@echo "[render-test-loss] build/render-loss.png 생성"

# ── 빌드 ───────────────────────────────────────────────
build: toolchain-check
	@if [ ! -f "$(SRC_MAIN)" ]; then \
		echo "[build] $(SRC_MAIN) 없음 — 아직 Win32 PE 진입점 미구현 (이슈 03)."; \
		exit 1; \
	fi
	@mkdir -p dist
	$(FASM) "$(SRC_MAIN)" "$(EXE)"
	@$(MAKE) --no-print-directory size

# ── 릴리즈: 빌드 → exact-fill 패딩(lore 임베드) → 정확히 1,474,560B 검증 ──
# 개발 중엔 build(패딩 없이 빠르게), 제출물은 release.
release: build
	@python3 tools/pad-to-size.py
	@$(MAKE) --no-print-directory release-check
	@python3 tools/verify-pe.py "$(EXE)"
	@echo "[release] $(EXE) = 정확히 $(DIST_LIMIT) B, 유효 PE — 제출 준비 완료"

# ── 용량 ───────────────────────────────────────────────
size:
	@total=$$(find dist -type f ! -name '.gitkeep' -exec cat {} + 2>/dev/null | wc -c | tr -d ' '); \
	total=$${total:-0}; \
	remain=$$(( $(DIST_LIMIT) - total )); \
	echo "[size] dist 총합: $$total B / 상한 $(DIST_LIMIT) B"; \
	echo "[size] exact-fill 까지 남은 바이트: $$remain"; \
	if [ "$$total" -gt "$(DIST_LIMIT)" ]; then echo "[size] 초과! 실패"; exit 1; fi; \
	if [ "$$total" -gt "$(SAFE_LIMIT)" ]; then echo "[size] 개발 안전 상한($(SAFE_LIMIT)) 초과 — 주의"; fi

# 릴리즈는 정확히 DIST_LIMIT 와 같아야 한다 (exact-fill, D004).
release-check:
	@total=$$(find dist -type f ! -name '.gitkeep' -exec cat {} + 2>/dev/null | wc -c | tr -d ' '); \
	total=$${total:-0}; \
	echo "[release] dist 총합: $$total B (목표 정확히 $(DIST_LIMIT) B)"; \
	if [ "$$total" -ne "$(DIST_LIMIT)" ]; then echo "[release] exact-fill 불일치 — 실패"; exit 1; fi; \
	echo "[release] exact-fill OK"

clean:
	@rm -f build/* dist/*.exe dist/*.bmp
	@touch build/.gitkeep dist/.gitkeep
	@echo "[clean] build/ dist/ 산출물 정리 완료"
