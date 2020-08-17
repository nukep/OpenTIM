import sys
from tim_patch_bytes import do_replacement_all

# Ghidra doesn't know how to represent abs(). It instead shows something like: (a ^ b) - b

# Original instructions:
# CWD
# XOR AX, DX
# SUB AX, DX

# New instructions:
# CALLF 2000:0000
do_replacement_all(sys.modules[__name__], currentAddress, b"\x99\x33\xc2\x2b\xc2", b"\x9a\x00\x00\x00\x20")
