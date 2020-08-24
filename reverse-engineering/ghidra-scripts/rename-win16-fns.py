import sys

from win16fns.gdi import gdi_fns
from win16fns.kernel import kernel_fns
from win16fns.user import user_fns

from ghidra.program.model.symbol.SourceType import *
from ghidra.program.model.listing.Function import FunctionUpdateType
from ghidra.program.model.listing import ParameterImpl
from ghidra.program.model.data import Pointer16DataType, Pointer32DataType, UnsignedIntegerDataType, IntegerDataType, CharDataType, UnsignedLongDataType, VoidDataType

arg_to_datatype_lookup = {
    'str': Pointer32DataType(CharDataType.dataType),
    'segstr': Pointer32DataType(CharDataType.dataType),
    'ptr': Pointer32DataType(VoidDataType.dataType),
    'segptr': Pointer32DataType(VoidDataType.dataType),
    'word': UnsignedIntegerDataType.dataType,
    's_word': IntegerDataType.dataType,
    'long': UnsignedLongDataType.dataType
}

fns_lookup = {
    'USER': user_fns,
    'KERNEL': kernel_fns,
    'GDI': gdi_fns
}

function_manager = currentProgram.getFunctionManager()

for f in function_manager.getFunctions(True):
    tf = f.getThunkedFunction(False)
    if tf is not None:
        thunkname = tf.getName()        # Ordinal_#    e.g. Ordinal_123, Ordinal_5
        libname = tf.getExternalLocation().getLibraryName()

        fns = fns_lookup.get(libname)

        if fns:
            # find matching ord
            match = None
            for x in fns:
                if "Ordinal_{}".format(x['ord']) == thunkname:
                    match = x
                    break
            
            if match:
                # only support pascal calling convention for now
                # TODO - add varargs
                if x['convention'] == 'pascal':
                    params = []

                    i = 1

                    # iterate in reverse order and pretend it's stdcall, because Ghidra doesn't natively support the pascal calling convention
                    for arg in x['args'][::-1]:
                        datatype = arg_to_datatype_lookup.get(arg)
                        if datatype is None:
                            print("WARNING - datatype is unknown for {}".format(arg))
                        
                        param_name = "param{}".format(i)
                        params.append(ParameterImpl(param_name, datatype, currentProgram))

                        i += 1

                    # Prefixing by the library name makes it easier to organize functions in Ghidra
                    f.setName("{}.{}".format(libname, match['fn']), USER_DEFINED)
                    f.setCallingConvention('__stdcall16far')
                    if x.get('ret') == 'void':
                        f.setReturnType(VoidDataType.dataType, USER_DEFINED)
                    else:
                        f.setReturnType(IntegerDataType.dataType, USER_DEFINED)
                    f.replaceParameters(params, FunctionUpdateType.DYNAMIC_STORAGE_ALL_PARAMS, False, USER_DEFINED)
