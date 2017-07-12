#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Global variables and utility functions
    @author   : Liangjun Zhu, Junzhi Liu
    @changelog: 16-06-16  first implementation version
                17-06-22  reformat according to pylint and google style
"""
from seims.pygeoc.pygeoc.utils.utils import StringClass
# Global variables
UTIL_ZERO = 1.e-6
MINI_SLOPE = 0.0001
DEFAULT_NODATA = -9999.
SQ2 = 1.4142135623730951
PI = 3.141592653589793
LFs = ['\r\n', '\n\r', '\r', '\n']


def status_output(status_msg, percent, file_object):
    """Print status and flush to file.
    Args:
        status_msg: status message
        percent: percentage rate of progress
        file_object: file handler
    """
    print ("[Output] %d..., %s" % (percent, status_msg))
    file_object.write("%d, %s\n" % (percent, status_msg))
    file_object.flush()


def read_data_items_from_txt(txt_file):
    """Read data items include title from text file, each data element are split by TAB or COMMA.
       Be aware, the separator for each line can only be TAB or COMMA, and COMMA is the recommended.
    Args:
        txt_file: full path of text data file
    Returns:
        2D data array
    """
    f = open(txt_file)
    data_items = []
    for line in f:
        str_line = line
        for LF in LFs:
            if LF in line:
                str_line = line.split(LF)[0]
                break

        if str_line != '' and str_line.find('#') < 0:
            line_list = StringClass.split_string(str_line, ['\t'])
            if len(line_list) <= 1:
                line_list = StringClass.split_string(str_line, [','])
            data_items.append(line_list)
    f.close()
    return data_items
