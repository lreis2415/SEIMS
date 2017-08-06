#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Import model calibration parameters, model configuration information etc.
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize as basic class
"""
from struct import pack

from gridfs import GridFS
from numpy import unique
from pymongo import ASCENDING

from seims.preprocess.text import ModelParamFields, ModelParamDataUtils, \
    DBTableNames, SubbsnStatsName, ModelCfgFields
from seims.preprocess.utility import read_data_items_from_txt
from seims.pygeoc.pygeoc.hydro.hydro import FlowModelConst
from seims.pygeoc.pygeoc.raster.raster import RasterUtilClass
from seims.pygeoc.pygeoc.utils.utils import StringClass, DEFAULT_NODATA, MathClass


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
        # delete if existed, create if not existed
        c_list = maindb.collection_names()
        if not StringClass.string_in_list(DBTableNames.main_parameter, c_list):
            maindb.create_collection(DBTableNames.main_parameter)
        else:
            maindb.drop_collection(DBTableNames.main_parameter)
        # create bulk operator
        bulk = maindb[DBTableNames.main_parameter].initialize_ordered_bulk_op()
        # read initial parameters from txt file
        data_items = read_data_items_from_txt(cfg.paramcfgs.init_params_file)
        field_names = data_items[0][0:]
        # print (field_names)
        for i, cur_data_item in enumerate(data_items):
            if i == 0:
                continue
            # print cur_data_item
            # initial one default blank parameter dict.
            data_import = {ModelParamFields.name: '', ModelParamFields.desc: '',
                           ModelParamFields.unit: '', ModelParamFields.module: '',
                           ModelParamFields.value: DEFAULT_NODATA,
                           ModelParamFields.impact: DEFAULT_NODATA,
                           ModelParamFields.change: 'NC',
                           ModelParamFields.max: DEFAULT_NODATA,
                           ModelParamFields.min: DEFAULT_NODATA,
                           ModelParamFields.type: ''}
            for k, v in data_import.items():
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
        bulk.execute()
        # create index by parameter's type and name by ascending order.
        maindb[DBTableNames.main_parameter].create_index([(ModelParamFields.type, ASCENDING),
                                                          (ModelParamFields.name, ASCENDING)])

    @staticmethod
    def calibrated_params_from_txt(cfg, maindb):
        """Read and update calibrated parameters."""
        # create bulk operator
        bulk = maindb[DBTableNames.main_parameter].initialize_ordered_bulk_op()
        # read initial parameters from txt file
        data_items = read_data_items_from_txt(cfg.modelcfgs.filecali)
        # print (field_names)
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
        bulk.execute()

    @staticmethod
    def subbasin_statistics(cfg, maindb):
        """
        Import subbasin numbers, outlet ID, etc. to MongoDB.
        """
        streamlink_r = cfg.spatials.stream_link
        flowdir_r = cfg.spatials.d8flow
        direction_items = dict()
        if cfg.is_TauDEM:
            direction_items = FlowModelConst.get_cell_shift('TauDEM')
        else:
            direction_items = FlowModelConst.get_cell_shift('ArcGIS')
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
                   ModelParamFields.unit: 'NONE',
                   ModelParamFields.module: 'ALL',
                   ModelParamFields.value: stat_v,
                   ModelParamFields.impact: DEFAULT_NODATA,
                   ModelParamFields.change: ModelParamFields.change_nc,
                   ModelParamFields.max: DEFAULT_NODATA,
                   ModelParamFields.min: DEFAULT_NODATA,
                   ModelParamFields.type: 'SUBBASIN'}
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
        file_out_path = cfg.paramcfgs.init_outputs_file
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
                raise ValueError('One item should only have one Tag and one value string,'
                                 ' split by "|"')
            file_in_dict[ModelCfgFields.tag] = values[0]
            file_in_dict[ModelCfgFields.value] = values[1]
            maindb[DBTableNames.main_filein].insert(file_in_dict)

        # begin to import initial outputs settings
        bulk = maindb[DBTableNames.main_fileout].initialize_unordered_bulk_op()
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
                raise ValueError('There are not any valid output item stored in file.out!')
            bulk.insert(file_out_dict)
        bulk.execute()

        # begin to import the desired outputs
        # create bulk operator
        bulk = maindb[DBTableNames.main_fileout].initialize_ordered_bulk_op()
        # read initial parameters from txt file
        data_items = read_data_items_from_txt(cfg.modelcfgs.fileout)
        # print (field_names)
        for i, cur_data_item in enumerate(data_items):
            data_import = dict()
            cur_filter = dict()
            # print (cur_data_item)
            if len(cur_data_item) == 7:
                data_import[ModelCfgFields.output_id] = cur_data_item[0]
                data_import[ModelCfgFields.type] = cur_data_item[1]
                data_import[ModelCfgFields.stime] = cur_data_item[2]
                data_import[ModelCfgFields.etime] = cur_data_item[3]
                data_import[ModelCfgFields.interval] = cur_data_item[4]
                data_import[ModelCfgFields.interval_unit] = cur_data_item[5]
                data_import[ModelCfgFields.subbsn] = cur_data_item[6]
                data_import[ModelCfgFields.use] = 1
                cur_filter[ModelCfgFields.output_id] = cur_data_item[0]
            else:
                raise RuntimeError('Items in file.out must have 7 columns, i.e., OUTPUTID,'
                                   'TYPE,STARTTIME,ENDTIME,INTERVAL,INTERVAL_UNIT,SUBBASIN.')

            bulk.find(cur_filter).update({'$set': data_import})
        # execute import operators
        bulk.execute()

    @staticmethod
    def lookup_tables_as_collection_and_gridfs(cfg, maindb):
        """Import lookup tables (from txt file) as Collection and GridFS
        Args:
            cfg: SEIMS config object
            maindb: workflow model database
        """
        for tablename, txt_file in cfg.paramcfgs.lookup_tabs_dict.items():
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
            item_values = []  # import as gridfs file
            for i, cur_data_item in enumerate(data_items):
                if i == 0:
                    continue
                data_import = dict()  # import as Collection
                item_value = []  # import as gridfs file
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
            bulk.execute()
            # begin import gridfs file
            n_row = len(item_values)
            # print (item_values)
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
