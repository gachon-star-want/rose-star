#!/usr/bin/env python3
import os
import sys

DIST_LIMIT = 1474560
EXE_PATH = "dist/rose-star.exe"
LORE_PATH = "docs/16-lore-bible.md"

def pad_to_exact_size():
    if not os.path.exists(EXE_PATH):
        print(f"[error] {EXE_PATH} 파일이 존재하지 않습니다. 먼저 빌드(make build)를 진행하세요.")
        sys.exit(1)
        
    current_size = os.path.getsize(EXE_PATH)
    print(f"[exact-fill] 현재 파일 크기: {current_size} B")
    
    if current_size > DIST_LIMIT:
        print(f"[error] 현재 크기가 {DIST_LIMIT} B를 초과했습니다 ({current_size} B). 빌드 예산을 줄이십시오.")
        sys.exit(1)
        
    if current_size == DIST_LIMIT:
        print("[exact-fill] 이미 정확히 1,474,560 B에 맞춰져 있습니다.")
        sys.exit(0)
        
    required_padding = DIST_LIMIT - current_size
    print(f"[exact-fill] 필요한 패딩 크기: {required_padding} B")
    
    # Lore 로드
    lore_data = b""
    if os.path.exists(LORE_PATH):
        with open(LORE_PATH, "rb") as f:
            lore_data = f.read()
            
    # 크레딧 추가
    credits_text = (
        b"\n\n"
        b"==========================================\n"
        b"                 CREDITS                  \n"
        b"==========================================\n"
        b"  Developer: Antigravity Pair-Programming\n"
        b"  Title: The Little Prince: The Rose Star\n"
        b"==========================================\n"
    )
    
    pad_data = lore_data + credits_text
    
    if len(pad_data) >= required_padding:
        # 패딩보다 lore+크레딧이 더 크면 필요한 크기만큼만 자름
        pad_data = pad_data[:required_padding]
    else:
        # 부족한 경우 0x00 바이트로 채움
        rem = required_padding - len(pad_data)
        pad_data = pad_data + (b"\x00" * rem)
        
    # 파일 끝에 쓰기
    with open(EXE_PATH, "ab") as f:
        f.write(pad_data)
        
    new_size = os.path.getsize(EXE_PATH)
    print(f"[exact-fill] 패딩 적용 완료. 새 파일 크기: {new_size} B")
    
    if new_size != DIST_LIMIT:
        print(f"[error] 크기 맞추기 실패: {new_size} != {DIST_LIMIT}")
        sys.exit(1)
        
    print("[exact-fill] exact-fill 성공!")

if __name__ == "__main__":
    pad_to_exact_size()
