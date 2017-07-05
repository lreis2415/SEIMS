#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Import model calibration parameters, model configuration information etc.
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize as basic class
"""
import os
import sqlite3

from numpy import unique

from seims.preprocess.db_sqlite import reconstruct_sqlite_db_file
from seims.preprocess.text import SQLiteParaUtils, ModelParamFields, \
    DBTableNames, SubbsnStatsName, ModelCfgFields
from seims.preprocess.utility import read_data_items_from_txt
from seims.pygeoc.pygeoc.hydro.hydro import FlowModelConst
from seims.pygeoc.pygeoc.raster.raster import RasterUtilClass
from seims.pygeoc.pygeoc.utils.utils import FileClass, StringClass, DEFAULT_NODATA


class ImportParam2Mongo(object):
    """Import model parameters to MongoDB,
       including default parameters, model configuration information, etc.
    """

    @staticmethod
    def initial_params_from_sqlite(cfg, maindb):
        """
        import initial calibration parameters from SQLite database file.
        Args:
            cfg: SEIMS config object
            maindb: MongoDB database object
        """
        sqlite_file = cfg.sqlitecfgs.sqlite_file
        if FileClass.is_file_exists(sqlite_file):
            os.remove(sqlite_file)
        reconstruct_sqlite_db_file(cfg)
        # delete if existed, create if not existed
        c_list = maindb.collection_names()
        if not StringClass.string_in_list(DBTableNames.main_parameter, c_list):
            maindb.create_collection(DBTableNames.main_parameter)
        else:
            maindb.drop_collection(DBTableNames.main_parameter)
        # read sqlite database
        sqlite_conn = sqlite3.connect(sqlite_file)
        sql_cursor = sqlite_conn.cursor()
        # get all the table_name
        sql_cursor.execute("select name from sqlite_master where type='table' order by name;")
        table_list = sql_cursor.fetchall()
        # Find parameter table list excluding "XXLookup"
        table_list = [item[0].encode("ascii") for item in table_list if (
            item[0].lower().find(SQLiteParaUtils.Tag_Lookup) < 0)]
        # print table_list
        field_list = [ModelParamFields.name, ModelParamFields.desc,
                      ModelParamFields.unit, ModelParamFields.module,
                      ModelParamFields.value, ModelParamFields.impact,
                      ModelParamFields.change, ModelParamFields.max,
                      ModelParamFields.min, ModelParamFields.use]
        for table_name in table_list:
            # print table_name
            str_sql = "select * from %s;" % (table_name,)
            cur = sql_cursor.execute(str_sql)
            records = cur.fetchall()
            for items in records:
                dic = dict()
                dic[ModelParamFields.type] = table_name

                for i, v in enumerate(items):
                    # if type(items[i]) == type('a') or type(items[i]) == type(u'a'):  # bad style
                    if isinstance(v, str) or isinstance(v, unicode):
                        dic[field_list[i]] = v.encode('ascii')
                    else:
                        dic[field_list[i]] = v
                curfilter = {ModelParamFields.name: dic[ModelParamFields.name],
                             ModelParamFields.type: table_name}
                maindb[DBTableNames.main_parameter].find_one_and_replace(curfilter, dic,
                                                                         upsert=True)
        maindb[DBTableNames.main_parameter].create_index(ModelParamFields.name)
        sql_cursor.close()
        sqlite_conn.close()

    @staticmethod
    def subbasin_statistics(cfg, maindb):
        """
        Import subbasin numbers, outlet ID, etc. to MongoDB.
        """
        streamlink_r = cfg.spatials.stream_link
        flowdir_r = cfg.spatials.d8flow
        direction_items = dict()
        if cfg.is_TauDEM:
            direction_items = FlowModelConst.get_cell_shift("TauDEM")
        else:
            direction_items = FlowModelConst.get_cell_shift("ArcGIS")
        streamlink_d = RasterUtilClass.read_raster(streamlink_r)
        nodata = streamlink_d.noDataValue
        nrows = streamlink_d.nRows
        ncols = streamlink_d.nCols
        streamlink_data = streamlink_d.data
        max_subbasin_id = int(streamlink_d.get_max())
        min_subbasin_id = int(streamlink_d.get_min())
        subbasin_num = len(unique(streamlink_data)) - 1
        # print max_subbasin_id, min_subbasin_id, subbasin_num
        flowdir_d = RasterUtilClass.read_raster(flowdir_r)
        flowdir_data = flowdir_d.data
        i_row = -1
        i_col = -1
        for row in range(nrows):
            for col in range(ncols):
                if streamlink_data[row][col] != nodata:
                    i_row = row
                    i_col = col
                    # print row, col
                    break
            else:
                continue
            break
        if i_row == -1 or i_col == -1:
            raise ValueError("Stream link data invalid, please check and retry.")

        def flow_down_stream_idx(dir_value, i, j):
            """Return row and col of downstream direction."""
            drow, dcol = direction_items[int(dir_value)]
            return i + drow, j + dcol

        def find_outlet_index(r, c):
            """Find outlet's coordinate"""
            flag = True
            while flag:
                fdir = flowdir_data[r][c]
                newr, newc = flow_down_stream_idx(fdir, r, c)
                if newr < 0 or newc < 0 or newr >= nrows or newc >= ncols \
                        or streamlink_data[newr][newc] == nodata:
                    flag = False
                else:
                    # print newr, newc, streamlink_data[newr][newc]
                    r = newr
                    c = newc
            return r, c

        o_row, o_col = find_outlet_index(i_row, i_col)
        outlet_bsn_id = int(streamlink_data[o_row][o_col])
        import_stats_dict = {SubbsnStatsName.outlet: outlet_bsn_id,
                             SubbsnStatsName.o_row: o_row,
                             SubbsnStatsName.o_col: o_col,
                             SubbsnStatsName.subbsn_max: max_subbasin_id,
                             SubbsnStatsName.subbsn_min: min_subbasin_id,
                             SubbsnStatsName.subbsn_num: subbasin_num}

        for stat, stat_v in import_stats_dict.items():
            dic = {ModelParamFields.name: stat,
                   ModelParamFields.desc: stat,
                   ModelParamFields.unit: "NONE",
                   ModelParamFields.module: "ALL",
                   ModelParamFields.value: stat_v,
                   ModelParamFields.impact: DEFAULT_NODATA,
                   ModelParamFields.change: ModelParamFields.change_nc,
                   ModelParamFields.max: DEFAULT_NODATA,
                   ModelParamFields.min: DEFAULT_NODATA,
                   ModelParamFields.use: ModelParamFields.use_y,
                   ModelParamFields.type: "SUBBASIN"}
            curfilter = {ModelParamFields.name: dic[ModelParamFields.name]}
            # print (dic, curfilter)
            maindb[DBTableNames.main_parameter].find_one_and_replace(curfilter, dic,
                                                                     upsert=True)
        maindb[DBTableNames.main_parameter].create_index(ModelParamFields.name)

    @staticmethod
    def model_io_configuration(cfg, maindb):
        """
        Import Input and Output Configuration of SEIMS, i.e., file.in and file.out
        Args:
            cfg: SEIMS config object
            maindb: MongoDB database object
        """
        file_in_path = cfg.modelcfgs.filein
        file_out_path = cfg.modelcfgs.fileout
        # create if collection not existed
        c_list = maindb.collection_names()
        conf_tabs = [DBTableNames.main_filein, DBTableNames.main_fileout]
        for item in conf_tabs:
            if not StringClass.string_in_list(item, c_list):
                maindb.create_collection(item)
            else:
                maindb.drop_collection(item)
        file_in_items = read_data_items_from_txt(file_in_path)
        file_out_items = read_data_items_from_txt(file_out_path)

        for item in file_in_items:
            file_in_dict = dict()
            values = StringClass.split_string(StringClass.strip_string(item[0]), ['|'])
            if len(values) != 2:
                raise ValueError("One item should only have one Tag and one value string,"
                                 " split by '|'")
            file_in_dict[ModelCfgFields.tag] = values[0]
            file_in_dict[ModelCfgFields.value] = values[1]
            maindb[DBTableNames.main_filein].find_one_and_replace(file_in_dict, file_in_dict,
                                                                  upsert=True)

        out_field_array = file_out_items[0]
        out_data_array = file_out_items[1:]
        # print out_data_array
        for item in out_data_array:
            file_out_dict = dict()
            for i, v in enumerate(out_field_array):
                if StringClass.string_match(ModelCfgFields.mod_cls, v):
                    file_out_dict[ModelCfgFields.mod_cls] = item[i]
                elif StringClass.string_match(ModelCfgFields.output_id, v):
                    file_out_dict[ModelCfgFields.output_id] = item[i]
                elif StringClass.string_match(ModelCfgFields.desc, v):
                    file_out_dict[ModelCfgFields.desc] = item[i]
                elif StringClass.string_match(ModelCfgFields.unit, v):
                    file_out_dict[ModelCfgFields.unit] = item[i]
                elif StringClass.string_match(ModelCfgFields.type, v):
                    file_out_dict[ModelCfgFields.type] = item[i]
                elif StringClass.string_match(ModelCfgFields.stime, v):
                    file_out_dict[ModelCfgFields.stime] = item[i]
                elif StringClass.string_match(ModelCfgFields.etime, v):
                    file_out_dict[ModelCfgFields.etime] = item[i]
                elif StringClass.string_match(ModelCfgFields.interval, v):
                    file_out_dict[ModelCfgFields.interval] = item[i]
                elif StringClass.string_match(ModelCfgFields.interval_unit, v):
                    file_out_dict[ModelCfgFields.interval_unit] = item[i]
                elif StringClass.string_match(ModelCfgFields.filename, v):
                    file_out_dict[ModelCfgFields.filename] = item[i]
                elif StringClass.string_match(ModelCfgFields.use, v):
                    file_out_dict[ModelCfgFields.use] = item[i]
                elif StringClass.string_match(ModelCfgFields.subbsn, v):
                    file_out_dict[ModelCfgFields.subbsn] = item[i]
            if file_out_dict.keys() is []:
                raise ValueError("There are not any valid output item stored in file.out!")
            cur_flt = {ModelCfgFields.mod_cls: file_out_dict[ModelCfgFields.mod_cls],
                       ModelCfgFields.output_id: file_out_dict[ModelCfgFields.output_id],
                       ModelCfgFields.stime: file_out_dict[ModelCfgFields.stime],
                       ModelCfgFields.etime: file_out_dict[ModelCfgFields.etime]}
            maindb[DBTableNames.main_fileout].find_one_and_replace(cur_flt, file_out_dict,
                                                                   upsert=True)

    @staticmethod
    def workflow(cfg, maindb):
        """Workflow"""
        ImportParam2Mongo.initial_params_from_sqlite(cfg, maindb)
        ImportParam2Mongo.model_io_configuration(cfg, maindb)
        ImportParam2Mongo.subbasin_statistics(cfg, maindb)


def main():
    """TEST CODE"""
    from seims.preprocess.config import parse_ini_configuration
    from seims.preprocess.db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    main_db = conn[seims_cfg.spatial_db]

    ImportParam2Mongo.workflow(seims_cfg, main_db)

    client.close()


if __name__ == "__main__":
    main()
