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

from gridfs import GridFS
from numpy import unique
from pygeoc.hydro import FlowModelConst
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import StringClass, DEFAULT_NODATA, MathClass
from pymongo import ASCENDING, InsertOne

from utility import read_data_items_from_txt
from preprocess.db_mongodb import MongoUtil
from preprocess.utils import dump_values, StringToPackDType
from preprocess.text import ModelParamFields, ModelParamDataUtils, \
    DBTableNames, SubbsnStatsName, ModelCfgFields


def read_output_item(output_fields, item):
    file_out_dict = dict()
    for i, v in enumerate(output_fields):
        if StringClass.string_match(ModelCfgFields.mod_cls, v):
            file_out_dict[ModelCfgFields.mod_cls] = item[i]
        elif StringClass.string_match(ModelCfgFields.output_id, v):
            file_out_dict[ModelCfgFields.output_id] = item[i].upper()
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


def parse_calibrated_parameter_item(item):
    """Parse one param.cali row into a PARAMETERS_SPEC document."""
    valid_changes = [ModelParamFields.change_vc, ModelParamFields.change_ac,
                     ModelParamFields.change_rc, ModelParamFields.change_nc]
    if not item or len(item) < 2:
        raise RuntimeError('param.cali MUST contain at least NAME and VALUE!')

    pname = item[0].strip().upper()
    if pname == ModelParamFields.name:
        return None

    value_idx = 1
    change = ModelParamFields.change_vc
    max_idx = None
    min_idx = None
    if len(item) >= 3 and item[1].strip().upper() in valid_changes:
        change = item[1].strip().upper()
        value_idx = 2
        max_idx = 3
        min_idx = 4
    else:
        if len(item) >= 3 and item[2].strip().upper() in valid_changes:
            change = item[2].strip().upper()
            max_idx = 3
            min_idx = 4

    try:
        value = float(item[value_idx])
    except (TypeError, ValueError):
        raise RuntimeError('Invalid calibrated value for parameter %s: %s' %
                           (pname, item[value_idx]))

    data_import = {
        ModelParamFields.name: pname,
        ModelParamFields.change: change,
    }
    if change == ModelParamFields.change_nc:
        data_import[ModelParamFields.impact] = DEFAULT_NODATA
    else:
        data_import[ModelParamFields.impact] = value
    if change == ModelParamFields.change_vc:
        data_import[ModelParamFields.value] = value
    if max_idx is not None and len(item) > max_idx:
        try:
            data_import[ModelParamFields.max] = float(item[max_idx])
        except (TypeError, ValueError):
            pass
    if min_idx is not None and len(item) > min_idx:
        try:
            data_import[ModelParamFields.min] = float(item[min_idx])
        except (TypeError, ValueError):
            pass
    return data_import


class ImportParam2Mongo(object):
    """Import model parameters to MongoDB,
       including default parameters, model configuration information, etc.
    """

    @staticmethod
    def initial_params_from_txt(cfg):
        """
        import initial calibration parameters from txt data file.
        Args:
            cfg: SEIMS config object
        """
        # delete if existed, initialize if not existed
        c_list = cfg.maindb.list_collection_names()
        if not StringClass.string_in_list(DBTableNames.main_parameter, c_list):
            cfg.maindb.create_collection(DBTableNames.main_parameter)
        else:
            cfg.maindb.drop_collection(DBTableNames.main_parameter)
        # read initial parameters from txt file
        data_items = read_data_items_from_txt(cfg.paramcfgs.init_params_file)
        field_names = data_items[0][0:]
        # print(field_names)
        bulk_requests = list()
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
                           ModelParamFields.type: '',
                           ModelParamFields.dtype: ''}
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
                        data_import[k] = DEFAULT_NODATA  # Be careful to check NODATA when used!
                else:
                    if MathClass.isnumerical(cur_data_item[idx]):
                        data_import[k] = float(cur_data_item[idx])
                    else:
                        data_import[k] = cur_data_item[idx]

            bulk_requests.append(InsertOne(data_import))
        # execute import operators
        results = MongoUtil.run_bulk_write(cfg.maindb[DBTableNames.main_parameter], bulk_requests)
        print('Inserted %d initial parameters!' % (results.inserted_count
                                                   if results is not None else 0))
        # initialize index by parameter's type and name by ascending order.
        cfg.maindb[DBTableNames.main_parameter].create_index([(ModelParamFields.type, ASCENDING),
                                                              (ModelParamFields.name, ASCENDING)])

    @staticmethod
    def subbasin_statistics(cfg):
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
                   ModelParamFields.type: 'SUBBASIN',
                   ModelParamFields.dtype: 'INT'}
            curfilter = {ModelParamFields.name: dic[ModelParamFields.name]}
            # print(dic, curfilter)
            cfg.maindb[DBTableNames.main_parameter].find_one_and_replace(curfilter, dic,
                                                                         upsert=True)
        cfg.maindb[DBTableNames.main_parameter].create_index(ModelParamFields.name)

    @staticmethod
    def model_initial_outputs(cfg):
        """
        Import initial output items of SEIMS
        Args:
            cfg: SEIMS config object
        """
        file_out_path = cfg.paramcfgs.init_outputs_file
        # initialize if collection FILE_OUT not existed
        c_list = cfg.maindb.list_collection_names()
        if not StringClass.string_in_list(DBTableNames.main_fileout, c_list):
            cfg.maindb.create_collection(DBTableNames.main_fileout)
        else:
            cfg.maindb.drop_collection(DBTableNames.main_fileout)
        # begin to import initial outputs settings
        file_out_items = read_data_items_from_txt(file_out_path)
        out_field_array = file_out_items[0]
        # print(out_data_array)

        insert_requests = list()
        for idx, iitem in enumerate(file_out_items):
            if idx == 0:
                continue
            iitem_dict = read_output_item(out_field_array, iitem)
            insert_requests.append(InsertOne(iitem_dict))
        results = MongoUtil.run_bulk_write(cfg.maindb[DBTableNames.main_fileout], insert_requests)
        print('Inserted %d initial outputs settings!' % (results.inserted_count
                                                         if results is not None else 0))

    @staticmethod
    def lookup_tables_as_collection_and_gridfs(cfg):
        """Import lookup tables (from txt file) as Collection and GridFS
        Args:
            cfg: SEIMS config object
            maindb: workflow model database
        """
        fltfmt = StringToPackDType(cfg.floattype)  # 'f' or 'd'
        for tablename, txt_file in list(cfg.paramcfgs.lookup_tabs_dict.items()):
            # import each lookup table as a collection and GridFS file.
            c_list = cfg.maindb.list_collection_names()
            if not StringClass.string_in_list(tablename.upper(), c_list):
                cfg.maindb.create_collection(tablename.upper())
            else:
                cfg.maindb.drop_collection(tablename.upper())

            # delete if the tablename gridfs file existed
            spatial = GridFS(cfg.maindb, DBTableNames.gridfs_spatial)
            if spatial.exists(filename=tablename.upper()):
                x = spatial.get_version(filename=tablename.upper())
                spatial.delete(x._id)

            # read data items
            data_items = read_data_items_from_txt(txt_file)
            field_names = data_items[0][0:]
            item_values = list()  # import as gridfs file
            insert_requests = list()
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
                insert_requests.append(InsertOne(data_import))
                if len(item_value) > 0:
                    item_values.append(item_value)
            res = MongoUtil.run_bulk_write(cfg.maindb[tablename.upper()], insert_requests)
            print('Inserted %d items of %s!' % (res.inserted_count if res is not None else 0,
                                                tablename))
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
                           ModelParamDataUtils.field_count: n_col,
                           'DATATYPE_OUT': cfg.floattype}
                cur_lookup_gridfs = spatial.new_file(filename=tablename.upper(), metadata=metadic)
                header = [n_row]
                cur_lookup_gridfs.write(dump_values(header, fltfmt))
                for i in range(n_row):
                    cur_lookup_gridfs.write(dump_values(item_values[i], fltfmt))
                cur_lookup_gridfs.close()

    @staticmethod
    def calibrated_params_from_txt(cfg):
        """
        Import calibrated parameters from a text file (e.g., param.cali) to MongoDB.
        This function is refactored from run_seims.py to separate data import from model execution.

        Args:
            cfg: SEIMS configuration object containing database connections and file paths.
        """
        # 1. Get database connection from the config object
        maindb = cfg.maindb
        # Access the collection for initial parameters (used to reset impacts)
        coll = maindb[DBTableNames.main_parameter]

        # 2. Clean up existing calibration settings in the main parameter table
        # The initial PARAMETERS table should not retain modification impacts from previous runs.
        # We reset the 'impact' field to default values based on the change type.
        print('Resetting impact factors in main parameters table...')
        # For Value Change (VC), reset impact to -9999 (NODATA)
        coll.update_many({ModelParamFields.change: ModelParamFields.change_vc},
                         {'$set': {ModelParamFields.impact: -9999.}})
        # For Relative Change (RC), reset impact to 1.0 (no change multiplier)
        coll.update_many({ModelParamFields.change: ModelParamFields.change_rc},
                         {'$set': {ModelParamFields.impact: 1.}})
        # For Absolute Change (AC), reset impact to 0.0 (no addition)
        coll.update_many({ModelParamFields.change: ModelParamFields.change_ac},
                         {'$set': {ModelParamFields.impact: 0.}})

        # 3. Determine Configuration Name and Task Name
        # These are used to tag the calibrated parameters in the database.
        # If specific names are not provided in the config, use defaults.
        cur_cfg = ModelCfgFields.configname_default
        if hasattr(cfg.paramcfgs, 'configname') and cfg.paramcfgs.configname:
            cur_cfg = cfg.paramcfgs.configname

        cur_task = ModelCfgFields.taskname_default
        if hasattr(cfg.paramcfgs, 'taskname') and cfg.paramcfgs.taskname:
            cur_task = cfg.paramcfgs.taskname

        # 4. Prepare the Parameter Specification Collection
        # This collection (main_param_spec) stores the specific calibration rules.
        param_spec_coll = maindb[DBTableNames.main_param_spec]

        # Delete any existing records for the current configuration and task
        # to ensure a clean import.
        param_spec_coll.delete_many({ModelCfgFields.configname: cur_cfg,
                                     ModelCfgFields.taskname: cur_task})

        # Create a unique index to prevent duplicate entries for the same parameter
        param_spec_coll.create_index([(ModelCfgFields.configname, 1),
                                      (ModelCfgFields.taskname, 1),
                                      (ModelParamFields.name, 1)], unique=True)

        # 5. Read the Calibration Text File
        model_dir = cfg.model_dir if hasattr(cfg, 'model_dir') else ''
        sub_folder = 'storm'
        cali_file_path = os.path.join(model_dir, sub_folder, 'param.cali')
        if not os.path.exists(cali_file_path):
            fallback_path = os.path.join(model_dir, 'param.cali')
            if os.path.exists(fallback_path):
                print(f"Warning: File not found in '{sub_folder}', falling back to: {fallback_path}")
                cali_file_path = fallback_path
            else:
                print(f"Error: Calibration file not found at {cali_file_path}")
                return

        data_items = read_data_items_from_txt(cali_file_path)
        insert_requests = list()

        # Iterate through the rows in the text file
        for i, cur_data_item in enumerate(data_items):
            try:
                data_import = parse_calibrated_parameter_item(cur_data_item)
            except RuntimeError as err:
                print(f"Skipping invalid line {i}: {err}")
                continue
            if data_import is None:
                continue

            # Add configuration metadata
            data_import[ModelCfgFields.configname] = cur_cfg
            data_import[ModelCfgFields.taskname] = cur_task

            # Add to the bulk request list
            insert_requests.append(InsertOne(data_import))

        # 6. Execute Bulk Write to MongoDB
        if insert_requests:
            results = MongoUtil.run_bulk_write(param_spec_coll, insert_requests)
            print('Inserted %d calibration parameters for config: %s!' %
                  (results.inserted_count if results is not None else 0, cur_cfg))
        else:
            print('No valid calibration parameters found to import.')

    @staticmethod
    def workflow(cfg):
        """Workflow"""
        ImportParam2Mongo.initial_params_from_txt(cfg)
        ImportParam2Mongo.calibrated_params_from_txt(cfg)
        ImportParam2Mongo.model_initial_outputs(cfg)
        ImportParam2Mongo.subbasin_statistics(cfg)
        ImportParam2Mongo.lookup_tables_as_collection_and_gridfs(cfg)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    ImportParam2Mongo.workflow(seims_cfg)


if __name__ == "__main__":
    main()
