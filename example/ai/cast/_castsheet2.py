#!/usr/bin/env python3
# 캐스트 2차 대조 시트: 특수적 4 + 타워 5 (밤배경 위 ×10 확대 + 라벨)
# 상태: 특수적4(뱀/허영쟁이/사업가/술꾼)·타워5(양/화산/가로등/사막바람/여우) 생성 완료(48px).
# 사업가/술꾼은 단일 보행 인물로 재생성, 화산/가로등/사막바람은 프린스 누수 제거 재생성.
# 다음: 타일/HUD → 24px·밤하늘 팔레트 통일 정리.
import os
from PIL import Image, ImageDraw

D = os.path.dirname(os.path.abspath(__file__))
NAVY = (10, 10, 46)
INK = (236, 231, 214)

rows = [
    ("ENEMIES", [
        ("Snake", "enemy1-snake.png"),
        ("Vain man", "enemy2-vain.png"),
        ("Businessman", "enemy3-businessman.png"),
        ("Tippler", "enemy4-tippler.png"),
    ]),
    ("TOWERS", [
        ("Sheep", "tower1-sheep.png"),
        ("Volcano", "tower2-volcano.png"),
        ("Lamp", "tower3-lamp.png"),
        ("Wind", "tower4-wind.png"),
        ("Fox", "tower5-fox.png"),
    ]),
]

def load(fn): return Image.open(os.path.join(D, fn)).convert("RGBA")
def up(img, s): return img.resize((img.width*s, img.height*s), Image.NEAREST)

S = 10
cell = 48*S
gap = 24
lab = 26
hdr = 30
cols = max(len(r[1]) for r in rows)
W = cols*cell + (cols+1)*gap
H = len(rows)*(hdr+cell+lab) + (len(rows)+1)*gap
sheet = Image.new("RGBA", (W, H), NAVY+(255,))
dr = ImageDraw.Draw(sheet)

y = gap
for title, items in rows:
    dr.text((gap, y), title, fill=(150, 170, 220, 255))
    y += hdr
    x = gap
    for label, fn in items:
        img = up(load(fn), S)
        sheet.alpha_composite(img, (x, y))
        dr.text((x, y + cell + 6), label, fill=INK+(255,))
        x += cell + gap
    y += cell + lab + gap

sheet.save(os.path.join(D, "_castsheet2.png"))
print("OK: example/ai/cast/_castsheet2.png", sheet.size)
