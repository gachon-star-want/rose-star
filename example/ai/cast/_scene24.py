#!/usr/bin/env python3
# 24px 네이티브 인게임 재검증 (D078): cast24/ 를 스케일 1.0로 256x192에 배치 후 ×4 확대.
import os, random
from PIL import Image, ImageDraw

D = os.path.dirname(os.path.abspath(__file__))
C = os.path.join(D, "cast24")
NAVY = (10, 12, 40)
INK = (236, 231, 214)

def load(fn): return Image.open(os.path.join(C, fn)).convert("RGBA")
def up(img, s): return img.resize((img.width*s, img.height*s), Image.NEAREST)

bw, bh, SC = 256, 192, 4
scene = Image.new("RGBA", (bw, bh), NAVY+(255,))
dr = ImageDraw.Draw(scene)

random.seed(7)
for _ in range(40):
    x, y = random.randint(0, bw-1), random.randint(0, 110)
    dr.point((x, y), fill=random.choice([(255,233,168),(200,210,255),(255,255,255)])+(255,))

ground, sand = load("tile-ground.png"), load("tile-path-sand.png")
for ty in range(7, bh//16+1):
    for tx in range(bw//16+1):
        scene.alpha_composite(ground, (tx*16, ty*16))
for ty in (7, 8):
    for tx in range(bw//16+1):
        scene.alpha_composite(sand, (tx*16, ty*16))

def paste(fn, cx, by):  # 바닥-중앙 앵커, 네이티브 24px
    s = load(fn)
    scene.alpha_composite(s, (int(cx - s.width/2), int(by - s.height)))

y_walk = 9*16 + 6
for fn, cx in [("enemy1-snake.png",40),("baobab4-giant.png",90),
               ("enemy3-businessman.png",140),("enemy4-tippler.png",186),("rose.png",234)]:
    paste(fn, cx, y_walk)

def bz(x, y): dr.rectangle([x, y, x+22, y+22], fill=(26,39,71,255), outline=(93,114,173,255))
for i, fn in enumerate(["tower1-sheep.png","tower2-volcano.png","tower3-lamp.png","tower5-fox.png"]):
    bx = 24 + i*56
    bz(bx, 150); paste(fn, bx+11, 174)

dr.rectangle([0,0,bw,16], fill=(6,8,26,235))
scene.alpha_composite(load("hud-starlight.png"), (2,-4)); dr.text((22,4),"120",fill=INK+(255,))
scene.alpha_composite(load("hud-petal.png"), (64,-4)); dr.text((84,4),"5",fill=INK+(255,))
dr.text((bw-60,4),"해넘이 12",fill=INK+(255,))

up(scene, SC).save(os.path.join(D, "_scene24.png"))
print("OK: example/ai/cast/_scene24.png")
