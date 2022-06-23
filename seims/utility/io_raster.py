"""Read and write of raster data

    @author   : Liangjun Zhu

    @changelog:
    - 22-06-07 - lj - Initial wrapper of mask_rasterio.
"""
from __future__ import absolute_import, unicode_literals

from io import open
from pygeoc.utils import UtilClass, FileClass, is_string


def mask_rasterio(bin_dir, inoutcfg,
                  mongoargs=None, maskfile=None, cfgfile=None,
                  include_nodata=True, mode='MASK'):
    """Call mask_rasterio program (cpp version) to perform input/output of raster

    TODO: this function is very preliminary, need to be improved and tested!
    """
    # concatenate command line
    commands = ['"%s/mask_rasterio"' % bin_dir]
    if mode.upper() not in ['MASK', 'DEC', 'COM', 'MASKDEC']:
        mode = 'MASK'
    commands.append('-mode %s' % mode)
    usemongo = False
    if mongoargs is not None and type(mongoargs) is list and len(mongoargs) >= 4:
        usemongo = True
        commands.append('-mongo %s' % ' '.join(i if is_string(i) else repr(i) for i in mongoargs))
    if maskfile is not None:
        if FileClass.is_file_exists(maskfile):
            commands.append('-mask SFILE %s' % maskfile)
        elif usemongo:
            commands.append('-mask GFS %s' % maskfile)
    commands.append('-include_nodata 1' if include_nodata else '-include_nodata 0')
    parsed_inout = list()
    for inout in inoutcfg:
        item_count = len(inout)
        inidx = 0
        outidx = 1 if item_count >= 2 else -1
        dvidx = 2 if item_count >= 3 else -1
        nodataidx = 3 if item_count >= 4 else -1
        outtypeidx = 4 if item_count >= 5 else -1
        reclsidx = 5 if item_count >= 6 else -1

        curstr = ''
        instr = ''
        if type(inout[inidx]) is list:
            instr = ','.join(inout[inidx])
        elif type(inout[inidx]) is str and inout[inidx] != '':
            instr = inout[inidx]
        if cfgfile is None:
            if instr != '':
                curstr += '-in %s ' % instr
            if outidx > 0 and inout[outidx] != '':
                curstr += '-out %s ' % inout[outidx]
            if dvidx > 0:
                curstr += '-default %s ' % repr(inout[dvidx])
            if nodataidx > 0:
                curstr += '-nodata %s ' % repr(inout[nodataidx])
            if outtypeidx > 0:
                curstr += '-outdatatype %s ' % inout[outtypeidx]
            if reclsidx > 0:
                curstr += '-reclass %s ' % inout[reclsidx]
        else:
            curstr += '%s;' % instr
            curstr += '%s;' % (inout[outidx] if outidx > 0 else '')
            curstr += '%s;' % (repr(inout[dvidx]) if dvidx > 0 else '')
            curstr += '%s;' % (repr(inout[nodataidx]) if nodataidx > 0 else '')
            curstr += '%s;' % (inout[outtypeidx] if outtypeidx > 0 else '')
            curstr += '%s;' % (inout[reclsidx] if reclsidx > 0 else '')
        parsed_inout.append(curstr)
    if cfgfile is not None:
        with open(cfgfile, 'w', encoding='utf-8') as f:
            f.write('\n'.join(parsed_inout))
        commands.append('-configfile %s' % cfgfile)
        UtilClass.run_command(commands)
    else:
        for curargs in parsed_inout:
            curcommands = commands[:]
            curcommands.append(curargs)
            UtilClass.run_command(curcommands)
