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
from pymongo import ASCENDING, InsertOne

from utility import read_data_items_from_txt
from preprocess.db_mongodb import MongoUtil
from preprocess.hydro_climate_utility import HydroClimateUtilClass
from preprocess.text import DBTableNames, DataValueFields, DataType


class ImportPrecipitation(object):
    """Import precipitation data, daily or storm."""

    @staticmethod
    def regular_data_from_txt(climdb, data_file):
        """Regular precipitation data from text file."""
        # delete existed precipitation data
        climdb[DBTableNames.data_values].delete_many({DataValueFields.type: DataType.p})
        tsysin, tzonein = HydroClimateUtilClass.get_time_system_from_data_file(data_file)
        clim_data_items = read_data_items_from_txt(data_file)
        clim_flds = clim_data_items[0]
        station_id = list()

        bulk_requests = list()
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
                bulk_requests.append(InsertOne(cur_dic))

        results = MongoUtil.run_bulk_write(climdb[DBTableNames.data_values],
                                           bulk_requests)
        print('Inserted %d initial parameters!' % (results.inserted_count
                                                   if results is not None else 0))
        # Create index
        climdb[DBTableNames.data_values].create_index([(DataValueFields.id, ASCENDING),
                                                       (DataValueFields.type, ASCENDING),
                                                       (DataValueFields.utc, ASCENDING)])

    @staticmethod
    def workflow(cfg):
        """Workflow"""
        print('Import Daily Precipitation Data... ')
        ImportPrecipitation.regular_data_from_txt(cfg.climatedb, cfg.prec_data)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    import time
    st = time.time()
    ImportPrecipitation.workflow(seims_cfg)
    et = time.time()
    print(et - st)


if __name__ == "__main__":
    main()
