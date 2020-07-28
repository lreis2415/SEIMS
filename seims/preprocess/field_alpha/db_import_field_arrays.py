"""Import spatial parameters corresponding to fields as GridFS to MongoDB

    @author   : Liangjun Zhu

    @changelog:
    - 18-06-08  - lj - first implementation version.
"""
from __future__ import absolute_import

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '../..')))

from pygeoc.utils import FileClass, StringClass
from gridfs import GridFS
from struct import pack

from preprocess.db_mongodb import ConnectMongoDB
from preprocess.utility import read_data_items_from_txt
from preprocess.text import DBTableNames


def combine_multi_layers_array(data_dict):
    """
    Combine multi-layers array data if existed.
    Args:
        data_dict: format: {'SOL_OM_1': [1.1, 0.9, 0.4],
                            'SOL_OM_2': [1.1, 0.9, 0.4],
                            'SOL_OM_3': [1.1, 0.9, 0.4],
                            'DEM': [100, 101, 102]
                           }

    Returns: Combined array dict which contains multi-layers data.
             format: {'SOL_OM': [[1.1, 0.9, 0.4], [1.1, 0.9, 0.4], [1.1, 0.9, 0.4]],
                      'DEM': [[100, 101, 102]]
                     }
    """
    comb_data_dict = dict()
    for key, value in list(data_dict.items()):
        key_split = key.split('_')
        if len(key_split) <= 1:
            comb_data_dict[key] = [value]
            continue
        # len(key_split) >= 2:
        try:
            pot_lyr_idx = int(key_split[-1]) - 1
            corename = key[0:key.rfind('_')]
            if pot_lyr_idx < 0:
                pot_lyr_idx = 0
            if corename not in comb_data_dict:
                comb_data_dict[corename] = list()
            comb_data_dict[corename].insert(pot_lyr_idx, value)
        except ValueError:
            comb_data_dict[key] = [value]
            continue
    return comb_data_dict


def read_field_arrays_from_csv(csvf):
    data_items = read_data_items_from_txt(csvf)
    if len(data_items) < 2:
        return
    flds = data_items[0]
    flds_array = dict()
    for idx, data_item in enumerate(data_items):
        if idx == 0:
            continue
        data_item_values = StringClass.extract_numeric_values_from_string(','.join(data_item))
        for fld_idx, fld_name in enumerate(flds):
            if fld_idx == 0 or StringClass.string_match(fld_name, 'FID'):
                continue
            if fld_name not in flds_array:
                flds_array[fld_name] = list()
            flds_array[fld_name].append(data_item_values[fld_idx])
    # for key, value in list(flds_array.items()):
    #     print('%s: %d' % (key, len(value)))
    return combine_multi_layers_array(flds_array)


def import_array_to_mongodb(gfs, array, fname):
    """
    Import array-like spatial parameters to MongoDB as GridFs
    Args:
        gfs: GridFs object
        array: format [[1,2,3], [2,2,2], [3,3,3], means an array with three layers
        fname: file name
    """
    fname = fname.upper()
    if gfs.exists(filename=fname):
        x = gfs.get_version(filename=fname)
        gfs.delete(x._id)

    rows = len(array)
    cols = len(array[0])

    # Currently, metadata is fixed.
    meta_dict = dict()
    if 'WEIGHT' in fname:
        meta_dict['NUM_SITES'] = rows
        meta_dict['NUM_CELLS'] = cols
        meta_dict['SUBBASIN'] = 9999  # Field-version
    else:
        meta_dict['TYPE'] = fname
        meta_dict['ID'] = fname
        meta_dict['DESCRIPTION'] = fname
        meta_dict['SUBBASIN'] = 9999
        meta_dict['CELLSIZE'] = 1
        meta_dict['NODATA_VALUE'] = -9999
        meta_dict['NCOLS'] = cols
        meta_dict['NROWS'] = 1
        meta_dict['XLLCENTER'] = 0
        meta_dict['YLLCENTER'] = 0
        meta_dict['LAYERS'] = rows
        meta_dict['CELLSNUM'] = cols
        meta_dict['SRS'] = ''

    myfile = gfs.new_file(filename=fname, metadata=meta_dict)
    for j in range(0, cols):
        cur_col = list()
        for i in range(0, rows):
            cur_col.append(array[i][j])
        fmt = '%df' % rows
        myfile.write(pack(fmt, *cur_col))
    myfile.close()
    print('Import %s done!' % fname)


def main():
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    db_model = conn[seims_cfg.spatial_db]

    spatial_gfs = GridFS(db_model, DBTableNames.gridfs_spatial)

    csv_path = r'C:\z_data\zhongTianShe\model_data_seims\field_scale_params'
    csv_files = FileClass.get_full_filename_by_suffixes(csv_path, ['.csv'])
    field_count = 7419
    prefix = 9999
    # Create mask file
    mask_name = '%d_MASK' % prefix
    mask_array = [[1] * field_count]
    import_array_to_mongodb(spatial_gfs, mask_array, mask_name)

    # Create spatial parameters
    for csv_file in csv_files:
        print('Import %s...' % csv_file)
        param_arrays = read_field_arrays_from_csv(csv_file)
        for key, value in list(param_arrays.items()):
            import_array_to_mongodb(spatial_gfs, value, '%d_%s' % (prefix, key))


if __name__ == "__main__":
    main()
