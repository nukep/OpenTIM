# Read all the functions from Data31

def getU32(addr):
    return getInt(addr) % 0x100000000

def getAddress(offset):
    return currentProgram.getAddressFactory().getDefaultAddressSpace().getAddress(offset)

item_addr = 0x10f00000

func_types = {}
funcs = {}

def setf(func_addr, func_type, part_type):
    if funcs.get(func_addr) == None: funcs[func_addr] = set()
    funcs[func_addr].add(part_type)

    if func_types.get(func_addr) == None: func_types[func_addr] = set()
    func_types[func_addr].add(func_type)

for n in range(66):
    bounce_func = getU32(getAddress(item_addr + 0x22))
    run_func    = getU32(getAddress(item_addr + 0x26))
    reset_func  = getU32(getAddress(item_addr + 0x2a))
    flip_func   = getU32(getAddress(item_addr + 0x2e))
    resize_func = getU32(getAddress(item_addr + 0x32))
    rope_func   = getU32(getAddress(item_addr + 0x36))

    setf(bounce_func, 'bounce', n)
    setf(run_func,    'run', n)
    setf(reset_func,  'reset', n)
    setf(flip_func,   'flip', n)
    setf(resize_func, 'resize', n)
    setf(rope_func,   'rope', n)

    item_addr += 0x3a

from ghidra.app.cmd.disassemble import DisassembleCommand
from ghidra.program.model.symbol.SourceType import *
from ghidra.program.model.listing.Function import FunctionUpdateType
from ghidra.program.model.listing import ParameterImpl
from ghidra.program.model.data import Pointer16DataType, Pointer32DataType, UnsignedIntegerDataType, IntegerDataType, CharDataType, UnsignedLongDataType, VoidDataType

function_manager = currentProgram.getFunctionManager()

PartPtrType = Pointer16DataType(currentProgram.getDataTypeManager().getDataType('/Part'))

intdt = IntegerDataType.dataType
voiddt = VoidDataType.dataType

type_to_params = {
    'bounce': [intdt,  ['part', PartPtrType]],
    'run':    [voiddt, ['part', PartPtrType]],
    'reset':  [voiddt, ['part', PartPtrType]],
    'flip':   [voiddt, ['part', PartPtrType], ['flag', intdt]],
    'resize': [voiddt, ['part', PartPtrType]],
    'rope':   [intdt,  ['p1', PartPtrType], ['p2', PartPtrType], ['rope_slot', intdt], ['flags', intdt], ['p1_mass', intdt], ['p1_force', UnsignedLongDataType.dataType]],
}

for func_addr, part_types in funcs.items():
    func_addr_int = func_addr
    ftypes = func_types[func_addr]
    assert(len(ftypes) == 1)
    ftype = list(ftypes)[0]
    func_addr = getAddress(func_addr)

    function_name = "d31_{}_{}".format(ftype, "+".join(["{:02X}".format(x) for x in sorted(part_types)]))

    # some functions may not have been disassembled yet.
    fn = function_manager.getFunctionAt(func_addr)

    if fn == None:
        cmd = DisassembleCommand(func_addr, None, True)
        cmd.applyTo(currentProgram, monitor)
        fn = createFunction(func_addr, function_name)

    fn.setName(function_name, USER_DEFINED)

    params_r = type_to_params[ftype]
    return_type = params_r[0]

    params = []
    for param_name, param_type in params_r[1:]:
        params.append(ParameterImpl(param_name, param_type, currentProgram))

    fn.setReturnType(return_type, USER_DEFINED)
    fn.replaceParameters(params, FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS, False, USER_DEFINED)

    # print("{:08X} - {}".format(func_addr_int, function_name))

    # print("{:08X} {} {}".format(func_addr, ftype, missing), part_types)
