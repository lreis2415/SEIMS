"""Read and write of raster data

    @author   : Liangjun Zhu

    @changelog:
    - 22-06-07 - lj - Initial wrapper of mask_rasterio.
"""
from __future__ import absolute_import, unicode_literals
from six import string_types

from io import open
from pygeoc.utils import UtilClass, FileClass, is_string


def mask_rasterio(bin_dir, inoutcfg,
                  mongoargs=None, maskfile=None, cfgfile=None,
                  include_nodata=True, mode='MASK'):
    """Call mask_rasterio program (cpp version) to perform input/output of raster

    TODO: this function is very preliminary, need to be improved and tested!
    """
    commands = ['"%s/mask_rasterio"' % bin_dir]
    if mode.upper() not in ['MASK', 'DEC', 'COM', 'MASKDEC']:
        mode = 'MASK'
    commands += ['-mode', mode]
    usemongo = False
    if mongoargs is not None and type(mongoargs) is list and len(mongoargs) >= 4:
        usemongo = True
        commands += ['-mongo']
        commands += [i if is_string(i) else repr(i) for i in mongoargs]
    if maskfile is not None:
        if FileClass.is_file_exists(maskfile):
            commands += ['-mask', 'SFILE', maskfile]
        elif usemongo:
            commands += ['-mask', 'GFS', maskfile]
    commands += ['-include_nodata', '1' if include_nodata else '0']

    parsed_inout = list()
    for inout in inoutcfg:
        item_count = len(inout)
        inidx = 0
        outidx = 1 if item_count >= 2 else -1
        dvidx = 2 if item_count >= 3 else -1
        nodataidx = 3 if item_count >= 4 else -1
        outtypeidx = 4 if item_count >= 5 else -1
        reclsidx = 5 if item_count >= 6 else -1

        cur_dict = dict()
        if type(inout[inidx]) is list:
            cur_dict['-in'] = ','.join(inout[inidx])
        elif isinstance(inout[inidx], string_types) and inout[inidx] != '':
            cur_dict['-in'] = inout[inidx]
        if outidx > 0 and inout[outidx] != '':
            cur_dict['-out'] = inout[outidx]
        if dvidx > 0:
            cur_dict['-default'] = repr(inout[dvidx])
        if nodataidx > 0:
            cur_dict['-nodata'] = repr(inout[nodataidx])
        if outtypeidx > 0:
            cur_dict['-outdatatype'] = inout[outtypeidx]
        if reclsidx > 0:
            cur_dict['-reclass'] = inout[reclsidx]
        parsed_inout.append(cur_dict)
    if cfgfile is not None:
        with open(cfgfile, 'w', encoding='utf-8') as f:
            for dic in parsed_inout:
                f.write('%s;%s;%s;%s;%s;%s\n' % (dic['-in'],
                                                 dic['-out'] if '-out' in dic else '',
                                                 dic['-default'] if '-default' in dic else '',
                                                 dic['-nodata'] if '-nodata' in dic else '',
                                                 dic['-outdatatype'] if '-outdatatype' in dic else '',
                                                 dic['-reclass'] if '-reclass' in dic else ''))
        commands += ['-configfile', cfgfile]
        UtilClass.run_command(commands)
    else:
        for curargs in parsed_inout:
            curcommands = commands[:]
            for ck, cv in curargs.items():
                curcommands.append(ck)
                curcommands.append(cv)
            UtilClass.run_command(curcommands)
