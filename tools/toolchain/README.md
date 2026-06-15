# FASM 프로젝트 로컬 툴체인

빌드 재현성을 위해 시스템 전역 `fasm`에 의존하지 않고, **프로젝트 로컬 고정 툴체인**만 사용한다(D021). `make toolchain-check`는 전역 어셈블러를 조용히 쓰지 않으며, 여기 명시된 경로의 FASM만 인정한다.

## 구성

```
tools/toolchain/fasm            # FASM 실행 파일 (Linux ELF32 바이너리)
tools/toolchain/fasm-docker.sh  # 이 바이너리를 linux/386 컨테이너로 감싸는 래퍼 (Makefile의 FASM=)
tools/toolchain/_pe-smoke.asm   # PE 방출 검증용 최소 스텁 (make smoke)
```

`fasm` 바이너리는 `.gitignore` 대상이다(머신별·재취득 가능). 래퍼 스크립트는 추적한다.

## 왜 Docker인가

개발 머신은 **macOS arm64**인데 FASM은 공식 macOS 빌드를 제공하지 않는다(Windows/Linux/DOS만). FASM은 **어느 OS에서 돌리든 출력은 `format PE`로 Windows x86 exe**를 낸다 — 그래서 Linux 바이너리를 `linux/386` 컨테이너(qemu-i386)에서 돌려 Windows PE를 뽑는다. FASM 1은 libc를 링크하지 않고 raw syscall만 쓰므로 Alpine i386 위에서 그대로 실행된다. 이 결정은 핵심 타깃(Windows PE, D005)·어셈블러(FASM, D007)를 바꾸지 않고 빌드 경로만 해결한다.

> 전제: 호스트에 Docker Desktop이 켜져 있어야 한다(`open -a Docker`). 래퍼가 데몬 미가동 시 안내한다.

## 확보 기록 (이슈 02 완료)

```
fasm version : 1.73.32
sha256       : 235a37bbe4bbe10467d2884537c66672b815aa0ad780666233499c3d69094937
source       : https://flatassembler.net/fasm-1.73.32.tgz  (fasm/fasm 의 Linux ELF32)
runtime      : linux/386 Docker (alpine:latest, qemu-i386) — 호스트 macOS arm64
acquired     : 2026-06-02
```

재취득 절차(바이너리 분실 시):

```sh
docker run --rm --platform linux/386 -v "$PWD/tools/toolchain:/out" alpine:latest sh -c '
  apk add --no-cache wget ca-certificates tar >/dev/null 2>&1
  wget -qO /tmp/f.tgz https://flatassembler.net/fasm-1.73.32.tgz
  tar xzf /tmp/f.tgz -C /tmp && cp /tmp/fasm/fasm /out/fasm && chmod +x /out/fasm'
shasum -a 256 tools/toolchain/fasm   # 위 sha256 과 일치 확인
```

## 검증

```sh
make toolchain-check   # fasm-docker.sh 가 FASM 버전을 출력하면 OK
make smoke             # 최소 PE 스텁을 어셈블 → tools/verify-pe.py 가 MZ/PE/i386/섹션 확인
```

## 빌드 경로 메모

- 최종 산출물은 **Windows x86 PE `.exe`**(`dist/rose-star.exe`)다.
- **빌드는 맥에서 가능**(Docker), 그러나 **실행/플레이 테스트는 Windows 환경**이 필요하다 — arm 맥에서 x86 Windows exe를 네이티브 실행할 수 없다. 후반(이슈 03+)에 UTM/Parallels(Windows-on-ARM, x86 에뮬) 또는 실기기로 확인한다.
- 툴체인은 **최종 제출물에 포함하지 않는다** — `dist/`의 실행 파일만 제출한다.
