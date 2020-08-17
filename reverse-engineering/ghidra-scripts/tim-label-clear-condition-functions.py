
def getU32(addr):
    return getInt(addr) % 0x100000000

def getAddress(offset):
    return currentProgram.getAddressFactory().getDefaultAddressSpace().getAddress(offset)

f_addr = 0x11082194

func_to_levelnum = {}

for n in range(1,160+1):
    addr = getU32(getAddress(f_addr))
    # print("{:04X}".format(addr))

    if func_to_levelnum.get(addr) == None: func_to_levelnum[addr] = []
    func_to_levelnum[addr].append(n)

    f_addr += 4

from ghidra.app.cmd.disassemble import DisassembleCommand
from ghidra.program.model.symbol.SourceType import *
from ghidra.program.model.listing.Function import FunctionUpdateType
# from ghidra.program.model.listing import ParameterImpl
from ghidra.program.model.data import Pointer16DataType, Pointer32DataType, UnsignedIntegerDataType, IntegerDataType, CharDataType, UnsignedLongDataType, VoidDataType

function_manager = currentProgram.getFunctionManager()

for func_addr,level_nums in func_to_levelnum.items():
    func_addr_int = func_addr
    func_addr = getAddress(func_addr)

    function_name = "clearcond_level_{}".format("+".join(["{:03}".format(x) for x in sorted(level_nums)]))

    # some functions may not have been disassembled yet.
    fn = function_manager.getFunctionAt(func_addr)

    if fn == None:
        print("Not defined: {:08X}".format(func_addr_int))
        cmd = DisassembleCommand(func_addr, None, True)
        cmd.applyTo(currentProgram, monitor)
        fn = createFunction(func_addr, function_name)
    
    fn.setName(function_name, USER_DEFINED)
    fn.setReturnType(VoidDataType.dataType, USER_DEFINED)
    fn.replaceParameters([], FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS, False, USER_DEFINED)
    fn.setCallingConvention('__cdecl16far')
