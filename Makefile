# 어린왕자: 장미의 별 — 빌드 인터페이스 (D021/D022)
# 공개 명령은 얇게 유지하고, 복잡한 로직은 tools/ 스크립트로 분리한다.
# 최종 제출물에는 Makefile/tools/를 포함하지 않는다.

# ── 설정 ───────────────────────────────────────────────
# DIST_LIMIT: 3.5" HD 플로피 물리 용량(바이트), exact-fill 목표 (D004)
# SAFE_LIMIT: 개발 중 안전 상한 (04-size-budget)
# FASM: 프로젝트 로컬 고정 툴체인 (D021). 전역 fasm 사용 금지.
DIST_LIMIT := 1474560
SAFE_LIMIT := 1400000
EXE        := dist/rose-star.exe
SRC_MAIN   := src/main.asm
FASM       := tools/toolchain/fasm

.PHONY: all toolchain-check build size clean release-check help

help:
	@echo "어린왕자: 장미의 별 — 빌드 명령"
	@echo "  make toolchain-check   프로젝트 로컬 FASM 존재/유효 확인"
	@echo "  make build             $(EXE) 빌드 (FASM 필요)"
	@echo "  make size              dist/ 총 바이트를 $(DIST_LIMIT) 와 비교"
	@echo "  make release-check     exact-fill( == $(DIST_LIMIT) ) 검증"
	@echo "  make clean             build/ dist/ 산출물 정리"

all: build

# ── 툴체인 ─────────────────────────────────────────────
# 전역 어셈블러를 조용히 쓰지 않는다. 반드시 프로젝트 로컬 FASM만 인정한다.
toolchain-check:
	@if [ -x "$(FASM)" ]; then \
		echo "[toolchain] OK: $(FASM)"; \
		"$(FASM)" 2>/dev/null | head -n 1 || true; \
	else \
		echo "[toolchain] MISSING: $(FASM)"; \
		echo "  → tools/toolchain/README.md 의 확보 절차를 따르세요 (이슈 02)."; \
		exit 1; \
	fi

# ── 빌드 ───────────────────────────────────────────────
build: toolchain-check
	@if [ ! -f "$(SRC_MAIN)" ]; then \
		echo "[build] $(SRC_MAIN) 없음 — 아직 Win32 PE 진입점 미구현 (이슈 03)."; \
		exit 1; \
	fi
	@mkdir -p dist
	"$(FASM)" "$(SRC_MAIN)" "$(EXE)"
	@$(MAKE) --no-print-directory size

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
