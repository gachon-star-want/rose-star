#!/usr/bin/env python3
"""헤드리스 하네스 덤프 앞 24바이트(6 dword 상태 헤더)를 읽어 기대값과 단언한다.
사용: python3 tools/check-state.py <fb.raw> <scenario>
헤더 순서: game_state, petals, starlight, active_count, to_spawn, en_pos[0]

시나리오 이름 규칙:
  <level>-win   : 해당 레벨 WIN 검증 (예: 1-1-win, 1-2-win)
  <level>-loss  : 해당 레벨 LOSS 검증 (예: 1-1-loss)
  또는 구버전 이름 win/loss (= 1-1-win/1-1-loss 로 해석)
"""
import sys, struct

GS_PLAY  = 0
GS_WON   = 1
GS_LOST  = 2
GS = {0: "PLAY", 1: "WON", 2: "LOST", 3: "TITLE", 4: "PAUSE"}
FIELDS = ["game_state", "petals", "starlight", "active_count", "to_spawn", "en_pos0"]

# 레벨별 WIN 기대값 (None=상관없음)
# starlight = start_light - 타워비용 + 처치보상합
# 양3×20=60 소비, 싹×n 처치보상 = n×1
WIN_EXPECT = {
    # 달1
    "1-1": {"game_state": GS_WON, "petals": 5, "starlight": 8,  "active_count": 0, "to_spawn": 0},
    "1-2": {"game_state": GS_WON, "petals": 5, "starlight": 10, "active_count": 0, "to_spawn": 0},
    "1-3": {"game_state": GS_WON, "petals": 5, "starlight": 12, "active_count": 0, "to_spawn": 0},
    "1-4": {"game_state": GS_WON, "petals": 5, "starlight": 19, "active_count": 0, "to_spawn": 0},
    "1-5": {"game_state": GS_WON, "petals": 5, "starlight": 21, "active_count": 0, "to_spawn": 0},
    # 달2 (harness 결과 기준 — 스테이지별 배치/경제에 따라 다름)
    "2-1": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-2": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-3": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-4": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-5": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-6": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-7": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-8": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-9": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-10": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "2-11": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    # 달3
    "3-1":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-2":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-3":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-4":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-5":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-6":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-7":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-8":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-9":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-10": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "3-11": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    # 달4
    "4-1":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-2":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-3":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-4":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-5":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-6":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-7":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-8":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-9":  {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-10": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
    "4-11": {"game_state": GS_WON, "petals": None, "starlight": None, "active_count": 0, "to_spawn": 0},
}
LOSS_EXPECT = {
    "1-1": {"game_state": GS_LOST, "to_spawn": 0, "starlight": 60},  # petals≤0 별도 체크
}

def resolve(scenario):
    # 구버전 호환
    if scenario == "win":
        scenario = "1-1-win"
    if scenario == "loss":
        scenario = "1-1-loss"
    # decode_record round-trip 테스트
    if scenario == "code":
        return {
            "game_state":   0,   # test1: level_idx = (1-1)*11+(1-1) = 0
            "petals":       0,   # test1: game_mode = 0 (Easy)
            "starlight":   43,   # test2: level_idx = (4-1)*11+(11-1) = 43
            "active_count": 1,   # test2: game_mode = 1 (Hard)
            "to_spawn":    -1,   # test3: 체크 오류 → -1
            "en_pos0":     -1,   # test4: 형식 오류 → -1
        }, False
    if scenario == "speed":
        # statebuf: [0]=en_pos[0](SPROUT 10틱), [1]=en_pos[1](SNAKE 10틱), [2]=petals
        # 헤더가 6 dword지만 하네스는 3 dword만 출력 — FIELDS 순서로 매핑
        return {
            "game_state":   10 * 179,   # en_pos[0]: SPROUT 10*179=1790
            "petals":       10 * 218,   # en_pos[1]: SNAKE  10*218=2180
            "starlight":    5,          # petals 누수 없음
        }, False
    if scenario == "aura":
        # statebuf: [0]=en_aura_buff[1](2타일내), [1]=en_aura_buff[2](3타일+밖), [2]=en_aura_buff[0](VAIN자신)
        return {
            "game_state":   1,          # en_aura_buff[1]=1 (범위 내)
            "petals":       0,          # en_aura_buff[2]=0 (범위 외)
            "starlight":    0,          # en_aura_buff[0]=0 (VAIN 자신 제외)
        }, False
    if scenario == "boost":
        # statebuf: [0]=tower_boosted[SHEEP_IDX], [1]=tower_boosted[LAMP_IDX]
        return {
            "game_state":   1,          # tower_boosted[SHEEP]=1 (FOX 1타일 내)
            "petals":       0,          # tower_boosted[LAMP]=0 (FOX 5타일 밖)
        }, False
    if scenario == "boss":
        # statebuf: [0]=active_count, [1]=boss_phase
        return {
            "game_state":   5,          # active_count=5 (보스+SPROUT×4)
            "petals":       1,          # boss_phase=1
        }, False
    if scenario == "slow":
        # statebuf: [0]=en_pos[0] after move_tick(슬로우), [1]=slow_timer after decay_tick
        return {
            "game_state":   64,         # en_pos[0]=128*128/256=64
            "petals":       9,          # slow_timer=10-1=9
        }, False
    if scenario == "evade":
        # statebuf: [0]=en_pos[0] SNAKE 타워 사거리 내 1틱 → 218*3/2=327
        return {
            "game_state":   327,        # en_pos[0]=327
        }, False
    if scenario == "erratic":
        # statebuf: [0]=en_pos[0] DRUNK prng_state=1→jitter=+1, speed=115+1=116
        return {
            "game_state":   116,        # en_pos[0]=116
        }, False
    if scenario == "boss-cascade":
        # statebuf: [0]=active_count, [1]=boss_phase
        return {
            "game_state":   11,         # active_count=11 (보스+4+3+3)
            "petals":       3,          # boss_phase=3
        }, False
    if scenario == "boost-cd":
        # statebuf: [0]=tower_cd[SHEEP_B], [1]=tower_cd[SHEEP_N]
        # SHEEP boosted by adjacent FOX → CD = 20*218>>8 = 17
        return {
            "game_state":   17,         # tower_cd[SHEEP_BOOSTED]=17
            "petals":       20,         # tower_cd[SHEEP_NORMAL]=20
        }, False
    if scenario == "callwave":
        # statebuf: [0]=starlight(Phase1), [1]=wave_scd(Phase1), [2]=starlight(Phase2)
        # Phase1: wave_scd=90 → 90/30=3 보너스 → starlight=13, wave_scd=0
        # Phase2: wave_scd=0  → 보너스 없음 → starlight=10
        return {
            "game_state":   13,         # Phase1 starlight=10+3=13
            "petals":       0,          # Phase1 wave_scd=0 (cleared)
            "starlight":    10,         # Phase2 starlight=10 (no bonus)
        }, False
    if scenario.endswith("-win"):
        level = scenario[:-4]
        if level not in WIN_EXPECT:
            print(f"알 수 없는 레벨: {level}"); sys.exit(2)
        return WIN_EXPECT[level], False
    if scenario.endswith("-loss"):
        level = scenario[:-5]
        if level not in LOSS_EXPECT:
            print(f"알 수 없는 레벨(loss): {level}"); sys.exit(2)
        return LOSS_EXPECT[level], True
    print(f"알 수 없는 시나리오: {scenario}"); sys.exit(2)

def main():
    if len(sys.argv) < 3:
        print("사용: python3 tools/check-state.py <fb.raw> <scenario>"); sys.exit(2)
    raw, scenario = sys.argv[1], sys.argv[2]
    expect, is_loss = resolve(scenario)

    with open(raw, "rb") as f:
        hdr = f.read(24)
    if len(hdr) < 24:
        print(f"헤더 부족: {len(hdr)}B"); sys.exit(1)
    vals = dict(zip(FIELDS, struct.unpack("<6i", hdr)))

    print(f"[check-state] 시나리오={scenario}")
    print(f"  game_state={vals['game_state']}({GS.get(vals['game_state'],'?')}) "
          f"petals={vals['petals']} starlight={vals['starlight']} "
          f"active={vals['active_count']} to_spawn={vals['to_spawn']} en_pos0={vals['en_pos0']}")

    fails = []
    for k, want in expect.items():
        if want is None: continue
        if vals[k] != want:
            fails.append(f"{k}={vals[k]} (기대 {want})")
    if is_loss and vals["petals"] > 0:
        fails.append(f"petals={vals['petals']} (기대 ≤0)")

    if fails:
        print("  ✗ 단언 실패: " + "; ".join(fails)); sys.exit(1)
    print("  ✓ 모든 상태 단언 통과"); sys.exit(0)

if __name__ == "__main__":
    main()
