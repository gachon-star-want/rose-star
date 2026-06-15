#!/usr/bin/env python3
# 아트 정리 패스 (D078): (1) 술꾼 림라이트+섀도 리프트 보강  (2) 48px→24px 정수 2× BOX 다운스케일 → cast24/
# 팔레트 하드 양자화는 안 함(엔진 팔레트 결정 시점으로 연기). 원본 48px는 cast/ 보존.
import os, shutil
import numpy as np
from PIL import Image

D = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(D, "cast24")
os.makedirs(OUT, exist_ok=True)

# 48px 캐릭터/적/타워 (24px로 다운스케일 대상)
SPR48 = [
    "rose.png",
    "baobab1-sprout.png", "baobab2-seedling.png", "baobab3-sapling.png", "baobab4-giant.png",
    "enemy1-snake.png", "enemy2-vain.png", "enemy3-businessman.png", "enemy4-tippler.png",
    "tower1-sheep.png", "tower2-volcano.png", "tower3-lamp.png", "tower4-wind.png", "tower5-fox.png",
]
# 그대로 복사(타일 16px 논리 그리드 / HUD 이미 24px)
KEEP = ["tile-path-sand.png", "tile-ground.png", "hud-starlight.png", "hud-petal.png"]

def shift(m, dy, dx):
    """m을 (dy,dx)만큼 이동, 바깥은 False/0으로 채움."""
    out = np.zeros_like(m)
    ys = slice(max(dy,0), m.shape[0]+min(dy,0))
    xs = slice(max(dx,0), m.shape[1]+min(dx,0))
    ys2 = slice(max(-dy,0), m.shape[0]+min(-dy,0))
    xs2 = slice(max(-dx,0), m.shape[1]+min(-dx,0))
    out[ys, xs] = m[ys2, xs2]
    return out

def boost_tippler(src):
    """저대비 술꾼: 불투명부 섀도 리프트(감마) + 상/좌 가장자리 따뜻한 림라이트(D072)."""
    im = Image.open(src).convert("RGBA")
    arr = np.array(im).astype(np.float32)
    rgb, a = arr[..., :3], arr[..., 3]
    mask = a > 100

    # 섀도 리프트: 어두운 톤을 들어올림(감마<1), 불투명부만
    g = 0.80
    lifted = 255.0 * np.clip(rgb / 255.0, 0, 1) ** g
    # 살짝 채도/온기: 어두운 갈색이 묻히지 않게 미세하게 따뜻하게
    lifted[..., 0] = np.clip(lifted[..., 0] * 1.05, 0, 255)   # R↑
    rgb2 = np.where(mask[..., None], lifted, rgb)

    # 림라이트: 위/왼쪽 가장자리(불투명인데 그 방향 이웃이 투명)
    top_edge  = mask & ~shift(mask, -1, 0)   # 위가 비었음
    left_edge = mask & ~shift(mask, 0, -1)   # 왼쪽이 비었음
    rim = top_edge | left_edge
    warm = np.array([248, 232, 180], np.float32)
    k = 0.55
    rgb2 = np.where(rim[..., None], rgb2 * (1 - k) + warm * k, rgb2)

    out = np.concatenate([np.clip(rgb2, 0, 255), a[..., None]], axis=-1).astype(np.uint8)
    return Image.fromarray(out, "RGBA")

def downscale(im):
    return im.resize((24, 24), Image.BOX)

# (1) 술꾼 보강 — 원본 백업 후 48px 마스터 갱신
tip = os.path.join(D, "enemy4-tippler.png")
raw = os.path.join(D, "enemy4-tippler_raw.png")
if not os.path.exists(raw):
    shutil.copy2(tip, raw)
boost_tippler(raw).save(tip)
print("boosted tippler (raw backup: enemy4-tippler_raw.png)")

# (2) 다운스케일 → cast24/
for fn in SPR48:
    downscale(Image.open(os.path.join(D, fn)).convert("RGBA")).save(os.path.join(OUT, fn))
for fn in KEEP:
    shutil.copy2(os.path.join(D, fn), os.path.join(OUT, fn))
print(f"cast24/: {len(SPR48)} sprites @24px + {len(KEEP)} kept (tiles/HUD)")
