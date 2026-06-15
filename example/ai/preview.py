#!/usr/bin/env python3
# AI 도트 시안 확대 미리보기 (Pillow). 실행: python3 example/ai/preview.py
import os
from PIL import Image, ImageDraw

D = os.path.dirname(os.path.abspath(__file__))
NAVY = (14, 22, 48)
SAND = (203, 169, 107)
SAND_D = (138, 110, 64)
BZ = (26, 39, 71)
BZ_E = (93, 114, 173)

def load(name): return Image.open(os.path.join(D, name)).convert("RGBA")
def up(img, s): return img.resize((img.width*s, img.height*s), Image.NEAREST)

sprites = [
    ("Prince", "prince24.png"),
    ("Rose", "rose24.png"),
    ("Sprout (baobab)", "baobab-sprout24.png"),
    ("Baobab (grown)", "baobab-grown24.png"),
]

# ── 대조 시트 (각 ×12, 밤배경) ──────────────────────────
S = 12
cell = 24*S
gap = 28
lab = 34
W = len(sprites)*cell + (len(sprites)+1)*gap
H = cell + lab + gap*2
sheet = Image.new("RGBA", (W, H), NAVY + (255,))
dr = ImageDraw.Draw(sheet)
x = gap
for label, fn in sprites:
    img = up(load(fn), S)
    sheet.alpha_composite(img, (x, gap))
    dr.text((x, gap + cell + 8), label, fill=(236, 231, 214, 255))
    x += cell + gap
sheet.save(os.path.join(D, "preview.png"))

# ── 인게임 미리보기 (밤배경+경로+빌드존, 가독성 실증) ────
bw, bh = 320, 176
sc = 4
scene = Image.new("RGBA", (bw, bh), NAVY + (255,))
dr = ImageDraw.Draw(scene)
# 별
for (sx, sy) in [(30,20),(90,40),(160,16),(220,30),(270,60),(50,80)]:
    dr.point((sx, sy), fill=(255, 233, 168, 255))
# 경로(모래 띠)
py = 120
dr.rectangle([0, py, bw, py+28], fill=SAND)
dr.rectangle([0, py, bw, py+1], fill=SAND_D); dr.rectangle([0, py+27, bw, py+28], fill=SAND_D)
# 빌드존 2칸
for bx in (70, 150):
    dr.rectangle([bx, 86, bx+24, 110], fill=BZ, outline=BZ_E)
def paste(fn, x, y):
    s = load(fn); scene.alpha_composite(s, (x, y))
# 밤배경 위(가독성 최악): 거목 + 싹
paste("baobab-grown24.png", 24, 28)
paste("baobab-sprout24.png", 250, 36)
# 경로 위
paste("baobab-sprout24.png", 110, 116)
paste("baobab-grown24.png", 175, 112)
paste("prince24.png", 258, 116)
paste("rose24.png", 288, 116)
scene = up(scene, sc)
scene.save(os.path.join(D, "scene.png"))
print("OK: example/ai/preview.png , example/ai/scene.png")
