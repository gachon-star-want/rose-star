#!/usr/bin/env python3
# 「어린왕자: 장미의 별」 도트 시안 렌더러 (D070/D072/D073)
# - 외부 라이브러리 0 (stdlib zlib/struct만). 에셋은 16x16 문자 그리드 = 팔레트 인덱스 데이터.
# - 그리드/팔레트를 고치고 다시 실행하면 example/*.png 가 갱신된다.  실행:  python3 example/render.py
import zlib, struct, os

OUT = os.path.dirname(os.path.abspath(__file__))

# ── 밤하늘 동화 팔레트 (가독성 우선, D072) ───────────────────────────
# 배경은 어둡고 채도 낮게(후퇴), 전경 유닛은 밝고 또렷하게(전진). '.' = 투명.
PALETTE = {
    'N': "#0E1630",  # 밤하늘 배경(짙은 남청)
    'n': "#243156",  # 흐린 별/성운
    '*': "#FFE9A8",  # 밝은 별
    's': "#CBA96B",  # 경로(모래빛) — 배경과 명도 대비 확실
    'd': "#8A6E40",  # 경로 그림자/외곽
    'b': "#1A2747",  # 빌드존 채움(배경보다 한 톤 밝게)
    'e': "#5D72AD",  # 빌드존 외곽선
    'y': "#FFD24A",  # 별빛(자원/HUD) — 고채도 노랑
    'r': "#E8413A",  # 장미 붉음
    'R': "#A8262A",  # 장미 어두운 붉음
    'm': "#F26B5E",  # 장미 하이라이트
    'g': "#3E8E5A",  # 초록(잎/줄기)
    'G': "#235437",  # 어두운 초록
    'h': "#F4D06A",  # 어린왕자 금발
    'f': "#F2C49B",  # 얼굴
    'c': "#2E7D4F",  # 외투(초록)
    'C': "#194E32",  # 외투 그림자
    'k': "#FFD24A",  # 노란 목도리(별빛과 같은 노랑)
    't': "#7A5230",  # 바오밥 줄기
    'T': "#4A2F18",  # 바오밥 줄기 어두움
    'o': "#6E8C3E",  # 바오밥 잎
    'O': "#3F5524",  # 바오밥 잎 어두움
    'w': "#CFBE92",  # 림 라이트(어두운 유닛이 밤배경에 안 묻히게 — D072 핵심)
    'x': "#120E1C",  # 어두운 외곽선
    'W': "#ECE7D6",  # 양털 크림/UI 흰
}
def rgb(hexs): return (int(hexs[1:3],16), int(hexs[3:5],16), int(hexs[5:7],16))
RGB = {k: rgb(v) for k, v in PALETTE.items()}

# ── 스프라이트 (16x16 문자 그리드) ───────────────────────────────────
SPRITES = {}
SPRITES['prince'] = [
 "................",
 "......hhhh......",
 ".....hhhhhh.....",
 "....hhhhhhhh....",
 "....hffffffh....",
 "....ffffffff....",
 ".....ffffff.....",
 "....kkkkkkkk....",
 "...wccccccccw...",
 "...wccccccccw...",
 "...wccccccccw...",
 "....cccccccc....",
 "....cCCCCCCc....",
 ".....C....C.....",
 ".....C....C.....",
 "....xx....xx....",
]
SPRITES['rose'] = [
 "................",
 "......rrrr......",
 ".....rRRRRr.....",
 "....rRrrrrRr....",
 "....rRrmmrRr....",
 "....rRRrrRRr....",
 ".....rRRRRr.....",
 "......rrrr......",
 ".......gg.......",
 ".......gg.......",
 "....g..gg..g....",
 "...gg..gg..gg...",
 ".......gg.......",
 ".......gg.......",
 "......wggw......",
 "................",
]
SPRITES['baobab-sprout'] = [
 "................",
 "................",
 "................",
 "................",
 "................",
 "................",
 "................",
 "................",
 "......oo.oo.....",
 ".....woOoOow....",
 "......oOOOo.....",
 ".......ttt......",
 ".......tTt......",
 ".......tTt......",
 "......wtTtw.....",
 "......ddddd.....",
]
SPRITES['baobab-grown'] = [
 "................",
 ".....oooooo.....",
 "...woooooooow...",
 "..woOOOOOOOOow..",
 "..wOOoooooOOOw..",
 "..woooooooooow..",
 "...woOOOOOOow...",
 "....woooooow....",
 "......tttt......",
 ".....wtTTtw.....",
 ".....wtTTtw.....",
 ".....wtTTtw.....",
 "....wtTTTTtw....",
 "....wtTTTTtw....",
 "...wtTTTTTTtw...",
 "..wttTTTTTTttw..",
]
SPRITES['sheep'] = [
 "................",
 "................",
 "......WWWW......",
 "....WWWWWWWW....",
 "...WWWWWWWWWW...",
 "..WWWWWWWWWWWW..",
 "..WWWWWWWWWWff..",
 "..WWWWWWWWWWfx..",
 "..WWWWWWWWWWff..",
 "...WWWWWWWWWW...",
 "..WWWWWWWWWWWW..",
 "...WWWWWWWWWW...",
 "....x..xx..x....",
 "....x..xx..x....",
 "................",
 "................",
]

# ── 검증: 모든 그리드 16x16, 알려진 문자만 ──────────────────────────
for name, g in SPRITES.items():
    assert len(g) == 16, f"{name}: 행 수 {len(g)} (16이어야)"
    for i, row in enumerate(g):
        assert len(row) == 16, f"{name} row{i}: 길이 {len(row)} (16이어야): {row!r}"
        for ch in row:
            assert ch == '.' or ch in RGB, f"{name} row{i}: 미정의 문자 {ch!r}"

# ── 의존성 없는 PNG 라이터 (RGBA) ───────────────────────────────────
def write_png(path, w, h, px):  # px: list of (r,g,b,a)
    def chunk(typ, data):
        c = typ + data
        return struct.pack(">I", len(data)) + c + struct.pack(">I", zlib.crc32(c) & 0xffffffff)
    raw = bytearray()
    for y in range(h):
        raw.append(0)
        for x in range(w):
            r, g, b, a = px[y*w + x]
            raw += bytes((r, g, b, a))
    out = (b"\x89PNG\r\n\x1a\n"
           + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
           + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
           + chunk(b"IEND", b""))
    with open(path, "wb") as fp:
        fp.write(out)

def render_sprite(grid, scale, bg=None):
    w = h = 16
    base = []
    for row in grid:
        for ch in row:
            if ch == '.':
                base.append(bg + (255,) if bg else (0, 0, 0, 0))
            else:
                base.append(RGB[ch] + (255,))
    # scale up (nearest)
    W, H = w*scale, h*scale
    px = [None]*(W*H)
    for y in range(H):
        for x in range(W):
            px[y*W + x] = base[(y//scale)*w + (x//scale)]
    return W, H, px

# ── 개별 스프라이트 PNG (투명 배경 + 밤배경 버전 둘 다) ──────────────
SCALE = 16
for name, g in SPRITES.items():
    W, H, px = render_sprite(g, SCALE, bg=None)
    write_png(os.path.join(OUT, f"{name}.png"), W, H, px)
    W, H, px = render_sprite(g, SCALE, bg=RGB['N'])  # 밤배경 위 가독성 확인용
    write_png(os.path.join(OUT, f"{name}-on-night.png"), W, H, px)

# ── 팔레트 스와치 ────────────────────────────────────────────────────
keys = [k for k in PALETTE]
sw = 24
cols = len(keys)
W, H = cols*sw, sw
px = [(0,0,0,0)]*(W*H)
for i, k in enumerate(keys):
    for y in range(sw):
        for x in range(sw):
            px[y*W + i*sw + x] = RGB[k] + (255,)
write_png(os.path.join(OUT, "palette.png"), W, H, px)

# ── 인게임 시안: 밤배경 + 경로 + 빌드존 + 유닛 배치 (가독성 실증) ────
TW, THh = 14, 8                      # 타일 14x8
T = 16                               # 타일 16px
SW, SH = TW*T, THh*T
scene = [RGB['N'] + (255,)] * (SW*SH)   # 밤하늘로 채움
def put(x, y, c):
    if 0 <= x < SW and 0 <= y < SH: scene[y*SW + x] = c
# 별 몇 개
for (sx, sy, c) in [(20,18,'*'),(60,30,'n'),(110,14,'*'),(150,40,'n'),(200,20,'*'),(40,60,'n'),(180,70,'*')]:
    put(sx, sy, RGB[c] + (255,))
# 경로(모래) : 타일행 5 가로 + 외곽
for tx in range(TW):
    for yy in range(T):
        for xx in range(T):
            X, Y = tx*T+xx, 5*T+yy
            edge = (yy == 0 or yy == T-1)
            put(X, Y, RGB['d' if edge else 's'] + (255,))
# 빌드존 : 타일 (3,4),(8,4)
for (tx, ty) in [(3,4),(8,4)]:
    for yy in range(T):
        for xx in range(T):
            edge = (xx==0 or yy==0 or xx==T-1 or yy==T-1)
            put(tx*T+xx, ty*T+yy, RGB['e' if edge else 'b'] + (255,))
def stamp(name, tx, ty):
    g = SPRITES[name]
    for i, row in enumerate(g):
        for j, ch in enumerate(row):
            if ch != '.':
                put(tx*T+j, ty*T+i, RGB[ch] + (255,))
# 배치: 양(빌드존), 경로 위 바오밥, 밤배경 위 거목(최악 가독성), 어린왕자+장미
stamp('sheep', 3, 4)
stamp('sheep', 8, 4)
stamp('baobab-sprout', 5, 5)
stamp('baobab-grown', 9, 5)
stamp('baobab-grown', 1, 1)     # 밤배경 위 — 림 라이트로 안 묻히는지 확인
stamp('baobab-sprout', 11, 1)   # 밤배경 위 작은 싹
stamp('prince', 12, 5)
stamp('rose', 13, 5)
# 확대
S = 5
W, H = SW*S, SH*S
big = [None]*(W*H)
for y in range(H):
    for x in range(W):
        big[y*W + x] = scene[(y//S)*SW + (x//S)]
write_png(os.path.join(OUT, "scene.png"), W, H, big)

print("OK — example/ 에 PNG 생성:")
for f in sorted(os.listdir(OUT)):
    if f.endswith(".png"):
        print("  example/" + f)
