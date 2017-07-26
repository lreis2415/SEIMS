#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Interpolate hydro-climate data from not regular observed data to desired time interval.
   This script is not intended to be integrated into SEIMS preprocess workflow.
   This function can be integrated into HydroClimateUtilClass in the future.
    @author   : Liangjun Zhu
    @changelog: 17-07-25  lj - initial implementation
"""
import os
import time
from collections import OrderedDict
from datetime import timedelta

from seims.preprocess.hydro_climate_utility import HydroClimateUtilClass
from seims.preprocess.utility import read_data_items_from_txt
from seims.pygeoc.pygeoc.utils.utils import FileClass, StringClass, MathClass


def interpolate_observed_data_to_regular_interval(in_file, time_interval, start_time, end_time,
                                                  time_sys_output='UTCTIME', day_divided_hour=24):
    """
    Interpolate not regular observed data to regular time interval data.
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
        time_sys_output: time system of output time_system, the format must be
                  '<time_system> [<time_zone>]', e.g.,
                  'LOCALTIME'
                  'LOCALTIME 8'
                  'UTCTIME' (default)
        day_divided_hour: If the time_interval is equal to N*1440, this parameter should be
                          carefully specified. The value must range from 1 to 24. e.g.,
                          day_divided_hour ==> day ranges (all expressed as 2013-02-03)
                          24 ==> 2013-02-03 00:00:00 to 2013-02-03 23:59:59 (default)
                          20 ==> 2013-02-02 20:00:00 to 2013-02-03 19:59:59
                          8  ==> 2013-02-03 08:00:00 to 2013-02-03 07:59:59
    Returns:
        The output data files are located in the same directory with the input file.
        The nomenclature is: <field name>_<time system>_<time interval>, e.g.,
        pcp_utctime_1440.txt, flow_localtime_60.txt
    """
    FileClass.check_file_exists(in_file)
    time_sys_input, time_zone_input = HydroClimateUtilClass.get_time_system_from_data_file(in_file)
    data_items = read_data_items_from_txt(in_file)
    flds = data_items[0][:]
    data_items.remove(flds)
    try:
        date_idx = flds.index('DATETIME')
        flds.remove('DATETIME')
    except ValueError:
        raise ValueError("DATETIME must be one of the fields!")
    ord_data = OrderedDict()
    time_zone_output = time.timezone / -3600
    if time_sys_output.lower().find('local') >= 0:
        tmpstrs = StringClass.split_string(time_sys_output, [' '])
        if len(tmpstrs) == 2 and MathClass.isnumerical(tmpstrs[1]):
            time_zone_output = int(tmpstrs[1])
        time_sys_output = 'LOCALTIME'
    else:
        time_sys_output = 'UTCTIME'
        time_zone_output = 0
    for item in data_items:
        org_datetime = HydroClimateUtilClass.get_datetime_from_string(item[date_idx])
        if time_sys_input == 'LOCALTIME':
            org_datetime -= timedelta(hours=time_zone_input)
        # now, org_datetime is UTC time.
        if time_sys_output == 'LOCALTIME':
            org_datetime += timedelta(hours=time_zone_output)
        # now, org_datetime is consistent with the output time system
        ord_data[org_datetime] = []
        for i, v in enumerate(item):
            if i == date_idx:
                continue
            ord_data[org_datetime].append(float(v))
    # print (ord_data)
    itp_data = OrderedDict()
    out_time_delta = timedelta(minutes=time_interval)
    sdatetime = HydroClimateUtilClass.get_datetime_from_string(start_time)
    edatetime = HydroClimateUtilClass.get_datetime_from_string(end_time)
    item_dtime = sdatetime + out_time_delta - timedelta(seconds=1)
    if time_interval % 1440 == 0:
        item_dtime = sdatetime.replace(hour=0, minute=0, second=0) + \
                     timedelta(minutes=day_divided_hour * 60) - timedelta(seconds=1)
    while item_dtime <= edatetime:
        # print (item_dtime)
        itp_data[item_dtime] = []
        sdt = item_dtime - out_time_delta + timedelta(seconds=1)  # current start datetime
        edt = item_dtime + timedelta(seconds=1)  # current start datetime
        # get original data items
        org_items = []
        pre_dt = list(ord_data.keys())[0]
        pre_added = False
        for i, v in ord_data.items():
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
        for v_idx, v_name in enumerate(flds):
            itp_data[item_dtime].append(0)
        if len(org_items) == 0:
            item_dtime += out_time_delta
            continue
        # core interpolation code
        flow_idx = -1
        for v_idx, v_name in enumerate(flds):
            if 'SED' in v_name.upper():  # FLOW must be existed
                for v_idx2, v_name2 in enumerate(flds):
                    if 'FLOW' in v_name2.upper():
                        flow_idx = v_idx2
                        break
                if flow_idx < 0:
                    raise RuntimeError("To interpolate SED, FLOW must be provided!")
        for v_idx, v_name in enumerate(flds):
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
                itp_value /= itp_auxiliary_value
            elif 'FLOW' in v_name.upper():
                itp_value /= (out_time_delta.days * 86400 + out_time_delta.seconds)
            elif 'PCP' in v_name.upper():  # the input is mm/h, and output is mm
                itp_value /= 3600.
            itp_data[item_dtime][v_idx] = round(itp_value, 4)
        item_dtime += out_time_delta

    # for i, v in itp_data.items():
    #     print (i, v)
    # output to files
    work_path = os.path.dirname(in_file)
    header_str = '#' + time_sys_output
    if time_sys_output == 'LOCALTIME':
        header_str = header_str + ' ' + str(time_zone_output)
    for idx, fld in enumerate(flds):
        file_name = fld + '_' + time_sys_output + '_' + str(time_interval) + '.txt'
        out_file = work_path + os.sep + file_name
        f = open(out_file, 'w')
        f.write(header_str + '\n')
        f.write('DATETIME,' + fld + '\n')
        for i, v in itp_data.items():
            cur_line = i.strftime('%Y-%m-%d %H:%M:%S') + ',' + str(v[idx]) + '\n'
            f.write(cur_line)
        f.close()


def main():
    """TEST CODE"""
    data_file = r'C:\z_data\ChangTing\observed\HE3520133140\2012\2012_storm_flow_sediment_not_regular.txt'
    #data_file = r'C:\z_data\ChangTing\climate\pcp\2014_pcp.txt'
    time_interval = 1440
    time_system = 'UTCTIME'
    stime = '2012-01-01 00:00:00'
    etime = '2012-12-31 23:59:59'
    divided_hour = 24
    interpolate_observed_data_to_regular_interval(data_file, time_interval,
                                                  stime, etime, time_system, divided_hour)


if __name__ == "__main__":
    main()
