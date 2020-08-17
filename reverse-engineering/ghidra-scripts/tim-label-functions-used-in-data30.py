# Functions are in segment 1078 (Code16)

# manually extracted from Data30
addrs_in_seg_1078 = [
    '023d',     # 0x00
    '0280',
    '02cc',
    '0323',
    '036f',
    '03c2',
    '0426',
    '0480',
    '04d2',     # 0x08
    '0511',
    '0554',
    '0592',
    '05e5',
    '0632',
    '068c',
    '06e0',
    '0728',     # 0x10
    '0770',
    '07c3',
    '0814',
    '0865',
    '08a8',
    '08f9',
    '0946',
    '0974',     # 0x18
    '09c5',
    '0a0d',
    '0a59',
    '0aa6',
    '0ae9',
    '0b18',
    '0b42',
    '0b8f',     # 0x20
    '0bd2',
    '0c25',
    '0c68',
    '0cb0',
    '0cfc',
    '0d49',
    '0d6e',
    '0db6',     # 0x28
    '0000',     # NULL!!!!!
    '0e10',
    '0e5d',
    '0ea0',
    '0ee3',
    '0280',
    '0f33',
    '0280',     # 0x30
    '0f7b',
    '0fa3',
    '0ff3',
    '1044',
    '1087',
    '10ca',
    '1112',
    '1170',     # 0x38
    '11b3',
    '11fe',
    '121f',
    '0280',
    '1262',
    '12af',
    '0e5d',
    '12fc',     # 0x40
    '133f'
]

addrs_in_seg_1078 = [int(x, 16) for x in addrs_in_seg_1078]

# multiple entries share the same function.
# populate an offset to indexes lookup.
off_to_idxs = {}
for i,off in enumerate(addrs_in_seg_1078):
    if off == 0: continue
    
    d = off_to_idxs.get(off)
    if d == None:
        d = set()
        off_to_idxs[off] = d
    d.add(i)


from ghidra.program.model.symbol.SourceType import *
from ghidra.program.model.listing.Function import FunctionUpdateType
from ghidra.program.model.listing import ParameterImpl
from ghidra.program.model.data import Pointer16DataType, Pointer32DataType, UnsignedIntegerDataType, IntegerDataType, CharDataType, UnsignedLongDataType, VoidDataType

function_manager = currentProgram.getFunctionManager()

PartPtrType = Pointer16DataType(currentProgram.getDataTypeManager().getDataType('/Part'))

def getAddress(offset):
    return currentProgram.getAddressFactory().getDefaultAddressSpace().getAddress(offset)

for off in off_to_idxs:
    fn = function_manager.getFunctionAt(getAddress("1078:{:04X}".format(off)))

    suffix = "+".join(["{:02X}".format(i) for i in sorted(off_to_idxs[off])])
    fn_name = "data30init_{}".format(suffix)

    fn.setName(fn_name, USER_DEFINED)

    fn.setReturnType(IntegerDataType.dataType, USER_DEFINED)

    params = [ParameterImpl("part", PartPtrType, currentProgram)]
    fn.replaceParameters(params, FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS, False, USER_DEFINED)

    # print(off, fn)
