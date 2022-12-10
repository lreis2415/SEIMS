"""Import precipitation data, daily or storm

    @author   : Liangjun Zhu, Junzhi Liu

    @changelog:
    - 16-12-07  - lj - rewrite for version 2.0
    - 17-07-04  - lj - reorganize according to pylint and google style
    - 17-07-05  - lj - Using bulk operation interface to improve MongoDB efficiency.
    - 17-08-05  - lj - Add Timezone preprocessor statement in the first line of data file.
    - 18-02-08  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
from datetime import timedelta

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import StringClass
from pymongo import ASCENDING

from utility import read_data_items_from_txt
from preprocess.db_mongodb import MongoUtil
from preprocess.hydro_climate_utility import HydroClimateUtilClass
from preprocess.text import DBTableNames, DataValueFields, DataType
from decimal import Decimal
import pandas as pd

class ImportPrecipitation(object):
    """Import precipitation data, daily or storm."""

    @staticmethod
    def regular_data_from_txt(climdb, data_file):
        """Regular precipitation data from text file."""
        # delete existed precipitation data
        climdb[DBTableNames.data_values].remove({DataValueFields.type: DataType.p})
        tsysin, tzonein = HydroClimateUtilClass.get_time_system_from_data_file(data_file)
        clim_data_items = read_data_items_from_txt(data_file)
        clim_flds = clim_data_items[0]
        station_id = list()
        bulk = climdb[DBTableNames.data_values].initialize_ordered_bulk_op()
        count = 0
        for fld in clim_flds:
            if not StringClass.string_in_list(fld,
                                              [DataValueFields.dt, DataValueFields.y,
                                               DataValueFields.m,
                                               DataValueFields.d, DataValueFields.hour,
                                               DataValueFields.minute, DataValueFields.second]):
                station_id.append(fld)
        for i, clim_data_item in enumerate(clim_data_items):
            if i == 0:
                continue
            dic = dict()
            precipitation = list()

            for j, clim_data_v in enumerate(clim_data_item):
                if StringClass.string_in_list(clim_flds[j], station_id):
                    precipitation.append(float(clim_data_v))
            utc_time = HydroClimateUtilClass.get_utcdatetime_from_field_values(clim_flds,
                                                                               clim_data_item,
                                                                               tsysin, tzonein)
            dic[DataValueFields.local_time] = utc_time - timedelta(minutes=tzonein * 60)
            dic[DataValueFields.time_zone] = tzonein
            dic[DataValueFields.utc] = utc_time

            for j, cur_id in enumerate(station_id):
                cur_dic = dict()
                cur_dic[DataValueFields.value] = precipitation[j]
                cur_dic[DataValueFields.id] = int(cur_id)
                cur_dic[DataValueFields.type] = DataType.p
                cur_dic[DataValueFields.time_zone] = dic[DataValueFields.time_zone]
                cur_dic[DataValueFields.local_time] = dic[DataValueFields.local_time]
                cur_dic[DataValueFields.utc] = dic[DataValueFields.utc]
                bulk.insert(cur_dic)
                count += 1
                if count % 500 == 0:  # execute each 500 records
                    MongoUtil.run_bulk(bulk)
                    bulk = climdb[DBTableNames.data_values].initialize_ordered_bulk_op()
        if count % 500 != 0:
            MongoUtil.run_bulk(bulk)
        # Create index
        climdb[DBTableNames.data_values].create_index([(DataValueFields.id, ASCENDING),
                                                       (DataValueFields.type, ASCENDING),
                                                       (DataValueFields.utc, ASCENDING)])

    """
        storm模式下，往往需要对降雨数据进行时间插值.例如：用户只有时间分辨率为1min的降雨数据，但是模拟时间步长为5s，因此需要将原数据插值为5s时间间隔
        注意：
        ①降雨数据对应的应该是一个时间分辨率的降雨量，例如：原数据时间分辨率是60s，值为2.4，则意味着1min降雨深度为2.5mm。新时间分辨率为5s，值为0.2，则意味着5s降雨深度为0.2mm
        ②conversion_matrix是单位的转换矩阵，指定origin_unit和new_unit就可以得到转换矩阵中对应的单位转换因数
        ③origin_interval和new_interval是原时间分辨率和新的时间分辨率
    """
    @staticmethod
    def storm_data_from_txt(climdb, data_file):
        conversion_matrix = [[1, 60, 3600],
                             [0.016667, 1, 60],
                             [0.000278, 0.016667, 1]]
        origin_index = 0
        new_index = 0
        origin_interval = 60
        new_interval = 5
        n_interval = int(origin_interval / new_interval)
        origin_unit = 'mm/min'
        new_unit = 'mm/s'
        origin_items = []
        new_items = []
        if origin_unit == 'mm/s':
            origin_index = 0
        elif origin_unit == 'mm/min':
            origin_index = 1
        elif origin_unit == 'mm/h':
            origin_index = 2
        else:
            print("origin unit is not supported")
            exit()

        if new_unit == 'mm/s':
            new_index = 0
        elif new_unit == 'mm/min':
            new_index = 1
        elif new_unit == 'mm/h':
            new_index = 2
        else:
            print("new unit is not supported")
            exit()
        conversion_factor = conversion_matrix[origin_index][new_index]

        """storm precipitation data from csv file."""
        # delete existed precipitation data
        climdb[DBTableNames.data_values].remove({DataValueFields.type: DataType.p})
        tsysin, tzonein = HydroClimateUtilClass.get_time_system_from_data_file(data_file)
        clim_data_items = read_data_items_from_txt(data_file)
        clim_flds = clim_data_items[0]
        station_id = list()
        bulk = climdb[DBTableNames.data_values].initialize_ordered_bulk_op()
        count = 0
        for fld in clim_flds:
            if not StringClass.string_in_list(fld,
                                              [DataValueFields.dt, DataValueFields.y,
                                               DataValueFields.m,
                                               DataValueFields.d, DataValueFields.hour,
                                               DataValueFields.minute, DataValueFields.second]):
                station_id.append(fld)
        dics = []
        # convert unit
        for i, clim_data_item in enumerate(clim_data_items):
            if i == 0:
                continue
            for j, clim_data_v in enumerate(clim_data_item):
                if j == 0:
                    continue
                clim_data_item[j] = round(float(clim_data_v) * conversion_factor * new_interval, 4)
        # time scale interpolate
        for i, clim_data_item in enumerate(clim_data_items):
            if i == 0 or (i >= len(clim_data_items) - 1):
                continue
            dic = dict()
            new_datetime = []
            new_precipitation = []

            for j, clim_data_v in enumerate(clim_data_item):
                if StringClass.string_in_list(clim_flds[j], station_id):
                    # split value interval
                    new_precipitation.append(ImportPrecipitation.split_value_ranges(float(clim_data_items[i][j]), float(clim_data_items[i + 1][j]), n_interval,i == (len(clim_data_items) -2)))
                else:
                    # split datetime interval
                    new_datetime = ImportPrecipitation.split_time_ranges(clim_data_v, clim_data_items[i + 1][0], new_interval,i == (len(clim_data_items) -2))
            # traverse interpolated datetimes
            for k, dt in enumerate(new_datetime):
                utc_time = HydroClimateUtilClass.get_utcdatetime(dt,tsysin, tzonein)
                dic[DataValueFields.local_time] = utc_time - timedelta(minutes=tzonein * 60)
                dic[DataValueFields.time_zone] = tzonein
                dic[DataValueFields.utc] = utc_time

                for j, cur_id in enumerate(station_id):
                    cur_dic = dict()
                    cur_dic[DataValueFields.value] = new_precipitation[j][k]
                    cur_dic[DataValueFields.id] = int(cur_id)
                    cur_dic[DataValueFields.type] = DataType.p
                    cur_dic[DataValueFields.time_zone] = dic[DataValueFields.time_zone]
                    cur_dic[DataValueFields.local_time] = dic[DataValueFields.local_time]
                    cur_dic[DataValueFields.utc] = dic[DataValueFields.utc]
                    bulk.insert(cur_dic)
                    dics.append(cur_dic)
                    count += 1
                    if count % 500 == 0:  # execute each 500 records
                        MongoUtil.run_bulk(bulk)
                        bulk = climdb[DBTableNames.data_values].initialize_ordered_bulk_op()
        if count % 500 != 0:
            MongoUtil.run_bulk(bulk)
        # Create index
        climdb[DBTableNames.data_values].create_index([(DataValueFields.id, ASCENDING),
                                                       (DataValueFields.type, ASCENDING),
                                                       (DataValueFields.utc, ASCENDING)])

    @staticmethod
    def split_value_ranges(from_value, to_value, n, contain_last):
        values_range = []
        dv = (to_value - from_value) / n
        for i in range(n):
            # values_range.append(Decimal(from_value + dv * i).quantize(Decimal("0.0000")))
            values_range.append(round(from_value + dv * i,4))
        if contain_last:
            values_range.append(round(to_value, 4))
        return values_range

    @staticmethod
    def split_time_ranges(from_time, to_time, frequency,contain_last):
        from_time, to_time = pd.to_datetime(from_time), pd.to_datetime(to_time)
        time_range = list(pd.date_range(from_time, to_time, freq='%sS' % frequency))
        if to_time in time_range and not contain_last:
            time_range.remove(to_time)
        time_range = [item.strftime("%Y/%m/%d %H:%M:%S") for item in time_range]
        return time_range

    @staticmethod
    def workflow(cfg, clim_db):
        """Workflow"""
        print('Import Daily Precipitation Data... ')
        # ImportPrecipitation.regular_data_from_txt(clim_db, cfg.prec_data)
        ImportPrecipitation.storm_data_from_txt(clim_db, cfg.prec_data)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    from preprocess.db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    hydroclim_db = conn[seims_cfg.climate_db]
    import time
    st = time.time()
    ImportPrecipitation.workflow(seims_cfg, hydroclim_db)
    et = time.time()
    print(et - st)

    client.close()


if __name__ == "__main__":
    main()
