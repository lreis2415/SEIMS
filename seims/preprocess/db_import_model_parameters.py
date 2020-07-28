"""Import model calibration parameters, model configuration information etc.

    @author   : Liangjun Zhu

    @changelog:
    - 16-12-07  lj - rewrite for version 2.0
    - 17-06-23  lj - reorganize as basic class
    - 18-01-30  lj - clean up calibration settings before import a new one
    - 18-02-08  lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from struct import pack

from gridfs import GridFS
from numpy import unique
from pygeoc.hydro import FlowModelConst
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import StringClass, DEFAULT_NODATA, MathClass
from pymongo import ASCENDING

from utility import read_data_items_from_txt
from preprocess.db_mongodb import MongoUtil
from preprocess.text import ModelParamFields, ModelParamDataUtils, \
    DBTableNames, SubbsnStatsName, ModelCfgFields


class ImportParam2Mongo(object):
    """Import model parameters to MongoDB,
       including default parameters, model configuration information, etc.
    """

    @staticmethod
    def initial_params_from_txt(cfg, maindb):
        """
        import initial calibration parameters from txt data file.
        Args:
            cfg: SEIMS config object
            maindb: MongoDB database object
        """
        # delete if existed, initialize if not existed
        c_list = maindb.collection_names()
        if not StringClass.string_in_list(DBTableNames.main_parameter, c_list):
            maindb.create_collection(DBTableNames.main_parameter)
        else:
            maindb.drop_collection(DBTableNames.main_parameter)
        # initialize bulk operator
        bulk = maindb[DBTableNames.main_parameter].initialize_ordered_bulk_op()
        # read initial parameters from txt file
        data_items = read_data_items_from_txt(cfg.paramcfgs.init_params_file)
        field_names = data_items[0][0:]
        # print(field_names)
        for i, cur_data_item in enumerate(data_items):
            if i == 0:
                continue
            # print(cur_data_item)
            # initial one default blank parameter dict.
            data_import = {ModelParamFields.name: '', ModelParamFields.desc: '',
                           ModelParamFields.unit: '', ModelParamFields.module: '',
                           ModelParamFields.value: DEFAULT_NODATA,
                           ModelParamFields.impact: DEFAULT_NODATA,
                           ModelParamFields.change: 'NC',
                           ModelParamFields.max: DEFAULT_NODATA,
                           ModelParamFields.min: DEFAULT_NODATA,
                           ModelParamFields.type: ''}
            for k, v in list(data_import.items()):
                idx = field_names.index(k)
                if cur_data_item[idx] == '':
                    if StringClass.string_match(k, ModelParamFields.change_ac):
                        data_import[k] = 0
                    elif StringClass.string_match(k, ModelParamFields.change_rc):
                        data_import[k] = 1
                    elif StringClass.string_match(k, ModelParamFields.change_nc):
                        data_import[k] = 0
                    elif StringClass.string_match(k, ModelParamFields.change_vc):
                        data_import[k] = DEFAULT_NODATA  # Be careful to check NODATA when use!
                else:
                    if MathClass.isnumerical(cur_data_item[idx]):
                        data_import[k] = float(cur_data_item[idx])
                    else:
                        data_import[k] = cur_data_item[idx]
            bulk.insert(data_import)
        # execute import operators
        MongoUtil.run_bulk(bulk, 'No operation during initial_params_from_txt.')
        # initialize index by parameter's type and name by ascending order.
        maindb[DBTableNames.main_parameter].create_index([(ModelParamFields.type, ASCENDING),
                                                          (ModelParamFields.name, ASCENDING)])

    @staticmethod
    def calibrated_params_from_txt(cfg, maindb):
        """Read and update calibrated parameters."""
        # initialize bulk operator
        coll = maindb[DBTableNames.main_parameter]
        bulk = coll.initialize_ordered_bulk_op()
        # read initial parameters from txt file
        data_items = read_data_items_from_txt(cfg.modelcfgs.filecali)
        # print(field_names)
        # Clean up the existing calibration settings
        coll.update_many({ModelParamFields.change: ModelParamFields.change_vc},
                         {'$set': {ModelParamFields.impact: -9999.}})
        coll.update_many({ModelParamFields.change: ModelParamFields.change_rc},
                         {'$set': {ModelParamFields.impact: 1.}})
        coll.update_many({ModelParamFields.change: ModelParamFields.change_ac},
                         {'$set': {ModelParamFields.impact: 0.}})
        for i, cur_data_item in enumerate(data_items):
            data_import = dict()
            cur_filter = dict()
            if len(cur_data_item) < 2:
                raise RuntimeError('param.cali at least contain NAME and IMPACT fields!')
            data_import[ModelParamFields.name] = cur_data_item[0]
            data_import[ModelParamFields.impact] = float(cur_data_item[1])
            cur_filter[ModelParamFields.name] = cur_data_item[0]
            if len(cur_data_item) >= 3:
                if cur_data_item[2] in [ModelParamFields.change_vc, ModelParamFields.change_ac,
                                        ModelParamFields.change_rc, ModelParamFields.change_nc]:
                    data_import[ModelParamFields.change] = cur_data_item[2]

            bulk.find(cur_filter).update({'$set': data_import})
        # execute import operators
        MongoUtil.run_bulk(bulk, 'No operations during calibrated_params_from_txt.')

    @staticmethod
    def subbasin_statistics(cfg, maindb):
        """
        Import subbasin numbers, outlet ID, etc. to MongoDB.
        """
        streamlink_r = cfg.spatials.stream_link
        flowdir_r = cfg.spatials.d8flow
        direction_items = dict()
        # Flow direction follows ArcGIS rule, which has been converted from TauDEM in
        #    sd_delineation.post_process_of_delineated_data()
        direction_items = FlowModelConst.get_cell_shift('ArcGIS')
        streamlink_d = RasterUtilClass.read_raster(streamlink_r)
        nodata = streamlink_d.noDataValue
        nrows = streamlink_d.nRows
        ncols = streamlink_d.nCols
        streamlink_data = streamlink_d.data
        max_subbasin_id = int(streamlink_d.get_max())
        min_subbasin_id = int(streamlink_d.get_min())
        subbasin_num = len(unique(streamlink_data)) - 1
        # print(max_subbasin_id, min_subbasin_id, subbasin_num)
        flowdir_d = RasterUtilClass.read_raster(flowdir_r)
        flowdir_data = flowdir_d.data
        i_row = -1
        i_col = -1
        for row in range(nrows):
            for col in range(ncols):
                if streamlink_data[row][col] != nodata:
                    i_row = row
                    i_col = col
                    # print(row, col)
                    break
            else:
                continue
            break
        if i_row == -1 or i_col == -1:
            raise ValueError('Stream link data invalid, please check and retry.')

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
                    # print(newr, newc, streamlink_data[newr][newc])
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

        for stat, stat_v in list(import_stats_dict.items()):
            dic = {ModelParamFields.name: stat,
                   ModelParamFields.desc: stat,
                   ModelParamFields.unit: 'NONE',
                   ModelParamFields.module: 'ALL',
                   ModelParamFields.value: stat_v,
                   ModelParamFields.impact: DEFAULT_NODATA,
                   ModelParamFields.change: ModelParamFields.change_nc,
                   ModelParamFields.max: DEFAULT_NODATA,
                   ModelParamFields.min: DEFAULT_NODATA,
                   ModelParamFields.type: 'SUBBASIN'}
            curfilter = {ModelParamFields.name: dic[ModelParamFields.name]}
            # print(dic, curfilter)
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
        file_out_path = cfg.paramcfgs.init_outputs_file
        # initialize if collection not existed
        c_list = maindb.collection_names()
        conf_tabs = [DBTableNames.main_filein, DBTableNames.main_fileout]
        for item in conf_tabs:
            if not StringClass.string_in_list(item, c_list):
                maindb.create_collection(item)
            else:
                maindb.drop_collection(item)
        file_in_items = read_data_items_from_txt(file_in_path)

        for item in file_in_items:
            file_in_dict = dict()
            values = StringClass.split_string(item[0].strip(), ['|'])
            if len(values) != 2:
                raise ValueError('One item should only have one Tag and one value string,'
                                 ' split by "|"')
            file_in_dict[ModelCfgFields.tag] = values[0]
            file_in_dict[ModelCfgFields.value] = values[1]
            maindb[DBTableNames.main_filein].insert(file_in_dict)

        # begin to import initial outputs settings
        file_out_items = read_data_items_from_txt(file_out_path)
        bulk = maindb[DBTableNames.main_fileout].initialize_unordered_bulk_op()
        out_field_array = file_out_items[0]
        # print(out_data_array)

        def read_output_item(output_fields, item):
            file_out_dict = dict()
            for i, v in enumerate(output_fields):
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
            if not list(file_out_dict.keys()):
                raise ValueError('There are not any valid output item stored in file.out!')
            return file_out_dict

        for idx, iitem in enumerate(file_out_items):
            if idx == 0:
                continue
            iitem_dict = read_output_item(out_field_array, iitem)
            bulk.insert(iitem_dict)
        MongoUtil.run_bulk(bulk, 'No operations to execute when import initial outputs settings.')

        # begin to import the desired outputs
        # initialize bulk operator
        bulk = maindb[DBTableNames.main_fileout].initialize_ordered_bulk_op()
        # read initial parameters from txt file
        data_items = read_data_items_from_txt(cfg.modelcfgs.fileout)
        # print(field_names)
        user_out_field_array = data_items[0]
        if ModelCfgFields.output_id not in user_out_field_array:
            if len(data_items[0]) != 7:  # For the compatibility of old code!
                raise RuntimeError('If header information is not provided,'
                                   'items in file.out must have 7 columns, i.e., OUTPUTID,'
                                   'TYPE,STARTTIME,ENDTIME,INTERVAL,INTERVAL_UNIT,SUBBASIN.'
                                   'Otherwise, the OUTPUTID MUST existed in the header!')
            user_out_field_array = ['OUTPUTID', 'TYPE', 'STARTTIME', 'ENDTIME', 'INTERVAL',
                                    'INTERVAL_UNIT', 'SUBBASIN']
            data_items.insert(0, user_out_field_array)

        for idx, iitem in enumerate(data_items):
            if idx == 0:
                continue
            data_import = read_output_item(user_out_field_array, iitem)
            data_import[ModelCfgFields.use] = 1
            cur_filter = dict()
            cur_filter[ModelCfgFields.output_id] = data_import[ModelCfgFields.output_id]
            bulk.find(cur_filter).update({'$set': data_import})
        # execute import operators
        MongoUtil.run_bulk(bulk, 'No operations to excute when import the desired outputs.')

    @staticmethod
    def lookup_tables_as_collection_and_gridfs(cfg, maindb):
        """Import lookup tables (from txt file) as Collection and GridFS
        Args:
            cfg: SEIMS config object
            maindb: workflow model database
        """
        for tablename, txt_file in list(cfg.paramcfgs.lookup_tabs_dict.items()):
            # import each lookup table as a collection and GridFS file.
            c_list = maindb.collection_names()
            if not StringClass.string_in_list(tablename.upper(), c_list):
                maindb.create_collection(tablename.upper())
            else:
                maindb.drop_collection(tablename.upper())
            # initial bulk operator
            bulk = maindb[tablename.upper()].initialize_ordered_bulk_op()
            # delete if the tablename gridfs file existed
            spatial = GridFS(maindb, DBTableNames.gridfs_spatial)
            if spatial.exists(filename=tablename.upper()):
                x = spatial.get_version(filename=tablename.upper())
                spatial.delete(x._id)

            # read data items
            data_items = read_data_items_from_txt(txt_file)
            field_names = data_items[0][0:]
            item_values = list()  # import as gridfs file
            for i, cur_data_item in enumerate(data_items):
                if i == 0:
                    continue
                data_import = dict()  # import as Collection
                item_value = list()  # import as gridfs file
                for idx, fld in enumerate(field_names):
                    if MathClass.isnumerical(cur_data_item[idx]):
                        tmp_value = float(cur_data_item[idx])
                        data_import[fld] = tmp_value
                        item_value.append(tmp_value)
                    else:
                        data_import[fld] = cur_data_item[idx]
                bulk.insert(data_import)
                if len(item_value) > 0:
                    item_values.append(item_value)
            MongoUtil.run_bulk(bulk, 'No operations during import %s.' % tablename)
            # begin import gridfs file
            n_row = len(item_values)
            # print(item_values)
            if n_row >= 1:
                n_col = len(item_values[0])
                for i in range(n_row):
                    if n_col != len(item_values[i]):
                        raise ValueError('Please check %s to make sure each item has '
                                         'the same numeric dimension. The size of first '
                                         'row is: %d, and the current data item is: %d' %
                                         (tablename, n_col, len(item_values[i])))
                    else:
                        item_values[i].insert(0, n_col)

                metadic = {ModelParamDataUtils.item_count: n_row,
                           ModelParamDataUtils.field_count: n_col}
                cur_lookup_gridfs = spatial.new_file(filename=tablename.upper(), metadata=metadic)
                header = [n_row]
                fmt = '%df' % 1
                s = pack(fmt, *header)
                cur_lookup_gridfs.write(s)
                fmt = '%df' % (n_col + 1)
                for i in range(n_row):
                    s = pack(fmt, *item_values[i])
                    cur_lookup_gridfs.write(s)
                cur_lookup_gridfs.close()

    @staticmethod
    def workflow(cfg, maindb):
        """Workflow"""
        ImportParam2Mongo.initial_params_from_txt(cfg, maindb)
        ImportParam2Mongo.calibrated_params_from_txt(cfg, maindb)
        ImportParam2Mongo.model_io_configuration(cfg, maindb)
        ImportParam2Mongo.subbasin_statistics(cfg, maindb)
        ImportParam2Mongo.lookup_tables_as_collection_and_gridfs(cfg, maindb)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    from preprocess.db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    main_db = conn[seims_cfg.spatial_db]

    ImportParam2Mongo.workflow(seims_cfg, main_db)

    client.close()


if __name__ == "__main__":
    main()
