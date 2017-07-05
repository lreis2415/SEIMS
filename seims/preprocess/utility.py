#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Global variables and utility functions
    @author   : Liangjun Zhu, Junzhi Liu
    @changelog: 16-06-16  first implementation version
                17-06-22  reformat according to pylint and google style
"""

import math
import sys

from seims.pygeoc.pygeoc.utils.utils import UtilClass

# Global variables
UTIL_ZERO = 1.e-6
MINI_SLOPE = 0.0001
DEFAULT_NODATA = -9999.
SQ2 = math.sqrt(2.)
LFs = ['\r\n', '\n\r', '\r', '\n']


def load_ini_configuration(inifile):
    """Parse ini configuration file
    deprecated by config.parse_ini_configuration() lj, 17-06-23
    Args:
        inifile: ini full path
    """
    command = '%s %s/config.py -ini %s' % (sys.executable, UtilClass.current_path(), inifile)
    UtilClass.run_command(command)


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
    """Read data items include title from text file, each data element are split by TAB.
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
            line_list = str_line.split('\t')
            data_items.append(line_list)
    f.close()
    return data_items
