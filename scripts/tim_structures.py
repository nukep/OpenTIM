## NOTE: This should be compatible with Ghidra (Jython 2.6+) as well as Python 3
## Imported as a library

import csv
import io
import os
from collections import namedtuple

CSV_DIRECTORY = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', 'structures'))

def make_filename(filename):
    return os.path.join(CSV_DIRECTORY, filename)

def read_part_type():
    o = {}
    with io.open(make_filename('PartType.csv'), 'r', encoding='utf-8-sig') as f:
        r = csv.DictReader(f)
        for x in r:
            id = int(x['id'], 16)
            name = x['name']
            o[id] = name
    return o

PART_LOOKUP = None
def part_id_to_name(id):
    # Lazy initialization
    global PART_LOOKUP
    if PART_LOOKUP == None: PART_LOOKUP = read_part_type()
    
    return PART_LOOKUP[id]

Field = namedtuple('Field', ['offset', 'type', 'count', 'name'])

STRUCTURES = {}
def read_structure(structure_name):
    # Lazy initialization
    global STRUCTURES
    if STRUCTURES.get(structure_name) == None:
        filename = "{}.csv".format(structure_name)
        o = []
        with io.open(make_filename(filename), 'r', encoding='utf-8-sig') as f:
            r = csv.DictReader(f)
            for x in r:
                type = x['type']
                # No type means we don't know what it is
                if type == '': continue
                
                offset = int(x['offset'], 16)
                count = x['count']
                if count == '':
                    count = None # as opposed to 1, because that'd be an array of 1 item
                else:
                    count = int(count)
                name = x['name']
                if name == '':
                    name = 'field_0x{:02X}'.format(offset)

                o.append(Field(offset, type, count, name))
        STRUCTURES[structure_name] = o
    
    return STRUCTURES[structure_name]

