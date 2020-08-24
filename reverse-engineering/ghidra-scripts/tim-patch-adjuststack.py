# Patch The "adjuststack" calls located at the beginning of many functions.

# The call usually looks something like:
# MOV AX, 0x6
# CALLF 1000:228f
# It turns out that the compiler generated a call at the beginning of many functions, to reserve stack space for local variables. This function also checks for stack overflows, which is probably why it was a function in the first place.
# However, for static analysis, we don't care about stack overflow checks.
# Ghidra doesn't understand what this function does, which means that operations that manipulate the stack (e.g. function calls) will corrupt local variables in the decompiler.

import sys
from tim_patch_bytes import do_replacement_all

# Original instructions:
# CALLF 1000:228f

# New instructions:
# SUB SP, AX
# SUB SP, 2

# I decided to also add "SUB SP, 2" to add some extra space, because (I think) the decompiler was still mangling some of the variables.
do_replacement_all(sys.modules[__name__], currentAddress, b"\x9a\x8f\x22\x00\x10", b"\x29\xC4\x83\xEC\x02")