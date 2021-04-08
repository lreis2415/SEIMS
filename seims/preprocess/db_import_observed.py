"""Import measurement data, such as discharge, sediment yield, and nutrient export etc.

    @author   : Liangjun Zhu, Fang Shen

    @changelog:
    - 16-12-07  - lj - rewrite for version 2.0
    - 17-06-26  - lj - reorganize according to pylint and google style
    - 17-07-05  - lj - Using bulk operation interface to improve MongoDB efficiency.
    - 17-08-05  - lj - Add Timezone preprocessor statement in the first line of data file.
    - 18-02-08  - lj - compatible with Python3.

    @TODO:
    - Check the location of observed stations and add subbasinID field.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
from datetime import timedelta

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.raster import RasterUtilClass
from pygeoc.utils import StringClass, FileClass

from utility import read_data_items_from_txt
from preprocess.db_mongodb import MongoUtil, MongoQuery
from preprocess.hydro_climate_utility import HydroClimateUtilClass
from preprocess.text import StationFields, DBTableNames, DataValueFields, SubbsnStatsName


class ImportObservedData(object):
    """
    Import observed values for current model. The procedure including several steps:
        1. Read monitor station information, filter by LocalX and LocalY coordinates,
           and store variables information (siteDic) and station IDs (siteIDs)
        2. Read observed data and import to MongoDB
        3. Add observed data with unit converted
    """

    @staticmethod
    def match_subbasin(subbsn_file, site_dict, maindb):
        """
        Match the ID of subbasin
            1. Read the coordinates of each subbasin's outlet, and
               the outlet ID of the whole basin (not finished yet)
            2. If the isOutlet field equals to
               2.1 - 0, then return the subbasin_id of the site's location
               2.2 - 1, then return the outlet ID of the whole basiin
               2.3 - 2, then return the outlet ID of nearest subbasin
               2.4 - 3, then return the outlet IDs of the conjunct subbasins
        """
        subbasin_raster = RasterUtilClass.read_raster(subbsn_file)
        localx = site_dict.get(StationFields.x)
        localy = site_dict.get(StationFields.y)
        site_type = site_dict.get(StationFields.outlet)
        subbasin_id = subbasin_raster.get_value_by_xy(localx, localy)
        if subbasin_id is None and site_type != 1:
            # the site is not inside the basin and not the outlet either.
            return False, None
        if site_type == 0:
            return True, [subbasin_id]
        elif site_type == 1:
            outid = int(MongoQuery.get_init_parameter_value(maindb, SubbsnStatsName.outlet))
            return True, [outid]
        elif site_type == 2:
            return True, [subbasin_id]  # TODO

    @staticmethod
    def data_from_txt(maindb, hydro_clim_db, obs_txts_list, sites_info_txts_list, subbsn_file):
        """
        Read observed data from txt file
        Args:
            maindb: Main spatial database
            hydro_clim_db: hydro-climate dababase
            obs_txts_list: txt file paths of observed data
            sites_info_txts_list: txt file paths of site information
            subbsn_file: subbasin raster file

        Returns:
            True or False
        """
        # 1. Read monitor station information, and store variables information and station IDs
        variable_lists = []
        site_ids = []
        for site_file in sites_info_txts_list:
            site_data_items = read_data_items_from_txt(site_file)
            site_flds = site_data_items[0]
            for i in range(1, len(site_data_items)):
                dic = dict()
                types = list()
                units = list()
                for j, v in enumerate(site_data_items[i]):
                    if StringClass.string_match(site_flds[j], StationFields.id):
                        dic[StationFields.id] = int(v)
                        site_ids.append(dic[StationFields.id])
                    elif StringClass.string_match(site_flds[j], StationFields.name):
                        dic[StationFields.name] = v.strip()
                    elif StringClass.string_match(site_flds[j], StationFields.type):
                        types = StringClass.split_string(v.strip(), '-')
                    elif StringClass.string_match(site_flds[j], StationFields.lat):
                        dic[StationFields.lat] = float(v)
                    elif StringClass.string_match(site_flds[j], StationFields.lon):
                        dic[StationFields.lon] = float(v)
                    elif StringClass.string_match(site_flds[j], StationFields.x):
                        dic[StationFields.x] = float(v)
                    elif StringClass.string_match(site_flds[j], StationFields.y):
                        dic[StationFields.y] = float(v)
                    elif StringClass.string_match(site_flds[j], StationFields.unit):
                        units = StringClass.split_string(v.strip(), '-')
                    elif StringClass.string_match(site_flds[j], StationFields.elev):
                        dic[StationFields.elev] = float(v)
                    elif StringClass.string_match(site_flds[j], StationFields.outlet):
                        dic[StationFields.outlet] = float(v)

                for j, cur_type in enumerate(types):
                    site_dic = dict()
                    site_dic[StationFields.id] = dic[StationFields.id]
                    site_dic[StationFields.name] = dic[StationFields.name]
                    site_dic[StationFields.type] = cur_type
                    site_dic[StationFields.lat] = dic[StationFields.lat]
                    site_dic[StationFields.lon] = dic[StationFields.lon]
                    site_dic[StationFields.x] = dic[StationFields.x]
                    site_dic[StationFields.y] = dic[StationFields.y]
                    site_dic[StationFields.unit] = units[j]
                    site_dic[StationFields.elev] = dic[StationFields.elev]
                    site_dic[StationFields.outlet] = dic[StationFields.outlet]
                    # Add SubbasinID field
                    matched, cur_sids = ImportObservedData.match_subbasin(subbsn_file, site_dic,
                                                                          maindb)
                    if not matched:
                        break
                    if len(cur_sids) == 1:  # if only one subbasin ID, store integer
                        cur_subbsn_id_str = cur_sids[0]
                    else:
                        cur_subbsn_id_str = ','.join(str(cid) for cid in cur_sids
                                                     if cur_sids is None)
                    site_dic[StationFields.subbsn] = cur_subbsn_id_str
                    curfilter = {StationFields.id: site_dic[StationFields.id],
                                 StationFields.type: site_dic[StationFields.type]}
                    # print(curfilter)
                    hydro_clim_db[DBTableNames.sites].find_one_and_replace(curfilter, site_dic,
                                                                           upsert=True)

                    var_dic = dict()
                    var_dic[StationFields.type] = types[j]
                    var_dic[StationFields.unit] = units[j]
                    if var_dic not in variable_lists:
                        variable_lists.append(var_dic)
        site_ids = list(set(site_ids))
        # 2. Read measurement data and import to MongoDB
        bulk = hydro_clim_db[DBTableNames.observes].initialize_ordered_bulk_op()
        count = 0
        for measDataFile in obs_txts_list:
            # print(measDataFile)
            obs_data_items = read_data_items_from_txt(measDataFile)
            tsysin, tzonein = HydroClimateUtilClass.get_time_system_from_data_file(measDataFile)
            # If the data items is EMPTY or only have one header row, then goto
            # next data file.
            if obs_data_items == [] or len(obs_data_items) == 1:
                continue
            obs_flds = obs_data_items[0]
            required_flds = [StationFields.id, DataValueFields.type, DataValueFields.value]

            for fld in required_flds:
                if not StringClass.string_in_list(fld, obs_flds):  # data can not meet the request!
                    raise ValueError('The %s can not meet the required format!' % measDataFile)
            for i, cur_obs_data_item in enumerate(obs_data_items):
                dic = dict()
                if i == 0:
                    continue
                for j, cur_data_value in enumerate(cur_obs_data_item):
                    if StringClass.string_match(obs_flds[j], StationFields.id):
                        dic[StationFields.id] = int(cur_data_value)
                        # if current site ID is not included, goto next data item
                        if dic[StationFields.id] not in site_ids:
                            continue
                    elif StringClass.string_match(obs_flds[j], DataValueFields.type):
                        dic[DataValueFields.type] = cur_data_value
                    elif StringClass.string_match(obs_flds[j], DataValueFields.value):
                        dic[DataValueFields.value] = float(cur_data_value)
                utc_t = HydroClimateUtilClass.get_utcdatetime_from_field_values(obs_flds,
                                                                                cur_obs_data_item,
                                                                                tsysin, tzonein)
                dic[DataValueFields.local_time] = utc_t - timedelta(minutes=tzonein * 60)
                dic[DataValueFields.time_zone] = tzonein
                dic[DataValueFields.utc] = utc_t
                # curfilter = {StationFields.id: dic[StationFields.id],
                #              DataValueFields.type: dic[DataValueFields.type],
                #              DataValueFields.utc: dic[DataValueFields.utc]}
                # bulk.find(curfilter).replace_one(dic)
                bulk.insert(dic)
                count += 1
                if count % 500 == 0:
                    MongoUtil.run_bulk(bulk)
                    bulk = hydro_clim_db[DBTableNames.observes].initialize_ordered_bulk_op()
                    # db[DBTableNames.observes].find_one_and_replace(curfilter, dic, upsert=True)
        if count % 500 != 0:
            MongoUtil.run_bulk(bulk)
        # 3. Add measurement data with unit converted
        # loop variables list
        added_dics = list()
        for curVar in variable_lists:
            # print(curVar)
            # if the unit is mg/L, then change the Type name with the suffix 'Conc',
            # and convert the corresponding data to kg if the discharge data is
            # available.
            cur_type = curVar[StationFields.type]
            cur_unit = curVar[StationFields.unit]
            # Find data by Type
            for item in hydro_clim_db[DBTableNames.observes].find({StationFields.type: cur_type}):
                # print(item)
                dic = dict()
                dic[StationFields.id] = item[StationFields.id]
                dic[DataValueFields.value] = item[DataValueFields.value]
                dic[StationFields.type] = item[StationFields.type]
                dic[DataValueFields.local_time] = item[DataValueFields.local_time]
                dic[DataValueFields.time_zone] = item[DataValueFields.time_zone]
                dic[DataValueFields.utc] = item[DataValueFields.utc]

                if cur_unit == 'mg/L' or cur_unit == 'g/L':
                    # update the Type name
                    dic[StationFields.type] = '%sConc' % cur_type
                    curfilter = {StationFields.id: dic[StationFields.id],
                                 DataValueFields.type: cur_type,
                                 DataValueFields.utc: dic[DataValueFields.utc]}
                    hydro_clim_db[DBTableNames.observes].find_one_and_replace(curfilter, dic,
                                                                              upsert=True)
                    dic[StationFields.type] = cur_type

                # find discharge on current day
                cur_filter = {StationFields.type: 'Q',
                              DataValueFields.utc: dic[DataValueFields.utc],
                              StationFields.id: dic[StationFields.id]}
                q_dic = hydro_clim_db[DBTableNames.observes].find_one(filter=cur_filter)

                if q_dic is not None:
                    q = q_dic[DataValueFields.value]
                else:
                    continue
                if cur_unit == 'mg/L':
                    # convert mg/L to kg
                    dic[DataValueFields.value] = round(dic[DataValueFields.value] *
                                                       q * 86400. / 1000., 2)
                elif cur_unit == 'g/L':
                    # convert g/L to kg
                    dic[DataValueFields.value] = round(dic[DataValueFields.value] * q * 86400., 2)
                elif cur_unit == 'kg':
                    dic[StationFields.type] = '%sConc' % cur_type
                    # convert kg to mg/L
                    dic[DataValueFields.value] = round(dic[DataValueFields.value]
                                                       / q * 1000. / 86400., 2)
                # add new data item
                added_dics.append(dic)
        # import to MongoDB
        for dic in added_dics:
            curfilter = {StationFields.id: dic[StationFields.id],
                         DataValueFields.type: dic[DataValueFields.type],
                         DataValueFields.utc: dic[DataValueFields.utc]}
            hydro_clim_db[DBTableNames.observes].find_one_and_replace(curfilter, dic, upsert=True)

    @staticmethod
    def workflow(cfg, maindb, climdb):
        """
        This function mainly to import measurement data to MongoDB
        data type may include Q (discharge, m3/s), SED (mg/L), TN (mg/L), TP (mg/L), etc.
        the required parameters that defined in configuration file (*.ini)
        """
        if not cfg.use_observed:
            return False
        c_list = climdb.collection_names()
        if not StringClass.string_in_list(DBTableNames.observes, c_list):
            climdb.create_collection(DBTableNames.observes)
        else:
            climdb.drop_collection(DBTableNames.observes)
        if not StringClass.string_in_list(DBTableNames.sites, c_list):
            climdb.create_collection(DBTableNames.sites)
        if not StringClass.string_in_list(DBTableNames.var_desc, c_list):
            climdb.create_collection(DBTableNames.var_desc)

        file_list = FileClass.get_full_filename_by_suffixes(cfg.observe_dir, ['.txt', '.csv'])
        meas_file_list = list()
        site_loc = list()
        for fl in file_list:
            if StringClass.is_substring('observed_', fl):
                meas_file_list.append(fl)
            else:
                site_loc.append(fl)
        ImportObservedData.data_from_txt(maindb, climdb, meas_file_list, site_loc,
                                         cfg.spatials.subbsn)
        return True


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    from preprocess.db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    main_db = conn[seims_cfg.spatial_db]
    hydroclim_db = conn[seims_cfg.climate_db]
    import time
    st = time.time()
    ImportObservedData.workflow(seims_cfg, main_db, hydroclim_db)
    et = time.time()
    print(et - st)

    client.close()


if __name__ == "__main__":
    main()
