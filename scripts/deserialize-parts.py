## NOTE: Not meant to be used with Ghidra

## Read a DOSBoxX memory dump and output all the Parts (and Ropes/Belts) as JSON.
## The JSON is used by the interactive HTML debugger tools.

import json
import tim_structures

def deserialize_binary(filename):
    with open(filename, 'rb') as f:
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

    def readulong(addr):
        return content[addr] + (content[addr+1] << 8) + (content[addr+2] << 16) + (content[addr+3] << 24)

    def readslong(addr):
        v = readulong(addr)
        if v >= 0x80000000:
            return v-0x100000000
        else:
            return v


    STATIC_PARTS_ROOT = 0x3faf
    MOVING_PARTS_ROOT = 0x3f0d
    PARTS_BIN_ROOT = 0x3e6b

    def read_bytevec(addr):
        x = readbyte(addr)
        y = readbyte(addr+1)
        return [x, y]

    def read_shortvec(addr):
        x = readsint(addr)
        y = readsint(addr+2)
        return [x, y]


    from collections import namedtuple

    FieldType = namedtuple('FieldType', ['read', 'size', 'repr'])
    FieldType.__new__.__defaults__ = (None,) # make repr optional

    def repr_pointer(v):
        return "{:04X}".format(v)

    def repr_flags16(v):
        return "{:04X} ({:04b} {:04b} {:04b} {:04b})".format(v, (v >> 12) & 0xF, (v >> 8) & 0xF, (v >> 4) & 0xF, (v >> 0) & 0xF)

    POINTER = FieldType(readuint, 2, repr_pointer)

    field_types = {
        'Part': POINTER,
        'PartType': FieldType(readuint, 2, tim_structures.part_id_to_name),
        'Flag': FieldType(readuint, 2, repr_flags16),
        'ByteVec': FieldType(read_bytevec, 2),
        'ShortVec': FieldType(read_shortvec, 4),
        'BeltData': POINTER,
        'RopeData': POINTER,
        'BorderPoints': POINTER,

        'byte': FieldType(readbyte, 1),
        'uint': FieldType(readuint, 2),
        'int': FieldType(readsint, 2),
        'long': FieldType(readslong, 4),
    }


    def deserialize_fields(fields, start_addr):
        obj = {}
        for field in fields:
            addr = start_addr + field.offset
            ft = field_types[field.type]

            repr_func = ft.repr
            if repr_func == None:
                repr_func = lambda x: x
            
            if field.count == None:
                value = ft.read(addr)
                repr_value = repr_func(value)
                obj[field.name] = {
                    'type': field.type,
                    'value': value,
                    'repr_value': repr_value
                }
            else:
                value = []
                for n in range(field.count):
                    value = ft.read(addr)
                    repr_value = repr_func(value)
                    addr += ft.size
                    obj["{}_{}".format(field.name, n+1)] = {
                        'type': field.type,
                        'value': value,
                        'repr_value': repr_value
                    }
        return obj

    def augment_border_repr_to_part(part):
        borders = []

        num_borders = part['num_borders']['value']
        borders_data_ptr = part['borders_data']['value']

        if borders_data_ptr != 0:
            ptr = borders_data_ptr
            for n in range(num_borders):
                x = readbyte(ptr+0)
                y = readbyte(ptr+1)
                normal_angle = readuint(ptr+2)/65536
                borders.append([x, y, normal_angle])
                ptr += 4
        
        part['borders_data']['repr_value'] = borders


    parts = {}

    parts_to_read = set()

    static_parts_in_order = []
    moving_parts_in_order = []

    part_addr = readuint(STATIC_PARTS_ROOT)
    while part_addr != 0:
        parts_to_read.add(part_addr)
        static_parts_in_order.append(part_addr)
        part_addr = readuint(part_addr)

    part_addr = readuint(MOVING_PARTS_ROOT)
    while part_addr != 0:
        parts_to_read.add(part_addr)
        moving_parts_in_order.append(part_addr)
        part_addr = readuint(part_addr)

    belts_to_read = set()
    ropes_to_read = set()

    for part_addr in parts_to_read:
        part = deserialize_fields(tim_structures.read_structure('Part'), part_addr)
        augment_border_repr_to_part(part)

        for v in part.values():
            if v['type'] == 'BeltData':
                belts_to_read.add(v['value'])
            if v['type'] == 'RopeData':
                ropes_to_read.add(v['value'])
        
        parts["{:04X}".format(part_addr)] = part

    belts_to_read.remove(0)
    ropes_to_read.remove(0)

    belts = {}

    for belt_addr in belts_to_read:
        belt = deserialize_fields(tim_structures.read_structure('BeltData'), belt_addr)
        belts["{:04X}".format(belt_addr)] = belt

    ropes = {}

    for rope_addr in ropes_to_read:
        rope = deserialize_fields(tim_structures.read_structure('RopeData'), rope_addr)
        ropes["{:04X}".format(rope_addr)] = rope

    out_json = {
        'static_parts_in_order': static_parts_in_order,
        'moving_parts_in_order': moving_parts_in_order,
        'parts': parts,
        'belts': belts,
        'ropes': ropes
    }
    return out_json

out_json = []

import sys

filenames = sys.argv[1:]
for filename in filenames:
    print(filename)
    out_json.append(deserialize_binary(filename))

with open('parts.json', 'w') as f:
    json.dump(out_json, f, indent=2)
