#!/usr/bin/env python3
"""주어진 파일이 유효한 Windows x86(i386) PE 실행파일인지 헤더 바이트로 검증한다 (이슈 02).
사용: python3 tools/verify-pe.py <file.exe>
종료코드 0 = 유효, 1 = 불일치."""
import struct, sys

def verify(path):
    d = open(path, "rb").read()
    ok = True
    def check(label, cond, extra=""):
        nonlocal ok
        mark = "OK " if cond else "FAIL"
        print(f"  [{mark}] {label} {extra}")
        ok = ok and cond

    print(f"검증: {path} ({len(d)} bytes)")
    check("MZ 매직", d[:2] == b"MZ", repr(d[:2]))
    if len(d) < 0x40:
        print("  [FAIL] 너무 작아 PE 헤더 없음"); return False
    e = struct.unpack_from("<I", d, 0x3c)[0]
    check("e_lfanew 범위", 0 < e < len(d) - 24, hex(e))
    check("PE 시그니처", d[e:e+4] == b"PE\x00\x00", repr(d[e:e+4]))
    mach = struct.unpack_from("<H", d, e+4)[0]
    check("머신 = i386(0x14c)", mach == 0x14c, hex(mach))
    nsec = struct.unpack_from("<H", d, e+6)[0]
    check("섹션 수 >= 1", nsec >= 1, str(nsec))
    return ok

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("사용: python3 tools/verify-pe.py <file.exe>"); sys.exit(2)
    sys.exit(0 if verify(sys.argv[1]) else 1)
