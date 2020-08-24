# Utility functions

from ghidra.program.model.address import AddressSet
from ghidra.app.cmd.disassemble import DisassembleCommand

def is_instructions(g, start, end):
    not_inst = 0
    is_inst = 0
    for x in g.currentProgram.getListing().getCodeUnits(AddressSet(start, end), True):
        if g.getInstructionContaining(x.getAddress()) == None:
            not_inst += 1
        else:
            is_inst += 1
    
    if not_inst == 0 and is_inst == 0:
        return 'empty'
    elif not_inst == 0:
        return 'all'
    elif is_inst == 0:
        return 'none'
    else:
        return 'mixed'

# returns next address, or None if there's nothing else
def do_replacement(g, addr, find, replace):
    if len(find) != len(replace):
        raise Exception("Buffer lengths don't match")
    
    buflen = len(find)
    
    addr = g.currentProgram.getMemory().findBytes(addr, find, None, True, g.monitor)
    # returns None if there are no more matches
    if addr == None:
        return None

    afterA = addr.add(buflen)
    
    # the ranges are inclusive, apparently
    endA = addr.add(buflen-1)

    st = is_instructions(g, addr, endA)

    # skip if it's not either all instructions or no instructions
    if st == 'empty': return afterA
    if st == 'mixed': return afterA

    print("Patching at", addr, st)
    
    if st == 'all':
        g.clearListing(addr, endA)

    g.setBytes(addr, replace)

    # disassemble if the original memory was previously disassembled
    if st == 'all':
        cmd = DisassembleCommand(addr, AddressSet(addr, endA), True)
        cmd.applyTo(g.currentProgram, g.monitor)
        
    return afterA

def do_replacement_all(g, addr, find, replace):
    while True:
        v = do_replacement(g, addr, find, replace)
        if v == None:
            print("Done!")
            break
        addr = v