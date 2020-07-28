"""Interpolate hydro-climate data from not regular observed data to desired time interval.

This script is not intended to be integrated into SEIMS preprocess workflow.
   This function can be integrated into HydroClimateUtilClass in the future.

    @author   : Liangjun Zhu

    @changelog:
    - 17-07-25  - lj - initial implementation
    - 18-02-08  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import time
from collections import OrderedDict
from datetime import timedelta
from io import open
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import FileClass, StringClass, MathClass

from utility import read_data_items_from_txt
from preprocess.hydro_climate_utility import HydroClimateUtilClass


def interpolate_observed_data_to_regular_interval(in_file, time_interval, start_time, end_time,
                                                  eliminate_zero=False,
                                                  time_sys_output='UTCTIME', day_divided_hour=0):
    """
    Interpolate not regular observed data to regular time interval data.

    Todo: Not tested yet!

    Args:
        in_file: input data file, the basic format is as follows:
                 line 1: #<time_system> [<time_zone>], e.g., #LOCALTIME 8, #UTCTIME
                 line 2: DATETIME,field1,field2,...
                 line 3: YYYY-mm-dd HH:MM:SS,field1_value,field2_value,...
                 line 4: ...
                 ...
                 Field name can be PCP, FLOW, SED
                 the unit is mm/h, m3/s, g/L (i.e., kg/m3), respectively.
        time_interval: time interval, unit is minute, e.g., daily output is 1440
        start_time: start time, the format must be 'YYYY-mm-dd HH:MM:SS', and the time system
                    is based on time_sys.
        end_time: end time, see also start_time.
        eliminate_zero: Boolean flag. If true, the time interval without original records will
                        not be output.
        time_sys_output: time system of output time_system, the format must be
                  '<time_system> [<time_zone>]', e.g.,
                  'LOCALTIME'
                  'LOCALTIME 8'
                  'UTCTIME' (default)
        day_divided_hour: If the time_interval is equal to N*1440, this parameter should be
                          carefully specified. The value must range from 0 to 23. e.g.,
                          day_divided_hour ==> day ranges (all expressed as 2013-02-03)
                          0  ==> 2013-02-03 00:00:00 to 2013-02-03 23:59:59 (default)
                          8  ==> 2013-02-03 08:00:00 to 2013-02-04 07:59:59
                          20 ==> 2013-02-03 20:00:00 to 2013-02-04 19:59:59
    Returns:
        The output data files are located in the same directory with the input file.
        The nomenclature is: <field name>_<time system>_<time interval>_<nonzero>, e.g.,
        pcp_utctime_1440_nonzero.csv, flow_localtime_60.csv.
        Note that `.txt` format is also supported.
    """
    FileClass.check_file_exists(in_file)
    time_sys_input, time_zone_input = HydroClimateUtilClass.get_time_system_from_data_file(in_file)
    data_items = read_data_items_from_txt(in_file)
    flds = data_items[0][:]
    data_items.remove(flds)
    if not 0 <= day_divided_hour <= 23:
        raise ValueError('Day divided hour must range from 0 to 23!')
    try:
        date_idx = flds.index('DATETIME')
        flds.remove('DATETIME')
    except ValueError:
        raise ValueError('DATETIME must be one of the fields!')
    # available field
    available_flds = ['FLOW', 'SED', 'PCP']

    def check_avaiable_field(cur_fld):
        """Check if the given field name is supported."""
        support_flag = False
        for fff in available_flds:
            if fff.lower() in cur_fld.lower():
                support_flag = True
                break
        return support_flag

    ord_data = OrderedDict()
    time_zone_output = time.timezone // 3600
    if time_sys_output.lower().find('local') >= 0:
        tmpstrs = StringClass.split_string(time_sys_output, [' '])
        if len(tmpstrs) == 2 and MathClass.isnumerical(tmpstrs[1]):
            time_zone_output = -1 * int(tmpstrs[1])
        time_sys_output = 'LOCALTIME'
    else:
        time_sys_output = 'UTCTIME'
        time_zone_output = 0
    for item in data_items:
        org_datetime = StringClass.get_datetime(item[date_idx])
        if time_sys_input == 'LOCALTIME':
            org_datetime += timedelta(hours=time_zone_input)  # now, org_datetime is UTC time.
        if time_sys_output == 'LOCALTIME':
            org_datetime -= timedelta(hours=time_zone_output)
        # now, org_datetime is consistent with the output time system
        ord_data[org_datetime] = list()
        for i, v in enumerate(item):
            if i == date_idx:
                continue
            if MathClass.isnumerical(v):
                ord_data[org_datetime].append(float(v))
            else:
                ord_data[org_datetime].append(v)
    # print(ord_data)
    itp_data = OrderedDict()
    out_time_delta = timedelta(minutes=time_interval)
    sdatetime = StringClass.get_datetime(start_time)
    edatetime = StringClass.get_datetime(end_time)
    item_dtime = sdatetime
    if time_interval % 1440 == 0:
        item_dtime = sdatetime.replace(hour=0, minute=0, second=0) + \
                     timedelta(minutes=day_divided_hour * 60)
    while item_dtime <= edatetime:
        # print(item_dtime)
        # if item_dtime.month == 12 and item_dtime.day == 31:
        #     print("debug")
        sdt = item_dtime  # start datetime of records
        edt = item_dtime + out_time_delta  # end datetime of records
        # get original data items
        org_items = list()
        pre_dt = list(ord_data.keys())[0]
        pre_added = False
        for i, v in list(ord_data.items()):
            if sdt <= i < edt:
                if not pre_added and pre_dt < sdt < i and sdt - pre_dt < out_time_delta:
                    # only add one item that less than sdt.
                    org_items.append([pre_dt] + ord_data.get(pre_dt))
                    pre_added = True
                org_items.append([i] + v)
            if i > edt:
                break
            pre_dt = i
        if len(org_items) > 0:
            org_items.append([edt])  # Just add end time for compute convenient
            if org_items[0][0] < sdt:
                org_items[0][0] = sdt  # set the begin datetime of current time interval
        # if eliminate time interval without original records
        # initial interpolated list
        itp_data[item_dtime] = [0.] * len(flds)
        if len(org_items) == 0:
            if eliminate_zero:
                itp_data.popitem()
            item_dtime += out_time_delta
            continue
        # core interpolation code
        flow_idx = -1
        for v_idx, v_name in enumerate(flds):
            if not check_avaiable_field(v_name):
                continue
            if 'SED' in v_name.upper():  # FLOW must be existed
                for v_idx2, v_name2 in enumerate(flds):
                    if 'FLOW' in v_name2.upper():
                        flow_idx = v_idx2
                        break
                if flow_idx < 0:
                    raise RuntimeError('To interpolate SED, FLOW must be provided!')
        for v_idx, v_name in enumerate(flds):
            if not check_avaiable_field(v_name):
                continue
            itp_value = 0.
            itp_auxiliary_value = 0.
            for org_item_idx, org_item_dtv in enumerate(org_items):
                if org_item_idx == 0:
                    continue
                org_item_dt = org_item_dtv[0]
                pre_item_dtv = org_items[org_item_idx - 1]
                pre_item_dt = pre_item_dtv[0]
                tmp_delta_dt = org_item_dt - pre_item_dt
                tmp_delta_secs = tmp_delta_dt.days * 86400 + tmp_delta_dt.seconds
                if 'SED' in v_name.upper():
                    itp_value += pre_item_dtv[v_idx + 1] * pre_item_dtv[flow_idx + 1] * \
                                 tmp_delta_secs
                    itp_auxiliary_value += pre_item_dtv[flow_idx + 1] * tmp_delta_secs
                else:
                    itp_value += pre_item_dtv[v_idx + 1] * tmp_delta_secs
            if 'SED' in v_name.upper():
                if MathClass.floatequal(itp_auxiliary_value, 0.):
                    itp_value = 0.
                    print('WARNING: Flow is 0 for %s, please check!' %
                          item_dtime.strftime('%Y-%m-%d %H:%M:%S'))
                itp_value /= itp_auxiliary_value
            elif 'FLOW' in v_name.upper():
                itp_value /= (out_time_delta.days * 86400 + out_time_delta.seconds)
            elif 'PCP' in v_name.upper():  # the input is mm/h, and output is mm
                itp_value /= 3600.
            itp_data[item_dtime][v_idx] = round(itp_value, 4)
        item_dtime += out_time_delta

    # for i, v in itp_data.items():
    #     print(i, v)
    # output to files
    work_path = os.path.dirname(in_file)
    header_str = '#' + time_sys_output
    if time_sys_output == 'LOCALTIME':
        header_str = header_str + ' ' + str(time_zone_output)
    for idx, fld in enumerate(flds):
        if not check_avaiable_field(fld):
            continue
        file_name = fld + '_' + time_sys_output + '_' + str(time_interval)
        if eliminate_zero:
            file_name += '_nonzero'
        file_name += '.csv'
        out_file = work_path + os.path.sep + file_name
        with open(out_file, 'w', encoding='utf-8') as f:
            f.write('%s\n' % header_str)
            f.write('DATETIME,%s\n' % fld)
            for i, v in list(itp_data.items()):
                cur_line = i.strftime('%Y-%m-%d %H:%M:%S') + ',' + str(v[idx]) + '\n'
                f.write('%s' % cur_line)


def main():
    """TEST CODE"""
    data_file = r'C:\z_data\ChangTing\observed\HE3520133140\2011\2011_flowsed_storm_not_regular.txt'
    time_interval = 1440
    stime = '2011-01-01 00:00:00'
    etime = '2011-12-31 23:59:59'
    elim_zero = False
    out_time_system = 'UTCTIME'
    divided_hour = 0
    interpolate_observed_data_to_regular_interval(data_file, time_interval, stime, etime,
                                                  elim_zero, out_time_system, divided_hour)


if __name__ == "__main__":
    main()
