#!/usr/bin/env python3
# 인게임 미리보기: 밤배경 + 타일 경로 + 빌드존에 적/타워/HUD 배치 → 가독성 실증
import os
from PIL import Image, ImageDraw

D = os.path.dirname(os.path.abspath(__file__))
NAVY = (10, 12, 40)
INK = (236, 231, 214)

def load(fn): return Image.open(os.path.join(D, fn)).convert("RGBA")
def up(img, s): return img.resize((img.width*s, img.height*s), Image.NEAREST)

# 프레임버퍼 256x192, 4배 확대
bw, bh = 256, 192
SC = 4
scene = Image.new("RGBA", (bw, bh), NAVY+(255,))
dr = ImageDraw.Draw(scene)

# 별
import random
random.seed(7)
for _ in range(40):
    x, y = random.randint(0, bw-1), random.randint(0, 110)
    b = random.choice([(255,233,168),(200,210,255),(255,255,255)])
    dr.point((x, y), fill=b+(255,))

# 타일 깔기: 하단 지면, 그 위 모래 경로 띠
ground = load("tile-ground.png").resize((16,16))
sand = load("tile-path-sand.png").resize((16,16))
for ty in range(7, bh//16+1):
    for tx in range(bw//16+1):
        scene.alpha_composite(ground, (tx*16, ty*16))
# 모래 경로: 가운데를 가로지르는 2타일 높이 띠 + 꺾임
path_rows = {7:(0,16), 8:(0,16)}
for ty,(a,b) in path_rows.items():
    for tx in range(bw//16+1):
        scene.alpha_composite(sand, (tx*16, ty*16))

def paste(fn, cx, by, scale=1.0):
    s = load(fn)
    if scale != 1.0:
        s = s.resize((int(s.width*scale), int(s.height*scale)), Image.NEAREST)
    scene.alpha_composite(s, (int(cx - s.width/2), int(by - s.height)))

# 경로 위 적들 (걸어오는 줄), 24px로 축소 미리보기
half = 0.5  # 48->24
y_walk = 9*16
paste("enemy1-snake.png", 40, y_walk, half)
paste("baobab4-giant.png", 90, y_walk, half)
paste("enemy3-businessman.png", 140, y_walk, half)
paste("enemy4-tippler.png", 188, y_walk, half)
# 장미(코어) 경로 끝
paste("rose.png", 236, y_walk, half)

# 빌드존 위 타워들
def bz(x, y):
    dr.rectangle([x, y, x+22, y+22], fill=(26,39,71,255), outline=(93,114,173,255))
for i,(fn) in enumerate(["tower1-sheep.png","tower2-volcano.png","tower3-lamp.png","tower5-fox.png"]):
    bx = 24 + i*56
    bz(bx, 150)
    paste(fn, bx+11, 172, half)

# HUD 상단 바
dr.rectangle([0,0,bw,14], fill=(6,8,26,235))
paste("hud-starlight.png", 12, 14, 0.5); dr.text((20,3), "120", fill=INK+(255,))
paste("hud-petal.png", 70, 14, 0.5); dr.text((78,3), "5", fill=INK+(255,))
dr.text((bw-70,3), "해넘이 12", fill=INK+(255,))

scene = up(scene, SC)
scene.save(os.path.join(D, "_scene.png"))
print("OK: example/ai/cast/_scene.png", scene.size)
