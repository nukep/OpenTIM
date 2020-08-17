import sys
from tim_patch_bytes import do_replacement_all

# Original instructions:
# CALLF 1040:141a
# SHL EAX,0x10
# SHRD EAX,EDX,0x10

# New instructions:
# CALLF 1040:141a
# NOP (x 9 times)
do_replacement_all(sys.modules[__name__], currentAddress, b"\x9a\x1a\x14\x40\x10\x66\xc1\xe0\x10\x66\x0f\xac\xd0\x10", b"\x9a\x1a\x14\x40\x10\x90\x90\x90\x90\x90\x90\x90\x90\x90")