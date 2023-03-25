# coding: utf-8
""" 本脚本把SEIMS气象数据转换成SWAT气象数据
    还不是很通用，只考虑了梅溪小流域的数据，功能不完善，坑多，使用的时候应该结合具体情况分析代码。
    使用的时候要放到SEIMS/seims文件夹下，因为需要依赖预处理脚本去计算Solar radiation。
    @author: ShenShen
"""
import math

import numpy as np
import pandas as pd
import csv
from datetime import datetime

from pygeoc.utils import DateClass

from preprocess.hydro_climate_utility import HydroClimateUtilClass

INPUT_DIR = 'C:/Users/shenshen/Desktop/meixi/data_prepare/climate/'
OUTPUT_DIR = 'C:/Users/shenshen/Desktop/meixi/data_prepare/climate/'
SITES_M_FILE = 'Sites_M.csv'
SITES_P_FILE = 'Sites_P.csv'
METEO_DAILY = 'meteo_daily.csv'
PCP_DAILY = 'pcp_daily.csv'
VARIABLES = 'Variables.csv'


# 接收SEIMS时间字符串，返回SWAT时间字符串
def convert_date_str(date_str):
    dt = datetime.strptime(date_str, '%Y-%m-%d %H:%M:%S')
    return dt.strftime('%Y%m%d')


# SWAT气象站点文件以“气象要素”命名，例如pcp.txt
# 观测数据文件以“站点要素”命名，例如robpcp.txt
# 该函数处理pcp文件
def convert_pcp():
    stationID2name = dict()
    input_sites_p = INPUT_DIR + SITES_P_FILE
    output_sites_p = OUTPUT_DIR + 'pcp.txt'
    fieldnames = ['ID', 'NAME', 'LAT', 'LONG', 'ELEVATION']
    # 避免空行python 3: open(output_sites_p,'w',newline='')
    with open(input_sites_p) as fi_sites_p, open(output_sites_p, 'wb') as fo_sites_p:
        reader = csv.DictReader(fi_sites_p)
        writer = csv.DictWriter(fo_sites_p, fieldnames=fieldnames)
        writer.writeheader()
        for row in reader:
            # print(row)
            # write pcp.txt
            record = dict()
            record['ID'] = row['StationID']
            record['NAME'] = row['Name'] + 'pcp' #与观测数据文件名保持一致
            record['LAT'] = row['Lat']
            record['LONG'] = row['Lon']
            record['ELEVATION'] = row['Elevation']
            writer.writerow(record)
            stationID2name[row['StationID']] = row['Name']

    input_daily_p = INPUT_DIR + PCP_DAILY
    df_pcp = pd.read_csv(input_daily_p, skiprows=1)

    print(df_pcp.head())
    sw_datestr = convert_date_str(df_pcp.loc[0, "DATETIME"])
    for col in df_pcp:
        if col == 'DATETIME': continue
        station_name = stationID2name[col]
        station_filename = OUTPUT_DIR + station_name + 'pcp.txt'
        with open(station_filename, 'w') as fo_station:
            fo_station.write(sw_datestr)
            fo_station.write('\n')
            for item in df_pcp[col].values.tolist():
                fo_station.write(str(item))
                fo_station.write('\n')


def convert_meteo():
    meteo_features = {'tmp': ['TMAX', 'TMIN'],
                      'hmd': 'RM',
                      'wnd': 'WS',
                      'slr': 'SSD'}  # mapping keys 除了上面处理过的降水
    input_sites_m = INPUT_DIR + SITES_M_FILE
    stationID2name = dict()
    fieldnames = ['ID', 'NAME', 'LAT', 'LONG', 'ELEVATION']
    # 避免空行python 3: open(output_sites_p,'w',newline='')
    with open(input_sites_m) as fi_sites_m:
        reader = csv.DictReader(fi_sites_m)
        for row in reader:
            record = dict()
            record['ID'] = row['StationID']
            record['NAME'] = row['Name']
            record['LAT'] = row['Lat']
            record['LONG'] = row['Lon']
            record['ELEVATION'] = row['Elevation']

            # 每种气象要素一个单独的文件
            for f in meteo_features.keys():
                output_sites_m = OUTPUT_DIR + f + '.txt'
                record['NAME'] += f  # 与观测数据文件名保持一致
                with open(output_sites_m, 'wb') as fo_sites_m:
                    writer = csv.DictWriter(fo_sites_m, fieldnames=fieldnames)
                    writer.writeheader()
                    writer.writerow(record)

            stationID2name[row['StationID']] = row

    input_daily_m = INPUT_DIR + METEO_DAILY
    df_m = pd.read_csv(input_daily_m, skiprows=1)

    print(df_m.head())
    sw_datestr = convert_date_str(df_m.loc[0, "DATETIME"])
    # 先实现一个气象站，后面有需要再扩充吧
    station_id = stationID2name.keys()[0]
    station_name = stationID2name[station_id]['Name']
    for mf, col_name in meteo_features.items():
        sub_df = None
        if mf == 'tmp':  # 温度数据有两列
            sub_df = df_m.loc[df_m['StationID'] == int(station_id), col_name]
            station_filename = OUTPUT_DIR + station_name + mf + '.txt'
            with open(station_filename, 'w') as fo_station:
                fo_station.write(sw_datestr)
                fo_station.write('\n')
                for item in sub_df.values:
                    tmax = str(item[0])
                    tmin = str(item[1])
                    fo_station.write(','.join([tmax, tmin]))
                    fo_station.write('\n')
        elif mf == 'slr':  # 光照时长需转换
            sub_df = df_m.loc[df_m['StationID'] == int(station_id), [col_name, 'DATETIME']]
            station_filename = OUTPUT_DIR + station_name + mf + '.txt'
            with open(station_filename, 'w') as fo_station:
                fo_station.write(sw_datestr)
                fo_station.write('\n')
                for item in sub_df.values:
                    ssd = float(item[0])
                    dt = datetime.strptime(item[1], '%Y-%m-%d %H:%M:%S')
                    station_lat = float(stationID2name[station_id]['Lat'])
                    sr = round(HydroClimateUtilClass.rs(DateClass.day_of_year(dt),
                                                        ssd, station_lat * math.pi / 180.), 1)
                    fo_station.write(str(sr))
                    fo_station.write('\n')
        else:  # 湿度、风速数据
            if mf == 'hmd':  # 湿度数据需除100
                sub_df = df_m.loc[df_m['StationID'] == int(station_id), col_name] / 100.0
            else:
                sub_df = df_m.loc[df_m['StationID'] == int(station_id), col_name]
            station_filename = OUTPUT_DIR + station_name + mf + '.txt'
            with open(station_filename, 'w') as fo_station:
                fo_station.write(sw_datestr)
                fo_station.write('\n')
                for item in sub_df.values.tolist():
                    fo_station.write(str(item))
                    fo_station.write('\n')


if __name__ == '__main__':
    convert_pcp()
    convert_meteo()
