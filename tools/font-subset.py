#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""tools/font-subset.py — 게임 표면 문자열의 고유 음절을 실제 한글 TTF로 12×12 1bpp
글리프로 래스터화해 src/font_data.inc 생성.

docs/09 UI 표 + 서사 문안(도입/막간3/엔딩2)에서 음절을 모은다. (D061/D068/D080)
글리프 = 실제 폰트(AppleSDGothicNeo)를 12×12에 맞춰 임계화 → draw_char 규약과 일치:
  행당 12비트(bit11=좌측 col0), byte0=(v>>4), byte1=(v<<4)&0xF0. 글리프당 24B.
사용: python3 tools/font-subset.py [--output src/font_data.inc] [--preview build/font.png]
"""
import os
import sys

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_story import STORY_SCENES  # 서사 음절 단일 소스

FONT_CANDS = [
    "/System/Library/Fonts/AppleSDGothicNeo.ttc",
    "/System/Library/Fonts/Supplemental/AppleGothic.ttf",
    "/Library/Fonts/AppleGothic.ttf",
]
RENDER_SIZE = 16   # 큰 사이즈로 렌더 후 12×12로 축소(더 또렷)
THRESH = 96

# ── docs/09 UI 문자열 (확정, D061/D062/D080) ──
UI_STRINGS = [
    "어린왕자: 장미의 별",
    "아무 키나 누르면 시작한다",
    "쉬움", "어려움",
    "별빛이 천천히 차오른다",
    "오직 네 손으로",
    "달 1", "달 2", "달 3", "달 4",
    "해넘이 N-N",
    "별빛 N", "꽃잎 N", "무리 N/N",
    "타워 N", "판매 N", "코드 N",
    "별빛이 모자라다", "꽃잎이 진다", "장미가 위태롭다",
    "바오밥을 막았다", "별빛이 돌아왔다",
    "별을 지켰다", "별이 어두워졌다",
    "달이 하나 저물었다", "오늘도 별은 무사하다",
    "카드를 남겼다", "카드를 남기지 못했다",
    "다음 무리", "멈췄다", "다시 시작", "이어하기",
    "기록 코드를 넣으면 이어갈 수 있다",
    "코드가 맞지 않는다",
    "N달 N해넘이까지 클리어한 기록",
    "별빛이 N 왔다",
    "새로 시작",
    "점등인의 별", "사막의 신기루",
]

# ── 서사 문안 (도입/막간1~3/엔딩2) — gen_story.STORY_SCENES 단일 소스 ──
NARRATIVE_STRINGS = [ln for scene in STORY_SCENES for ln in scene]

ASCII_CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZcs[]:-/ .,"


def find_font():
    for c in FONT_CANDS:
        if os.path.exists(c):
            return c
    raise SystemExit("한글 TTF를 찾지 못함: " + ", ".join(FONT_CANDS))


def extract_hangul(strings):
    s = set()
    for st in strings:
        for ch in st:
            if 0xAC00 <= ord(ch) <= 0xD7A3:
                s.add(ch)
    return sorted(s)


def render_rows(ch, fnt):
    """ch → 12행 × 12비트(bit11=좌측). 실제 폰트 래스터화 후 12×12 맞춤."""
    if ch == ' ':
        return [0] * 12
    C = 28
    img = Image.new("L", (C, C), 0)
    d = ImageDraw.Draw(img)
    d.text((C // 2, C // 2), ch, font=fnt, fill=255, anchor="mm")
    bbox = img.getbbox()
    if bbox is None:
        return [0] * 12
    crop = img.crop(bbox)
    w, h = crop.size
    scale = min(12.0 / w, 12.0 / h)
    nw, nh = max(1, round(w * scale)), max(1, round(h * scale))
    crop = crop.resize((nw, nh), Image.LANCZOS)
    g = Image.new("L", (12, 12), 0)
    g.paste(crop, ((12 - nw) // 2, (12 - nh) // 2))
    px = g.load()
    rows = []
    for y in range(12):
        v = 0
        for x in range(12):
            if px[x, y] >= THRESH:
                v |= (1 << (11 - x))
        rows.append(v)
    return rows


def rows_to_bytes(rows):
    out = []
    for v in rows:
        out.append((v >> 4) & 0xFF)
        out.append((v << 4) & 0xF0)
    return out


def main():
    out = "src/font_data.inc"
    preview = None
    if "--output" in sys.argv:
        out = sys.argv[sys.argv.index("--output") + 1]
    if "--preview" in sys.argv:
        preview = sys.argv[sys.argv.index("--preview") + 1]

    font_path = find_font()
    fnt = ImageFont.truetype(font_path, RENDER_SIZE)

    hangul = extract_hangul(UI_STRINGS + NARRATIVE_STRINGS)
    ascii_chars = sorted(set(ASCII_CHARS), key=ord)
    # codepoint 오름차순 정렬 (draw_char 이진탐색 전제) — ASCII < 한글
    glyphs = []
    for ch in ascii_chars:
        glyphs.append((ord(ch), ch))
    for ch in hangul:
        glyphs.append((ord(ch), ch))
    glyphs.sort(key=lambda t: t[0])

    lines = [
        "; font_data.inc — 비트맵 폰트 서브셋 (12×12 1bpp, 24B/글리프)",
        "; 자동 생성: tools/font-subset.py (실제 TTF 래스터화, 직접 편집 금지)",
        f"; 한글 {len(hangul)}자 + ASCII {len(ascii_chars)}자 = {len(glyphs)}글리프",
        "",
        f"FONT_GLYPH_COUNT = {len(glyphs)}",
        "FONT_GLYPH_BYTES = 24",
        "",
        "; 글리프 테이블: [codepoint_u16(2B)][bitmap(24B)] × N, codepoint 오름차순",
        "font_glyph_table:",
    ]
    for cp, ch in glyphs:
        b = rows_to_bytes(render_rows(ch, fnt))
        disp = ch if (ch.isprintable() and ch not in "';") else f"#{cp}"
        lines.append(f"    dw 0x{cp:04X}  ; '{disp}'")
        lines.append("    db " + ", ".join(f"0x{x:02X}" for x in b))
    with open(out, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print(f"[font-subset] {len(hangul)} 한글 + {len(ascii_chars)} ASCII → {out}  (폰트: {os.path.basename(font_path)})")

    if preview:
        # 미리보기: 글리프를 격자로 12×12씩 배치(×4 확대)
        cols = 24
        rows_n = (len(glyphs) + cols - 1) // cols
        scale = 4
        canvas = Image.new("RGB", (cols * 13 * scale, rows_n * 13 * scale), (10, 14, 30))
        dr = ImageDraw.Draw(canvas)
        for i, (cp, ch) in enumerate(glyphs):
            gr = render_rows(ch, fnt)
            cx, cy = (i % cols) * 13 * scale, (i // cols) * 13 * scale
            for y in range(12):
                for x in range(12):
                    if gr[y] & (1 << (11 - x)):
                        dr.rectangle([cx + x * scale, cy + y * scale,
                                      cx + x * scale + scale - 1, cy + y * scale + scale - 1],
                                     fill=(255, 230, 160))
        canvas.save(preview)
        print(f"[font-subset] 미리보기 → {preview}")


if __name__ == "__main__":
    main()
