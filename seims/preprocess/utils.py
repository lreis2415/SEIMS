"""Utility functions

    @author   : Liangjun Zhu

    @changelog:
    - 09-08-2025  - lj - initial implementation
"""
from __future__ import absolute_import, unicode_literals

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from struct import pack, unpack, calcsize
from pygeoc.utils import StringClass


def dump_values(value_list, dtype='d', endian='<'):
    # ENDIAN = '<'  # or '>', '!', select one and use it consistently
    # DTYPE = 'd'  # 'i' = int32, 'f' = float32, 'd' = float64
    fmt = f'{endian}{len(value_list)}{dtype}'
    return pack(fmt, *value_list)


def StringToPackDType(datatype):
    if StringClass.string_match(datatype, 'FLOAT') or StringClass.string_match(datatype,
                                                                               'FLOAT32'):
        return 'f'
    elif StringClass.string_match(datatype, 'DOUBLE') or StringClass.string_match(datatype,
                                                                                  'FLOAT64'):
        return 'd'
    elif StringClass.string_match(datatype, 'INT') or StringClass.string_match(datatype,
                                                                               'INT32'):
        return 'i'
    else:
        return ''


def load_values(buf, size=-1, dtype='d', endian='<'):
    item_size = calcsize(f'{endian}{dtype}')
    n = len(buf) // item_size
    if size > 0 and n != size:
        print("Size unmatched when unpacking values from buffer!")
        return None
    fmt = f'{endian}{n}{dtype}'
    return list(unpack(fmt, buf))
