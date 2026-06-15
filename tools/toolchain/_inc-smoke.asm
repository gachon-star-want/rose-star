; INCLUDE 해석 스모크: win32ax.inc(매크로/import) 체인이 컨테이너에서 풀리는지 검증.
; invoke / import 매크로가 동작하면 PE에 import table까지 생긴다. 동작 게임 아님.
include 'win32ax.inc'

.code
start:
        invoke ExitProcess, 0
.end start
