from tim_structures import read_structure
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--js', action='store_true')
args = parser.parse_args()

# (WASM-32 size, WASM-32 align, C type)
type_to_c = {
    'Part':         (4, 4, 'struct Part *{}'),
    'PartType':     (2, 2, 'uint {}'),
    'Flag':         (2, 2, 'uint {}'),
    'int':          (2, 2, 'sint {}'),
    'uint':         (2, 2, 'uint {}'),
    'byte':         (1, 1, 'byte {}'),
    'ByteVec':      (2, 1, 'struct ByteVec {}'),
    'ShortVec':     (4, 2, 'struct ShortVec {}'),
    'long':         (4, 4, 'slong {}'),
    'RopeData':     (4, 4, 'struct RopeData *{}'),
    'BeltData':     (4, 4, 'struct BeltData *{}'),
    'BorderPoints': (4, 4, 'struct BorderPoint *{}'),
}

def generate(type_name):
    print('struct {} {{'.format(type_name))
    for field in read_structure(type_name):
        c_size, _c_align, c_field = type_to_c[field.type]
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
        c_size, c_align, c_field = type_to_c[field.type]
        if offset % c_align != 0:
            # align the field
            offset = offset - (offset % c_align) + c_align
        
        print('  [0x{:02X}, \"{}\", \"{}\"],'.format(offset, field.type, field.name))

        count = 1 if field.count == None else field.count

        offset += c_size * count
    print(']')
    print('const {}_size = 0x{:02X}'.format(type_name, offset))

gen_f = generate
if args.js:
    gen_f = generate_js

gen_f('Part')
gen_f('RopeData')
gen_f('BeltData')
