#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""tools/gen-story.py — 서사 장면 한글 텍스트 → src/story_data.inc (UTF-16LE FASM 데이터).

docs/09 서사 문안(D068/D071)을 화면 폭(256px, 12px/글리프 → 한 줄 ≤ ~20글리프)에 맞춰
짧은 줄로 나눈 것이 STORY_SCENES다. font-subset.py가 이 음절을 폰트에 포함시킨다(단일 소스).

장면: 0=도입 1=막간1 2=막간2 3=막간3 4=엔딩(승리) 5=엔딩(패배)
사용: python3 tools/gen-story.py [--output src/story_data.inc]
"""
import sys

# 각 장면 = 화면에 한 줄씩 표시할 한글 문자열 리스트 (절제·서정, docs/09 충실)
STORY_SCENES = [
    # 0 — 도입 (첫 시작)
    [
        "내겐 별이 하나 있다",
        "내가 길들인 장미 한 송이",
        "별엔 바오밥 씨가 돋는다",
        "두면 뿌리가 별을 쪼갠다",
        "떠날 수도 있었지만",
        "나는 장미 곁에 남았다",
    ],
    # 1 — 막간 1 (달1 끝)
    [
        "한 달이 저물었다",
        "싹들은 새싹이 되었다",
        "멀리 다른 별의 그림자가",
        "어른거린다",
    ],
    # 2 — 막간 2 (달2 끝)
    [
        "황금빛 뱀이 모래 위를 지난다",
        "헛됨도 셈도 취함도",
        "어른들의 것이 내 별까지 왔다",
        "나는 장미를 본다",
        "아직 피어 있다",
    ],
    # 3 — 막간 3 (달3 끝)
    [
        "가장 오래된 바오밥이",
        "하늘을 가린다",
        "길들인 것은 끝까지",
        "책임지는 거라고",
        "마지막 밤이다",
    ],
    # 4 — 엔딩 (4-11 클리어)
    [
        "거목이 쓰러졌다",
        "별은 조용하다",
        "장미가 웃는다 나도 웃는다",
        "이제 밤하늘을 보면",
        "어느 별에서 장미가 피어 있다",
        "모든 별이 웃는 것처럼",
    ],
    # 5 — 엔딩 (패배)
    [
        "마지막 꽃잎이 졌다",
        "별이 어두워졌다",
        "다시 해볼까",
    ],
]


def emit(out):
    lines = [
        "; story_data.inc — 서사 장면 문자열 (UTF-16LE). 자동 생성: tools/gen-story.py",
        "; scene table: scene_id → (lines_ptr dd, line_count dd). 각 줄: [len byte][dw×len]",
        "",
        "STORY_SCENE_COUNT = %d" % len(STORY_SCENES),
        "",
        "story_scene_table:",
    ]
    for i, scene in enumerate(STORY_SCENES):
        lines.append("    dd story_s%d, %d" % (i, len(scene)))
    lines.append("")
    for i, scene in enumerate(STORY_SCENES):
        lines.append("story_s%d:" % i)
        for ln in scene:
            cps = [ord(c) for c in ln]
            lines.append("    db %d" % len(cps))
            lines.append("    dw " + ", ".join("0x%04X" % c for c in cps) + "   ; %s" % ln)
        lines.append("")
    with open(out, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    n = sum(len(s) for s in STORY_SCENES)
    print("[gen-story] %s 생성 (%d 장면, %d 줄)" % (out, len(STORY_SCENES), n))


if __name__ == "__main__":
    out = "src/story_data.inc"
    if "--output" in sys.argv:
        out = sys.argv[sys.argv.index("--output") + 1]
    emit(out)
