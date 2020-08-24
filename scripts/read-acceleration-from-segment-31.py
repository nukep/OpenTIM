# Used to generate the unit-test fixtures for calculating acceleration and terminal velocity
#
# 1. Adjust gravity and air pressure in the game's control panel. The table in segment 31 will automatically update.
# 2. Put DOSBoxX into debug mode and dump segment 31. Run the command:
#        memdumpbin XXXX:0000 0EF4
#    Note: XXXX is the selector for segment 31 in hexadecimal, which you must find out.
# 3. Run this script.

import sys
memdumpbin_filename = sys.argv[1]

with open(memdumpbin_filename, 'rb') as f:
    content = f.read()

def readbyte(addr):
    return content[addr]

def readuint(addr):
    return content[addr] + (content[addr+1] << 8)

def readsint(addr):
    v = readuint(addr)
    if v >= 0x8000:
        return v-0x10000
    else:
        return v

global_terminal_vel = readsint(0x0A)
print("Terminal velocity: {}".format(global_terminal_vel))

lookup = {}

for n in range(66):
    offset = n * 0x3A
    density      = readuint(offset + 0x00)
    acceleration = readsint(offset + 0x08)
    terminal_vel = readsint(offset + 0x0A)

    if not (n == 20 or n == 63):
        old_val = lookup.get(density)
        if old_val != None:
            if acceleration != old_val:
                print("ERROR - different acceleration at {}. density={}, {} vs {}".format(n, density, old_val, acceleration))
        
        lookup[density] = acceleration

    if not (n == 20 or n == 43):
        if terminal_vel != global_terminal_vel:
            print("ERROR - non-matching terminal velocity at {}".format(n))

for density in sorted(lookup.keys()):
    print("{{ {:5}, {:5} }}, ".format(density, lookup[density]))

print("Count: {}".format(len(lookup)))
