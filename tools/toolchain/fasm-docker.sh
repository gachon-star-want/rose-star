#!/bin/sh
# FASM(Linux ELF32)을 linux/386 컨테이너에서 실행하는 래퍼 (이슈 02 / D045).
# 맥(arm64)에는 네이티브 FASM이 없으므로 Docker로 감싸 어디서든 동일하게 Windows PE를 뽑는다.
# 출력 포맷은 호스트 OS와 무관 — 컨테이너 안 FASM이 'format PE'로 Windows x86 exe를 방출한다.
# 프로젝트 루트를 /work 로 마운트하고, 받은 인자(루트 기준 상대경로)를 그대로 FASM에 넘긴다.
#   사용: fasm-docker.sh <source.asm> [output.exe]
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
FASM_BIN="tools/toolchain/fasm"

if [ ! -x "$ROOT/$FASM_BIN" ]; then
  echo "[fasm] $FASM_BIN 없음 — tools/toolchain/README.md 의 확보 절차를 따르세요." >&2
  exit 1
fi
if ! docker info >/dev/null 2>&1; then
  echo "[fasm] Docker 데몬이 꺼져 있습니다. Docker Desktop을 켜세요:  open -a Docker" >&2
  exit 1
fi

# INCLUDE: FASM 표준 Win32 헤더(win32ax.inc 등). Windows 배포본에서 받아 둠.
# 마운트 볼륨이 macOS APFS(대소문자 무관)라 소문자 include 'win32ax.inc' 가 그대로 해석된다.
exec docker run --rm --platform linux/386 \
  -e INCLUDE=/work/tools/toolchain/include \
  -v "$ROOT:/work" -w /work \
  alpine:latest "/work/$FASM_BIN" "$@"
