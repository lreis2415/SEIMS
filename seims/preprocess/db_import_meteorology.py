"""Meteorological daily data import, and calculate related statistical values

    @author   : Liangjun Zhu, Junzhi Liu, Fang Shen

    @changelog:
    - 16-12-07  - lj - rewrite for version 2.0
    - 17-06-26  - lj - reorganize according to pylint and google style
    - 17-07-05  - lj - Using bulk operation interface to improve MongoDB efficiency.
    - 17-08-05  - lj - Add Timezone preprocessor statement in the first line of data file.
    - 18-02-08  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
from datetime import timedelta
import logging
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import DateClass, StringClass
from pymongo import ASCENDING, InsertOne

from utility import read_data_items_from_txt, DEFAULT_NODATA, PI
from preprocess.db_mongodb import MongoUtil
from preprocess.hydro_climate_utility import HydroClimateUtilClass
from preprocess.text import DBTableNames, DataValueFields, DataType, VariableDesc


class ClimateStats(object):
    """Common used annual climate statistics based on mean temperature, e.g. PHU."""
    _T_BASE = 0.

    def __init__(self, t_base=0.):
        ClimateStats._T_BASE = t_base
        self.Count = dict()
        self.MeanTmp = dict()
        self.PHUTOT = dict()
        self.MeanTmp0 = 0.
        self.PHU0 = 0.

    def add_item(self, item_dict):
        """Add mean temperature of each day. Dict MUST have {YEAR: 2017, TMEAN: 10.} at least."""
        if DataValueFields.y not in list(item_dict.keys()):
            raise ValueError('The hydroClimate dict must have year!')
        if DataType.mean_tmp not in list(item_dict.keys()):
            raise ValueError('The hydroClimate dict must have mean temperature!')
        cur_y = item_dict[DataValueFields.y]
        cur_tmp = item_dict[DataType.mean_tmp]
        if cur_y not in list(self.Count.keys()):
            self.Count[cur_y] = 1
            self.MeanTmp[cur_y] = cur_tmp
            if cur_tmp > ClimateStats._T_BASE:
                self.PHUTOT[cur_y] = cur_tmp
            else:
                self.PHUTOT[cur_y] = 0.
        else:
            self.Count[cur_y] += 1
            self.MeanTmp[cur_y] += cur_tmp
            if cur_tmp > ClimateStats._T_BASE:
                self.PHUTOT[cur_y] += cur_tmp

    def annual_stats(self):
        """Calculate annual statistics."""
        for Y, v in list(self.Count.items()):
            self.MeanTmp[Y] = round(self.MeanTmp[Y] / v, 1)
            self.MeanTmp0 += self.MeanTmp[Y]
            self.PHUTOT[Y] = round(self.PHUTOT[Y], 1)
            self.PHU0 += self.PHUTOT[Y]
        self.PHU0 = round(self.PHU0 / len(list(self.Count.keys())), 1)
        self.MeanTmp0 = round(self.MeanTmp0 / len(list(self.Count.keys())), 1)


class ImportMeteoData(object):
    """Meteorological daily data import, and calculate related statistical values"""

    @staticmethod
    def daily_data_from_txt(climdb, data_txt_file, sites_info_dict):
        """Import climate data table"""
        tsysin, tzonein = HydroClimateUtilClass.get_time_system_from_data_file(data_txt_file)
        clim_data_items = read_data_items_from_txt(data_txt_file)
        clim_flds = clim_data_items[0]
        # PHUCalDic is used for Calculating potential heat units (PHU)
        # for each climate station and each year.
        # format is {StationID:{Year1:[values],Year2:[Values]...}, ...}
        # PHUCalDic = {}
        # format: {StationID1: climateStats1, ...}
        hydro_climate_stats = dict()
        required_flds = [DataType.max_tmp, DataType.min_tmp, DataType.rm, DataType.ws]
        output_flds = [DataType.mean_tmp, DataType.max_tmp, DataType.min_tmp,
                       DataType.rm, DataType.pet, DataType.ws, DataType.sr]
        # remove existed records
        for fld in output_flds:
            climdb[DBTableNames.data_values].delete_many({'TYPE': fld})
        for fld in required_flds:
            if not StringClass.string_in_list(fld, clim_flds):
                raise ValueError('Meteorological Daily data MUST contain %s!' % fld)

        bulk_requests = list()
        count = 0
        for i, cur_clim_data_item in enumerate(clim_data_items):
            if i == 0:
                continue
            dic = dict()
            cur_ssd = DEFAULT_NODATA

            for j, clim_data_v in enumerate(cur_clim_data_item):
                if StringClass.string_match(clim_flds[j], DataValueFields.id):
                    dic[DataValueFields.id] = int(clim_data_v)
                elif StringClass.string_match(clim_flds[j], DataType.mean_tmp):
                    dic[DataType.mean_tmp] = float(clim_data_v)
                elif StringClass.string_match(clim_flds[j], DataType.min_tmp):
                    dic[DataType.min_tmp] = float(clim_data_v)
                elif StringClass.string_match(clim_flds[j], DataType.max_tmp):
                    dic[DataType.max_tmp] = float(clim_data_v)
                elif StringClass.string_match(clim_flds[j], DataType.pet):
                    dic[DataType.pet] = float(clim_data_v)
                elif StringClass.string_match(clim_flds[j], DataType.sr):
                    dic[DataType.sr] = float(clim_data_v)
                elif StringClass.string_match(clim_flds[j], DataType.ws):
                    dic[DataType.ws] = float(clim_data_v)
                elif StringClass.string_match(clim_flds[j], DataType.rm):
                    dic[DataType.rm] = float(clim_data_v) * 0.01
                elif StringClass.string_match(clim_flds[j], DataType.ssd):
                    cur_ssd = float(clim_data_v)
            # Get datetime and utc/local transformation
            utc_time = HydroClimateUtilClass.get_utcdatetime_from_field_values(clim_flds,
                                                                               cur_clim_data_item,
                                                                               tsysin, tzonein)
            dic[DataValueFields.local_time] = utc_time - timedelta(minutes=tzonein * 60)
            dic[DataValueFields.time_zone] = tzonein
            dic[DataValueFields.utc] = utc_time
            dic[DataValueFields.y] = utc_time.year

            # Do if some of these data are not provided
            if DataType.mean_tmp not in list(dic.keys()):
                dic[DataType.mean_tmp] = (dic[DataType.max_tmp] + dic[DataType.min_tmp]) / 2.
            if DataType.sr not in list(dic.keys()):
                if cur_ssd == DEFAULT_NODATA:
                    raise ValueError(DataType.sr + ' or ' + DataType.ssd + ' must be provided!')
                else:
                    if dic[DataValueFields.id] in list(sites_info_dict.keys()):
                        cur_lon, cur_lat = sites_info_dict[dic[DataValueFields.id]].lon_lat()
                        sr = round(HydroClimateUtilClass.rs(DateClass.day_of_year(utc_time),
                                                            float(cur_ssd), cur_lat * PI / 180.), 1)
                        dic[DataType.sr] = sr

            for fld in output_flds:
                cur_dic = dict()
                if fld in list(dic.keys()):
                    cur_dic[DataValueFields.value] = dic[fld]
                    cur_dic[DataValueFields.id] = dic[DataValueFields.id]
                    cur_dic[DataValueFields.utc] = dic[DataValueFields.utc]
                    cur_dic[DataValueFields.time_zone] = dic[DataValueFields.time_zone]
                    cur_dic[DataValueFields.local_time] = dic[DataValueFields.local_time]
                    cur_dic[DataValueFields.type] = fld

                    bulk_requests.append(InsertOne(cur_dic))
                    count += 1
                    # if count % 500 == 0:  # execute each 500 records
                    #     results = MongoUtil.run_bulk_write(climdb[DBTableNames.data_values],
                    #                                        bulk_requests)
                    #     print('Inserted %d initial parameters!' % (results.inserted_count
                    #                                                if results is not None else 0))
                    #     bulk_requests.clear()

            if dic[DataValueFields.id] not in list(hydro_climate_stats.keys()):
                hydro_climate_stats[dic[DataValueFields.id]] = ClimateStats()
            hydro_climate_stats[dic[DataValueFields.id]].add_item(dic)
        # execute the remained records
        # if count % 500 != 0:
        results = MongoUtil.run_bulk_write(climdb[DBTableNames.data_values],
                                           bulk_requests)
        logging.info('Inserted %d data items!' % (results.inserted_count
                                           if results is not None else 0))

        for item, cur_climate_stats in list(hydro_climate_stats.items()):
            cur_climate_stats.annual_stats()
        # Create index
        climdb[DBTableNames.data_values].create_index([(DataValueFields.id, ASCENDING),
                                                       (DataValueFields.type, ASCENDING),
                                                       (DataValueFields.utc, ASCENDING)])
        # prepare dic for MongoDB
        for s_id, stats_v in list(hydro_climate_stats.items()):
            for YYYY in list(stats_v.Count.keys()):
                # import annual mean PHU
                cur_dic = dict()
                cur_dic[DataValueFields.value] = stats_v.PHUTOT[YYYY]
                cur_dic[DataValueFields.id] = s_id
                cur_dic[DataValueFields.y] = YYYY
                cur_dic[VariableDesc.unit] = 'heat units'
                cur_dic[VariableDesc.type] = DataType.phu_tot
                curfilter = {DataValueFields.id: s_id,
                             VariableDesc.type: DataType.phu_tot,
                             DataValueFields.y: YYYY}
                climdb[DBTableNames.annual_stats].find_one_and_replace(curfilter, cur_dic,
                                                                       upsert=True)
                # import annual mean temperature
                cur_dic[VariableDesc.type] = DataType.mean_tmp
                cur_dic[VariableDesc.unit] = 'deg C'
                cur_dic[DataValueFields.value] = stats_v.MeanTmp[YYYY]
                curfilter = {DataValueFields.id: s_id,
                             VariableDesc.type: DataType.mean_tmp,
                             DataValueFields.y: YYYY}
                climdb[DBTableNames.annual_stats].find_one_and_replace(curfilter, cur_dic,
                                                                       upsert=True)
            # import multi-annual mean PHU
            mul_dic = dict()
            mul_dic[DataValueFields.value] = stats_v.PHU0
            mul_dic[DataValueFields.id] = s_id
            mul_dic[DataValueFields.y] = DEFAULT_NODATA
            mul_dic[VariableDesc.unit] = 'heat units'
            mul_dic[VariableDesc.type] = DataType.phu0
            curfilter = {DataValueFields.id: s_id,
                         VariableDesc.type: DataType.phu0,
                         DataValueFields.y: DEFAULT_NODATA}
            climdb[DBTableNames.annual_stats].find_one_and_replace(curfilter, mul_dic,
                                                                   upsert=True)
            # import multi-annual mean temperature
            mul_dic[VariableDesc.type] = DataType.mean_tmp0
            mul_dic[VariableDesc.unit] = 'deg C'
            mul_dic[DataValueFields.value] = stats_v.MeanTmp0
            curfilter = {DataValueFields.id: s_id,
                         VariableDesc.type: DataType.mean_tmp0,
                         DataValueFields.y: DEFAULT_NODATA}
            climdb[DBTableNames.annual_stats].find_one_and_replace(curfilter, mul_dic,
                                                                   upsert=True)

    @staticmethod
    def workflow(cfg):
        """Workflow"""
        logging.info('Import Daily Meteorological Data... ')
        site_m_loc = HydroClimateUtilClass.query_climate_sites(cfg.climatedb, 'M')
        ImportMeteoData.daily_data_from_txt(cfg.climatedb, cfg.Meteo_data, site_m_loc)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    import time
    st = time.time()
    ImportMeteoData.workflow(seims_cfg)
    et = time.time()
    logging.info(et - st)


if __name__ == "__main__":
    main()
