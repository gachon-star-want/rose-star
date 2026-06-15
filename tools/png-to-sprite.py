#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""cast24 PNG → src/sprites.inc (4bpp 인덱스 스프라이트 + 16색 서브팔레트).

docs/18 §4 스프라이트 포맷 구현:
  - 4bpp 인덱스, 인덱스 0 = 투명. 24×24=288B/스프라이트, 16×16 타일=128B.
  - 스프라이트마다 16색 서브팔레트(idx0=투명 placeholder, idx1~15=양자화색).
  - 알파 < 임계 → 투명(idx0). 불투명 색은 median-cut으로 ≤15색 양자화.
  - 팩: byte = (왼픽셀<<4)|오른픽셀. 행 폭이 짝수라 깔끔(24→12B, 16→8B).
  - 가독성(D072): 불투명 평균/최대 명도를 배경(C_GROUND)과 비교해 리포트.

사용: python3 tools/png-to-sprite.py [--out src/sprites.inc]
"""
import glob
import os
import sys

from PIL import Image

SRC_DIR = "example/ai/cast/cast24"
OUT_DEFAULT = "src/sprites.inc"
ALPHA_THRESH = 128
C_GROUND = (0x14, 0x2A, 0x1E)  # 밤하늘 배경 (game_defs.inc C_GROUND)

# 파일 stem → asm 식별자. 게임은 spr_<id> / pal_<id> 로 참조한다.
NAMES = [
    ("baobab1-sprout", "sprout"),
    ("baobab2-seedling", "seedling"),
    ("baobab3-sapling", "sapling"),
    ("baobab4-giant", "giant"),
    ("enemy1-snake", "snake"),
    ("enemy2-vain", "vain"),
    ("enemy3-businessman", "biz"),
    ("enemy4-tippler", "drunk"),
    ("tower1-sheep", "sheep"),
    ("tower2-volcano", "volcano"),
    ("tower3-lamp", "lamp"),
    ("tower4-wind", "wind"),
    ("tower5-fox", "fox"),
    ("rose", "rose"),
    ("hud-petal", "petal"),
    ("hud-starlight", "starlight"),
    ("tile-ground", "ground"),
    ("tile-path-sand", "path"),
]
# 적(가독성 점검 대상)
ENEMY_IDS = {"sprout", "seedling", "sapling", "giant", "snake", "vain", "biz", "drunk"}


def luma(rgb):
    r, g, b = rgb
    return (r * 30 + g * 59 + b * 11) // 100


def quantize(im):
    """RGBA → (palette[16], index_grid[h][w]). idx0=투명, idx1..15=색."""
    w, h = im.size
    px = im.load()
    opaque = []  # (x,y,rgb)
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a >= ALPHA_THRESH:
                opaque.append((x, y, (r, g, b)))
    # 불투명 색을 median-cut 15색으로
    colors = [c for (_, _, c) in opaque]
    uniq = list(dict.fromkeys(colors))
    if len(uniq) <= 15:
        pal_colors = uniq
    else:
        tmp = Image.new("RGB", (len(colors), 1))
        tmp.putdata(colors)
        q = tmp.quantize(colors=15, method=Image.MEDIANCUT)
        rawpal = q.getpalette()
        used = sorted(set(q.getdata()))
        pal_colors = [tuple(rawpal[i * 3:i * 3 + 3]) for i in used]
        pal_colors = pal_colors[:15]
    # 16엔트리 팔레트: idx0=투명(0), idx1.. = pal_colors
    palette = [(0, 0, 0)] + pal_colors
    while len(palette) < 16:
        palette.append((0, 0, 0))

    def nearest(rgb):
        best, bi = 1 << 30, 1
        for i in range(1, len(pal_colors) + 1):
            pr, pg, pb = palette[i]
            d = (pr - rgb[0]) ** 2 + (pg - rgb[1]) ** 2 + (pb - rgb[2]) ** 2
            if d < best:
                best, bi = d, i
        return bi

    grid = [[0] * w for _ in range(h)]
    for (x, y, rgb) in opaque:
        grid[y][x] = nearest(rgb)
    return palette, grid, opaque


def pack4bpp(grid):
    w = len(grid[0])
    h = len(grid)
    out = bytearray()
    for y in range(h):
        x = 0
        while x < w:
            hi = grid[y][x] & 0xF
            lo = grid[y][x + 1] & 0xF if x + 1 < w else 0
            out.append((hi << 4) | lo)
            x += 2
    return bytes(out)


def hexdd(rgb):
    r, g, b = rgb
    return "0x%02X%02X%02X" % (r, g, b)


def emit_bytes(byts):
    lines = []
    for i in range(0, len(byts), 12):
        chunk = byts[i:i + 12]
        lines.append("    db " + ",".join("0x%02X" % b for b in chunk))
    return "\n".join(lines)


def main():
    out_path = OUT_DEFAULT
    if "--out" in sys.argv:
        out_path = sys.argv[sys.argv.index("--out") + 1]

    blocks = []
    report = []
    for stem, ident in NAMES:
        path = os.path.join(SRC_DIR, stem + ".png")
        if not os.path.isfile(path):
            print("MISSING:", path, file=sys.stderr)
            sys.exit(1)
        im = Image.open(path).convert("RGBA")
        w, h = im.size
        palette, grid, opaque = quantize(im)
        byts = pack4bpp(grid)
        # 가독성 리포트
        if opaque:
            ls = [luma(c) for (_, _, c) in opaque]
            avg, mx = sum(ls) // len(ls), max(ls)
            bg = luma(C_GROUND)
            tag = ""
            if ident in ENEMY_IDS and mx - bg < 60:
                tag = "  ⚠ 적 최대명도-배경 < 60 (가독성 D072 확인)"
            report.append("  %-9s opaque=%3d avgL=%3d maxL=%3d bgL=%3d%s"
                          % (ident, len(opaque), avg, mx, bg, tag))
        # asm 블록
        b = []
        b.append("; %s  (%dx%d, 불투명 %d px)" % (stem, w, h, len(opaque)))
        b.append("pal_%s:" % ident)
        b.append("    dd " + ", ".join(hexdd(c) for c in palette))
        b.append("spr_%s:  ; %dx%d 4bpp, %dB" % (ident, w, h, len(byts)))
        b.append(emit_bytes(byts))
        blocks.append("\n".join(b))

    header = (
        "; ─────────────────────────────────────────────────────────────────────\n"
        "; sprites.inc — cast24 PNG → 4bpp 인덱스 스프라이트 + 16색 서브팔레트.\n"
        ";   자동 생성: tools/png-to-sprite.py (직접 편집 금지).\n"
        ";   포맷: pal_<id> = 16색(dd, idx0=투명), spr_<id> = 4bpp(db, 왼<<4|오른).\n"
        ";   스프라이트 24x24(12B/행), 타일 16x16(8B/행). blit_sprite 가 소비.\n"
        "; ─────────────────────────────────────────────────────────────────────\n"
    )
    with open(out_path, "w") as f:
        f.write(header + "\n" + "\n\n".join(blocks) + "\n")
    print("[png-to-sprite] %s 생성 (%d 스프라이트)" % (out_path, len(NAMES)))
    print("[가독성 리포트]")
    print("\n".join(report))


if __name__ == "__main__":
    main()
