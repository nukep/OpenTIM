from tim_structures import read_structure
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--js', action='store_true')
parser.add_argument('--rs', action='store_true')
args = parser.parse_args()

# (WASM-32 size, WASM-32 align, C type, Rust type)
type_to_c = {
    'Part':         (4, 4, 'struct Part *{}',        '*mut Part'),
    'PartType':     (2, 2, 'u16 {}',                 'u16'),
    'Flag':         (2, 2, 'u16 {}',                 'u16'),
    'int':          (2, 2, 's16 {}',                 'i16'),
    'uint':         (2, 2, 'u16 {}',                 'u16'),
    'byte':         (1, 1, 'byte {}',                'u8'),
    'ByteVec':      (2, 1, 'struct ByteVec {}',      'ByteVec'),
    'ShortVec':     (4, 2, 'struct ShortVec {}',     'ShortVec'),
    'long':         (4, 4, 's32 {}',                 'i32'),
    'RopeData':     (4, 4, 'struct RopeData *{}',    '*mut RopeData'),
    'BeltData':     (4, 4, 'struct BeltData *{}',    '*mut BeltData'),
    'BorderPoints': (4, 4, 'struct BorderPoint *{}', '*mut BorderPoint'),
}

def generate(type_name):
    print('struct {} {{'.format(type_name))
    for field in read_structure(type_name):
        c_size, _c_align, c_field, rust_field = type_to_c[field.type]
        c_field = c_field.format(field.name)
        if field.count != None:
            c_field += '[{}]'.format(field.count)
        
        c_comment = 'TIMWIN offset: {:02X}'.format(field.offset)
        c_line = '    {:<40} // {}'.format(c_field + ';', c_comment)
        
        print(c_line)
    print('};')

def generate_js(type_name):
    print('const {}_fields = ['.format(type_name))
    offset = 0
    for field in read_structure(type_name):
        c_size, c_align, c_field, rust_field = type_to_c[field.type]
        if offset % c_align != 0:
            # align the field
            offset = offset - (offset % c_align) + c_align
        
        print('  [0x{:02X}, \"{}\", \"{}\"],'.format(offset, field.type, field.name))

        count = 1 if field.count == None else field.count

        offset += c_size * count
    print(']')
    print('const {}_size = 0x{:02X}'.format(type_name, offset))

def generate_rs(type_name):
    print('#[repr(C)]')
    print('#[derive(Debug)]')
    print('pub struct {} {{'.format(type_name))
    for field in read_structure(type_name):
        c_size, _c_align, c_field, rust_type = type_to_c[field.type]

        field_name = field.name

        # Workaround. "type" is a keyword in Rust. Don't feel like updating all the C code, so it's Rust-specific.
        if type_name == 'Part' and field_name == 'type':
            field_name = 'part_type'

        if field.count == None:
            rust_field = 'pub {}: {}'.format(field_name, rust_type)
        else:
            rust_field = 'pub {}: [{}; {}]'.format(field_name, rust_type, field.count)
        
        rust_comment = 'TIMWIN offset: {:02X}'.format(field.offset)
        rust_line = '    {:<40} // {}'.format(rust_field + ',', rust_comment)
        
        print(rust_line)
    print('}')

gen_f = generate
if args.js:
    gen_f = generate_js
if args.rs:
    gen_f = generate_rs

gen_f('Part')
gen_f('RopeData')
gen_f('BeltData')
