#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Meteorological daily data import, and calculate related statistical values
    @author   : Liangjun Zhu, Junzhi Liu, Fang Shen
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-26  lj - reorganize according to pylint and google style
                17-07-05  lj - Using bulk operation interface to improve MongoDB efficiency.
"""
import datetime
import math
import time

from pymongo import ASCENDING

from seims.preprocess.hydro_climate_utility import HydroClimateUtilClass
from seims.preprocess.text import DBTableNames, DataValueFields, DataType, VariableDesc
from seims.preprocess.utility import read_data_items_from_txt, DEFAULT_NODATA
from seims.pygeoc.pygeoc.utils.utils import DateClass, StringClass


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
        if DataValueFields.y not in item_dict.keys():
            raise ValueError("The hydroClimate dict must have year!")
        if DataType.mean_tmp not in item_dict.keys():
            raise ValueError("The hydroClimate dict must have mean temperature!")
        cur_y = item_dict[DataValueFields.y]
        cur_tmp = item_dict[DataType.mean_tmp]
        if cur_y not in self.Count.keys():
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
        for Y, v in self.Count.items():
            self.MeanTmp[Y] = round(self.MeanTmp[Y] / v, 1)
            self.MeanTmp0 += self.MeanTmp[Y]
            self.PHUTOT[Y] = round(self.PHUTOT[Y], 1)
            self.PHU0 += self.PHUTOT[Y]
        self.PHU0 = round(self.PHU0 / len(self.Count.keys()), 1)
        self.MeanTmp0 = round(self.MeanTmp0 / len(self.Count.keys()), 1)


class ImportMeteoData(object):
    """Meteorological daily data import, and calculate related statistical values"""

    @staticmethod
    def daily_data_from_txt(db, data_txt_file, sites_info_dict, is_first):
        """Import climate data table"""
        clim_data_items = read_data_items_from_txt(data_txt_file)
        clim_flds = clim_data_items[0]
        # PHUCalDic is used for Calculating potential heat units (PHU)
        # for each climate station and each year.
        # format is {StationID:{Year1:[values],Year2:[Values]...}, ...}
        # PHUCalDic = {}
        # format: {StationID1: climateStats1, ...}
        hydro_climate_stats = {}
        required_flds = [DataValueFields.y, DataValueFields.m, DataValueFields.d,
                         DataType.max_tmp, DataType.min_tmp,
                         DataType.rm, DataType.ws]
        for fld in required_flds:
            if not StringClass.string_in_list(fld, clim_flds):
                raise ValueError("Meteorological Daily data is invalid, please Check!")
        # Create bulk object
        bulk = db[DBTableNames.data_values].initialize_ordered_bulk_op()
        count = 0
        for i, cur_clim_data_item in enumerate(clim_data_items):
            if i == 0:
                continue
            dic = dict()
            cur_ssd = DEFAULT_NODATA
            cur_y = 0
            cur_m = 0
            cur_d = 0
            for j, clim_data_v in enumerate(cur_clim_data_item):
                if StringClass.string_match(clim_flds[j], DataValueFields.id):
                    dic[DataValueFields.id] = int(clim_data_v)
                elif StringClass.string_match(clim_flds[j], DataValueFields.y):
                    cur_y = int(clim_data_v)
                    dic[DataValueFields.y] = cur_y
                elif StringClass.string_match(clim_flds[j], DataValueFields.m):
                    cur_m = int(clim_data_v)
                elif StringClass.string_match(clim_flds[j], DataValueFields.d):
                    cur_d = int(clim_data_v)
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
            # Date transformation
            dt = datetime.datetime(cur_y, cur_m, cur_d, 0, 0)
            sec = time.mktime(dt.timetuple())
            utc_time = time.gmtime(sec)
            dic[DataValueFields.local_time] = dt
            dic[DataValueFields.time_zone] = time.timezone / 3600
            dic[DataValueFields.utc] = datetime.datetime(utc_time[0], utc_time[1],
                                                         utc_time[2], utc_time[3])

            # Do if some of these data are not provided
            if DataType.mean_tmp not in dic.keys():
                dic[DataType.mean_tmp] = (dic[DataType.max_tmp] + dic[DataType.min_tmp]) / 2.
            if DataType.sr not in dic.keys():
                if cur_ssd == DEFAULT_NODATA:
                    raise ValueError(DataType.sr + " or " + DataType.ssd + " must be provided!")
                else:
                    if dic[DataValueFields.id] in sites_info_dict.keys():
                        cur_lon, cur_lat = sites_info_dict[dic[DataValueFields.id]].lon_lat()
                        dic[DataType.sr] = round(HydroClimateUtilClass.rs(DateClass.day_of_year(dt),
                                                                          float(cur_ssd), cur_lat *
                                                                          math.pi / 180.), 1)
            output_flds = [DataType.mean_tmp, DataType.max_tmp, DataType.min_tmp,
                           DataType.rm, DataType.pet, DataType.ws, DataType.sr]
            for fld in output_flds:
                cur_dic = dict()
                if fld in dic.keys():
                    cur_dic[DataValueFields.value] = dic[fld]
                    cur_dic[DataValueFields.id] = dic[
                        DataValueFields.id]
                    cur_dic[DataValueFields.utc] = dic[DataValueFields.utc]
                    cur_dic[DataValueFields.time_zone] = dic[DataValueFields.time_zone]
                    cur_dic[DataValueFields.local_time] = dic[DataValueFields.local_time]
                    cur_dic[DataValueFields.type] = fld
                    curfilter = {DataValueFields.id: dic[DataValueFields.id],
                                 DataValueFields.utc: dic[DataValueFields.utc],
                                 DataValueFields.type: fld}
                    # Old code, insert or update one item a time, which is quite inefficiency
                    # Update by using bulk operation interface. lj
                    if is_first:
                        # db[DBTableNames.data_values].insert_one(cur_dic)
                        bulk.insert(cur_dic)
                    else:
                        # db[DBTableNames.data_values].find_one_and_replace(curfilter, cur_dic,
                        #                                                   upsert=True)
                        if db[DBTableNames.data_values].find(curfilter).count() != 0:
                            bulk.find(curfilter).replace_one(cur_dic)
                        else:
                            bulk.insert(cur_dic)
                    count += 1
                    if count % 500 == 0:  # execute each 500 records
                        bulk.execute()
                        bulk = db[DBTableNames.data_values].initialize_ordered_bulk_op()

            if dic[DataValueFields.id] not in hydro_climate_stats.keys():
                hydro_climate_stats[dic[DataValueFields.id]] = ClimateStats()
            hydro_climate_stats[dic[DataValueFields.id]].add_item(dic)
        # execute the remained records
        if count % 500 != 0:
            bulk.execute()
        for item, cur_climate_stats in hydro_climate_stats.items():
            cur_climate_stats.annual_stats()
        # Create index
        db[DBTableNames.data_values].create_index([(DataValueFields.id, ASCENDING),
                                                   (DataValueFields.type, ASCENDING),
                                                   (DataValueFields.utc, ASCENDING)])
        # prepare dic for MongoDB
        for s_id, stats_v in hydro_climate_stats.items():
            for YYYY in stats_v.Count.keys():
                cur_dic = dict()
                cur_dic[DataValueFields.value] = stats_v.PHUTOT[YYYY]
                cur_dic[DataValueFields.id] = s_id
                cur_dic[DataValueFields.y] = YYYY
                cur_dic[VariableDesc.unit] = "heat units"
                cur_dic[VariableDesc.type] = DataType.phu_tot
                curfilter = {DataValueFields.id: s_id,
                             VariableDesc.type: DataType.phu_tot,
                             DataValueFields.y: YYYY}
                db[DBTableNames.annual_stats].find_one_and_replace(curfilter, cur_dic,
                                                                   upsert=True)
                # import annual mean temperature
                cur_dic[VariableDesc.type] = DataType.mean_tmp
                cur_dic[VariableDesc.unit] = "deg C"
                cur_dic[DataValueFields.value] = stats_v.MeanTmp[YYYY]
                curfilter = {DataValueFields.id: s_id,
                             VariableDesc.type: DataType.mean_tmp,
                             DataValueFields.y: YYYY}
                db[DBTableNames.annual_stats].find_one_and_replace(curfilter, cur_dic,
                                                                   upsert=True)
            cur_dic[DataValueFields.value] = stats_v.PHU0
            cur_dic[DataValueFields.id] = s_id
            cur_dic[DataValueFields.y] = DEFAULT_NODATA
            cur_dic[VariableDesc.unit] = "heat units"
            cur_dic[VariableDesc.type] = DataType.phu0
            curfilter = {DataValueFields.id: s_id,
                         VariableDesc.type: DataType.phu0,
                         DataValueFields.y: DEFAULT_NODATA}
            db[DBTableNames.annual_stats].find_one_and_replace(curfilter, cur_dic,
                                                               upsert=True)
            # import annual mean temperature
            cur_dic[VariableDesc.type] = DataType.mean_tmp0
            cur_dic[VariableDesc.unit] = "deg C"
            cur_dic[DataValueFields.value] = stats_v.MeanTmp0
            curfilter = {DataValueFields.id: s_id,
                         VariableDesc.type: DataType.mean_tmp0,
                         DataValueFields.y: DEFAULT_NODATA}
            db[DBTableNames.annual_stats].find_one_and_replace(curfilter, cur_dic,
                                                               upsert=True)

    @staticmethod
    def workflow(cfg, clim_db):
        """Workflow"""
        print ("Import Daily Meteorological Data... ")
        site_m_loc = HydroClimateUtilClass.query_climate_sites(clim_db, 'M')
        c_list = clim_db.collection_names()
        tables = [DBTableNames.data_values, DBTableNames.annual_stats]
        first_import = False
        for tb in tables:
            if not StringClass.string_in_list(tb, c_list):
                clim_db.create_collection(tb)
                first_import = True
        if clim_db[DBTableNames.data_values].find({DataValueFields.type: DataType.m}).count() == 0:
            first_import = True
        ImportMeteoData.daily_data_from_txt(clim_db, cfg.Meteo_data, site_m_loc, first_import)


def main():
    """TEST CODE"""
    from seims.preprocess.config import parse_ini_configuration
    from seims.preprocess.db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    db = conn[seims_cfg.climate_db]
    import time
    st = time.time()
    ImportMeteoData.workflow(seims_cfg, db)
    et = time.time()
    print et-st
    client.close()


if __name__ == "__main__":
    main()
