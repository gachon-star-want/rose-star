# FASM 프로젝트 로컬 툴체인

빌드 재현성을 위해 시스템 전역 `fasm`에 의존하지 않고, **프로젝트 로컬 고정 툴체인**만 사용한다(D021). `make toolchain-check`는 전역 어셈블러를 조용히 쓰지 않으며, 여기 명시된 경로의 FASM만 인정한다.

## 기대 위치

```
tools/toolchain/fasm        # macOS/Linux 실행 파일 (또는 fasm.exe)
```

이 바이너리는 `.gitignore` 대상이다(머신별·대용량). 각자 환경에서 아래 절차로 확보한다.

## 확보 절차 (이슈 02에서 자동화 예정)

1. flat assembler 공식 배포본(https://flatassembler.net/)에서 대상 플랫폼용 FASM을 받는다.
2. 받은 실행 파일을 `tools/toolchain/fasm` 로 둔다(실행 권한 부여: `chmod +x`).
3. **버전과 SHA256을 아래에 기록**한다(재현성).

```
fasm version : (예: 1.73.32)
sha256       : (record here)
platform     : (예: macos-arm64 via Wine / linux-x64 / windows-x64)
acquired     : YYYY-MM-DD
```

## 빌드 경로 메모

- 최종 산출물은 **Windows x86 PE `.exe`**(`dist/rose-star.exe`)다.
- macOS에서 FASM 실행 경로가 애매하면 Linux/Windows VM 또는 Wine 빌드 경로를 여기에 고정한다(D021).
- 툴체인은 **최종 제출물에 포함하지 않는다** — `dist/`의 실행 파일만 제출한다.
