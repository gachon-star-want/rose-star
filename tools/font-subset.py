#!/usr/bin/env python3
"""
tools/font-subset.py — docs/09 UI 문자열에서 고유 음절 추출 → 비트맵 폰트 .inc 생성
사용: python3 tools/font-subset.py [--output src/font_data.inc]
"""
import sys, os, re

# docs/09 UI 문자열 (확정 + 새 문자열 포함)
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
    # 새 문자열 (D080)
    "다음 무리", "멈췄다", "다시 시작", "이어하기",
    "기록 코드를 넣으면 이어갈 수 있다",
    "코드가 맞지 않는다",
    "N달 N해넘이까지 클리어한 기록",
    "별빛이 N 왔다",
    "새로 시작",
]

def extract_syllables(strings):
    syllables = set()
    for s in strings:
        for ch in s:
            cp = ord(ch)
            if 0xAC00 <= cp <= 0xD7A3:  # 한글 음절
                syllables.add(ch)
    return sorted(syllables)

def decompose_hangul(syllable):
    """한글 음절 → (초성, 중성, 종성) 인덱스"""
    cp = ord(syllable) - 0xAC00
    jong = cp % 28
    jung = (cp // 28) % 21
    cho  = cp // 28 // 21
    return cho, jung, jong

# 12×12 한글 비트맵 생성 (단순화된 형태 — 실제론 폰트 데이터 필요)
# 여기서는 구조를 잡고, 실제 픽셀은 임시로 채움 (추후 진짜 폰트로 교체)
def make_dummy_glyph_12x12(syllable):
    """12×12 1bpp 글리프 24바이트 생성 (임시)"""
    cho, jung, jong = decompose_hangul(syllable)
    bitmap = []
    for row in range(12):
        # 단순 패턴: 초성/중성/종성 구역별 선 그리기
        if row < 5:     # 초성 구역
            mask = (0b111111000000 >> (cho % 6)) & 0xFFF
        elif row < 9:   # 중성 구역
            mask = (0b100000100000 >> (jung % 3)) & 0xFFF
        else:           # 종성 구역
            mask = (0b111100000000 >> (jong % 4)) & 0xFFF if jong else 0
        # 12비트 → 2바이트 (상위 4비트 0 패딩)
        bitmap.append((mask >> 4) & 0xFF)
        bitmap.append((mask << 4) & 0xF0)
    return bytes(bitmap)

def make_ascii_glyph_12x12(ch):
    """ASCII 문자 12×12 1bpp (단순 박스)"""
    if ch == ' ':
        return bytes(24)
    # 단순히 문자 코드 기반 패턴
    bitmap = []
    code = ord(ch)
    for row in range(12):
        if row == 0 or row == 11:
            mask = 0b111111110000
        elif row == 6:
            mask = 0b100000010000 if (code & 0x40) else 0b111111110000
        else:
            bit = (code >> (row % 7)) & 1
            mask = (0b100000000000 | (bit << 4))
        bitmap.append((mask >> 4) & 0xFF)
        bitmap.append((mask << 4) & 0xF0)
    return bytes(bitmap)

def gen_font_inc(output_path):
    syllables = extract_syllables(UI_STRINGS)

    # ASCII 서브셋
    ascii_chars = list("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZcs[ ]:-/ ")

    lines = [
        "; font_data.inc — 비트맵 폰트 서브셋 (12×12 1bpp, 24B/글리프)",
        "; 자동 생성: tools/font-subset.py",
        f"; 한글: {len(syllables)}자, ASCII: {len(ascii_chars)}자",
        "",
        f"FONT_GLYPH_COUNT = {len(syllables) + len(ascii_chars)}",
        "FONT_GLYPH_BYTES = 24",
        "",
        "; 글리프 테이블: [codepoint_u16(2B)][bitmap(24B)] × N",
        "font_glyph_table:",
    ]

    # 한글 (정렬 순서 = 코드포인트 오름차순)
    for syl in syllables:
        cp = ord(syl)
        glyph = make_dummy_glyph_12x12(syl)
        hex_bytes = ', '.join(f'0x{b:02X}' for b in glyph)
        lines.append(f"    dw 0x{cp:04X}  ; '{syl}'")
        lines.append(f"    db {hex_bytes}")

    # ASCII
    for ch in sorted(set(ascii_chars), key=ord):
        cp = ord(ch)
        glyph = make_ascii_glyph_12x12(ch)
        hex_bytes = ', '.join(f'0x{b:02X}' for b in glyph)
        disp = ch if ch.isprintable() and ch != "'" else f'#{cp}'
        lines.append(f"    dw 0x{cp:04X}  ; '{disp}'")
        lines.append(f"    db {hex_bytes}")

    lines.append("")
    lines.append(f"; 총 {len(syllables) + len(ascii_chars)}글리프 × 26B = {(len(syllables)+len(ascii_chars))*26}B")

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines) + '\n')

    print(f"[font-subset] {len(syllables)} 한글 + {len(ascii_chars)} ASCII → {output_path}")
    print(f"[font-subset] 한글 음절: {''.join(syllables)}")

if __name__ == '__main__':
    out = 'src/font_data.inc'
    if '--output' in sys.argv:
        idx = sys.argv.index('--output')
        out = sys.argv[idx+1]
    gen_font_inc(out)
