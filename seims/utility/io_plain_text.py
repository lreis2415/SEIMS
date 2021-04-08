"""Read and write of plain text file.

    @author   : Liangjun Zhu

    @changelog:
    - 18-10-29 - lj - Extract from other packages.
"""
from __future__ import absolute_import, unicode_literals

import json
from io import open
import os
from collections import OrderedDict
from datetime import datetime

from typing import List, Dict, Union, AnyStr
from numpy import ndarray as np_array
from pygeoc.utils import StringClass, UtilClass, FileClass


class SpecialJsonEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np_array):
            return obj.tolist()
        elif isinstance(obj, datetime):
            return obj.strftime('%Y-%m-%d %H:%M:%S')
        return json.JSONEncoder.default(self, obj)


def status_output(status_msg, percent, file_name):
    # type: (AnyStr, Union[int, float], AnyStr) -> None
    """Print status and flush to file.
    Args:
        status_msg: status message
        percent: percentage rate of progress
        file_name: file name
    """
    UtilClass.writelog(file_name, "[Output] %d..., %s" % (percent, status_msg), 'a')


def read_data_items_from_txt(txt_file):
    # type: (AnyStr) -> List[List[AnyStr]]
    """Read data items include title from text file, each data element are split by TAB or COMMA.
       Be aware, the separator for each line can only be TAB or COMMA, and COMMA is the recommended.
    Args:
        txt_file: full path of text data file
    Returns:
        2D data array
    """
    data_items = list()
    with open(txt_file, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            str_line = line.strip()
            if str_line != '' and str_line.find('#') < 0:
                line_list = StringClass.split_string(str_line, ['\t'])
                if len(line_list) <= 1:
                    line_list = StringClass.split_string(str_line, [','])
                data_items.append(line_list)
    return data_items


def read_simulation_from_txt(ws,  # type: AnyStr
                             plot_vars,  # type: List[AnyStr]
                             subbsnID,  # type: int
                             stime,  # type: datetime
                             etime  # type: datetime
                             ):
    # type: (...) -> (List[AnyStr], Dict[datetime, List[float]])
    """
    Read simulation data from text file according to subbasin ID.
    Returns:
        1. Matched variable names, [var1, var2, ...]
        2. Simulation data dict of all plotted variables, with UTCDATETIME.
           {Datetime: [value_of_var1, value_of_var2, ...], ...}
    """
    plot_vars_existed = list()
    sim_data_dict = OrderedDict()
    for i, v in enumerate(plot_vars):
        txtfile = ws + os.path.sep + v + '.txt'
        if not FileClass.is_file_exists(txtfile):
            print('WARNING: Simulation variable file: %s is not existed!' % txtfile)
            continue
        data_items = read_data_items_from_txt(txtfile)
        found = False
        data_available = False
        for item in data_items:
            item_vs = StringClass.split_string(item[0], ' ', elim_empty=True)
            if len(item_vs) == 2:
                if int(item_vs[1]) == subbsnID and not found:
                    found = True
                elif int(item_vs[1]) != subbsnID and found:
                    break
            if not found:
                continue
            if len(item_vs) != 3:
                continue
            date_str = '%s %s' % (item_vs[0], item_vs[1])
            sim_datetime = StringClass.get_datetime(date_str, "%Y-%m-%d %H:%M:%S")

            if stime <= sim_datetime <= etime:
                if sim_datetime not in sim_data_dict:
                    sim_data_dict[sim_datetime] = list()
                sim_data_dict[sim_datetime].append(float(item_vs[2]))
                data_available = True
        if data_available:
            plot_vars_existed.append(v)

    print('Read simulation from %s to %s done.' % (stime.strftime('%c'),
                                                   etime.strftime('%c')))
    return plot_vars_existed, sim_data_dict
