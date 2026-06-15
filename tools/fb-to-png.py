#!/usr/bin/env python3
"""헤드리스 하네스가 덤프한 raw framebuffer(BGRX dword, 256×192)를 PNG로 변환한다.
사용: python3 tools/fb-to-png.py <fb.raw> <out.png> [scale] [offset]
  offset: 프레임버퍼 앞에 붙은 상태 헤더 바이트 수(stage3+ 하네스는 24).
프레임버퍼 픽셀은 0x00RRGGBB dword → 리틀엔디언 메모리 = B,G,R,0 → Pillow 'BGRX'."""
import sys
from PIL import Image

FB_W, FB_H = 256, 192

def main():
    if len(sys.argv) < 3:
        print("사용: python3 tools/fb-to-png.py <fb.raw> <out.png> [scale] [offset]"); sys.exit(2)
    raw, out = sys.argv[1], sys.argv[2]
    scale = int(sys.argv[3]) if len(sys.argv) > 3 else 3
    offset = int(sys.argv[4]) if len(sys.argv) > 4 else 0
    data = open(raw, "rb").read()[offset:]
    need = FB_W * FB_H * 4
    if len(data) != need:
        print(f"경고: 크기 {len(data)} != 기대 {need}")
    img = Image.frombytes("RGB", (FB_W, FB_H), data[:need], "raw", "BGRX")
    if scale != 1:
        img = img.resize((FB_W*scale, FB_H*scale), Image.NEAREST)
    img.save(out)
    print(f"[fb-to-png] {out} ({img.width}x{img.height})")

if __name__ == "__main__":
    main()
