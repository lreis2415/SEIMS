"""Extract landuse parameters
    @author   : Liangjun Zhu, Junzhi Liu
    @changelog: 13-01-10  jz - initial implementation
                16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize as basic class
                17-07-07  lj - remove SQLite database file as intermediate file
                18-02-08  lj - compatible with Python3.\n
"""
from __future__ import absolute_import, unicode_literals

import logging
from io import open
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from numpy import frompyfunc as np_frompyfunc
from osgeo.gdal import GDT_Float32
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import UtilClass, MathClass, FileClass, StringClass, is_string

from utility import read_data_items_from_txt, DEFAULT_NODATA, UTIL_ZERO
from utility import mask_rasterio
from preprocess.text import ModelParamDataUtils, ParamAbstractionTypes


class LanduseUtilClass(object):
    """Landuse/Landcover related utility functions."""

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def lookup_landuse_parameters_from_mongodb(cfg):
        """Lookup landuse parameters from MongoDB.

        Returns:
            recls_dict: dict, e.g., {'MANNING': '1:0.15,2:0.15,10:0.2'}
        """
        query_result = cfg.maindb['LANDUSELOOKUP'].find()
        if query_result is None:
            raise RuntimeError('LanduseLookup Collection is not existed or empty!')
        lu_dict = dict()
        parm_dict = dict()
        for row in query_result:
            # print(row)
            value_map = dict()
            luid = -1
            for k, v in row.items():
                upperk = k.upper()
                if upperk == 'LANDUSE_ID':
                    luid = int(v)
                    continue
                if upperk not in ModelParamDataUtils.landuse_fields:
                    continue
                if is_string(v):
                    v = StringClass.extract_numeric_values_from_string(v)[0]
                value_map[upperk] = v
                if upperk not in parm_dict:
                    parm_dict[upperk] = dict()
            if luid < 0:
                continue
            lu_dict[luid] = value_map

        avail_keys = parm_dict.keys()
        for luid, param_kv in lu_dict.items():
            for param_k, param_v in param_kv.items():
                if param_k not in avail_keys:
                    continue
                parm_dict[param_k][luid] = param_v
        recls_dict = dict()
        for avail_k, lu_kv in parm_dict.items():
            recls_dict[avail_k] = ','.join('%s:%s' % (repr(k), repr(v)) for k, v in lu_kv.items())

        return recls_dict

    @staticmethod
    def reclassify_landuse_parameters(bin_dir, config_file, dst_dir, landuse_file, lookup_dir,
                                      landuse_attr_list, default_landuse_id):
        """
        Reclassify landuse parameters by lookup table.

        Deprecated: remove in next revision
        """
        # prepare reclassify configuration file
        with open(config_file, 'w', encoding='utf-8') as f_reclass_lu:
            f_reclass_lu.write('%s\t%d\n' % (landuse_file, default_landuse_id))
            f_reclass_lu.write('%s\n' % lookup_dir)
            f_reclass_lu.write(dst_dir + "\n")
            n = len(landuse_attr_list)
            f_reclass_lu.write('%d\n' % n)
            f_reclass_lu.write('\n'.join(landuse_attr_list))
        s = '"%s/reclassify" %s' % (bin_dir, config_file)
        UtilClass.run_command(s)

    @staticmethod
    def lookup_specific_landcover_parameters(cfg):
        """generate user-specific landcover related parameters"""
        # read user-specific initial parameters
        lc_data_items = read_data_items_from_txt(cfg.landcover_init_param)
        # print(lc_data_items)
        field_names = lc_data_items[0]
        lu_id = -1
        for i, v in enumerate(field_names):
            if StringClass.string_match(v, 'LANDUSE_ID'):
                lu_id = i
                break
        data_items = lc_data_items[1:]
        replace_dicts = dict()
        for item in data_items:
            for i, v in enumerate(item):
                if i == lu_id:
                    continue
                if field_names[i].upper() not in list(replace_dicts.keys()):
                    replace_dicts[field_names[i].upper()] = {int(item[lu_id]): float(v)}
                else:
                    replace_dicts[field_names[i].upper()][int(item[lu_id])] = float(v)
        # print(replace_dicts)
        for avail_k, lu_kv in replace_dicts.items():
            replace_dicts[avail_k] = ','.join('%s:%s' % (repr(k), repr(v))
                                              for k, v in lu_kv.items())
        return replace_dicts

    @staticmethod
    def read_crop_lookup_table(cfg):
        """read crop lookup table"""
        FileClass.check_file_exists(cfg.paramcfgs.crop_file)
        data_items = read_data_items_from_txt(cfg.paramcfgs.crop_file)
        attr_dic = dict()
        fields = data_items[0]
        n = len(fields)
        select_idx = list()
        for i in range(n):
            fld = fields[i].upper()
            if fld not in ModelParamDataUtils.crop_fields:
                continue
            select_idx.append(i)
            attr_dic[fld] = dict()

        for items in data_items[1:]:
            cur_id = int(items[0])
            for i in select_idx:
                dic = attr_dic[fields[i].upper()]
                try:
                    dic[cur_id] = float(items[i])
                except ValueError:
                    dic[cur_id] = items[i]
        for avail_k, lc_kv in attr_dic.items():
            attr_dic[avail_k] = ','.join('%s:%s' % (repr(k), repr(v))
                                         for k, v in lc_kv.items())
        return attr_dic

    @staticmethod
    def reclassify_landcover_parameters(landuse_file, landcover_file, landcover_initial_fields_file,
                                        landcover_lookup_file, attr_names, dst_dir):
        """reclassify landcover_init_param parameters"""
        recls_dict = LanduseUtilClass.lookup_specific_landcover_parameters(
            landcover_initial_fields_file)
        land_cover_codes = list(recls_dict['LANDCOVER'].values())
        attr_map = LanduseUtilClass.read_crop_lookup_table(landcover_lookup_file)
        n = len(attr_names)
        replace_dicts = list()
        dst_crop_tifs = list()
        for i in range(n):
            cur_attr = attr_names[i]
            cur_dict = dict()
            dic = attr_map[cur_attr]
            for code in land_cover_codes:
                if MathClass.floatequal(code, DEFAULT_NODATA):
                    continue
                if code not in list(cur_dict.keys()):
                    cur_dict[code] = dic.get(code)
            replace_dicts.append(cur_dict)
            dst_crop_tifs.append(dst_dir + os.path.sep + cur_attr + '.tif')
        # print(replace_dicts)
        # print(len(replace_dicts))
        # print(dst_crop_tifs)
        # print(len(dst_crop_tifs))
        # Generate GTIFF
        for i, v in enumerate(dst_crop_tifs):
            # print(dst_crop_tifs[i])
            RasterUtilClass.raster_reclassify(landcover_file, replace_dicts[i], v)

    @staticmethod
    def generate_cn2(maindb, landuse_file, hydrogroup_file, cn2_filename):
        """Generate CN2 raster."""
        query_result = maindb['LANDUSELOOKUP'].find()
        if query_result is None:
            raise RuntimeError("LanduseLoop Collection does not exist or is empty!")
        # cn2 list for each landuse type and hydrological soil group
        cn2_map = dict()
        for row in query_result:
            lu_id = row.get('LANDUSE_ID')
            cn2_list = [row.get('CN2A'), row.get('CN2B'), row.get('CN2C'), row.get('CN2D')]
            cn2_map[lu_id] = cn2_list
        # print(cn2Map)
        lu_r = RasterUtilClass.read_raster(landuse_file)
        data_landuse = lu_r.data
        xsize = lu_r.nCols
        ysize = lu_r.nRows
        nodata_value = lu_r.noDataValue

        hg_r = RasterUtilClass.read_raster(hydrogroup_file)
        data_hg = hg_r.data

        def cal_cn2(lucc_id, hg):
            """Calculate CN2 value from landuse ID and Hydro Group number."""
            lucc_id = int(lucc_id)
            if lucc_id < 0 or MathClass.floatequal(lucc_id, nodata_value) or hg == DEFAULT_NODATA:
                return DEFAULT_NODATA
            else:
                hg = int(hg) - 1
                if lucc_id not in cn2_map:
                    print("lucc %d does not exist in cn2 lookup table!" % lucc_id)
                    return DEFAULT_NODATA
                return cn2_map[lucc_id][hg]

        cal_cn2_numpy = np_frompyfunc(cal_cn2, 2, 1)
        data_prop = cal_cn2_numpy(data_landuse, data_hg)
        RasterUtilClass.write_gtiff_file(cn2_filename, ysize, xsize, data_prop, lu_r.geotrans,
                                         lu_r.srs, nodata_value, GDT_Float32)

    @staticmethod
    def generate_runoff_coefficient(maindb, landuse_file, slope_file, soil_texture_file,
                                    runoff_coeff_file, imper_perc=0.3):
        """Generate potential runoff coefficient."""
        # read landuselookup table from MongoDB
        prc_fields = ['PRC_ST%d' % (i,) for i in range(1, 13)]
        sc_fields = ['SC_ST%d' % (i,) for i in range(1, 13)]
        query_result = maindb['LANDUSELOOKUP'].find()
        if query_result is None:
            raise RuntimeError("LanduseLoop Collection is not existed or empty!")

        runoff_c0 = dict()
        runoff_s0 = dict()
        for row in query_result:
            tmpid = row.get('LANDUSE_ID')
            runoff_c0[tmpid] = [float(row.get(item)) for item in prc_fields]
            runoff_s0[tmpid] = [float(row.get(item)) for item in sc_fields]

        landu_raster = RasterUtilClass.read_raster(landuse_file)
        landu_data = landu_raster.data
        nodata_value1 = landu_raster.noDataValue
        xsize = landu_raster.nCols
        ysize = landu_raster.nRows
        nodata_value2 = landu_raster.noDataValue

        slo_data = RasterUtilClass.read_raster(slope_file).data
        soil_texture_obj = RasterUtilClass.read_raster(soil_texture_file)
        soil_texture_array = soil_texture_obj.data
        soil_nodata = soil_texture_obj.noDataValue
        id_omited = list()

        def coef_cal(lu_id, soil_texture, slope):
            """Calculate runoff coefficient by landuse, soil texture and slope."""
            if abs(lu_id - nodata_value1) < UTIL_ZERO or int(lu_id) < 0:
                return nodata_value2
            if int(lu_id) not in list(runoff_c0.keys()):
                if int(lu_id) not in id_omited:
                    print('The landuse ID: %d does not exist.' % int(lu_id))
                    id_omited.append(int(lu_id))
            if soil_texture == soil_nodata:
                return nodata_value2
            stid = int(soil_texture) - 1
            c0 = runoff_c0[int(lu_id)][stid]
            s0 = runoff_s0[int(lu_id)][stid] / 100.
            slp = slope

            if slp + s0 < 0.0001:
                return c0
            coef1 = (1 - c0) * slp / (slp + s0)
            coef2 = c0 + coef1
            # TODO, Check if it is (lu_id >= 98), by lj
            # special treatment for urban landuse
            if int(lu_id) == 106:
                return 0.1
            elif int(lu_id) == 107 or int(lu_id) == 105:
                return coef2 * (1 - imper_perc) + imper_perc
            else:
                return coef2

        coef_cal_numpy = np_frompyfunc(coef_cal, 3, 1)
        coef = coef_cal_numpy(landu_data, soil_texture_array, slo_data)

        RasterUtilClass.write_gtiff_file(runoff_coeff_file, ysize, xsize, coef,
                                         landu_raster.geotrans, landu_raster.srs, nodata_value2,
                                         GDT_Float32)

    @staticmethod
    def parameters_extraction(cfg):
        """Landuse spatial parameters extraction."""
        logging.info('Getting reclassification from landuse lookup tables...')
        lurecls_dict = LanduseUtilClass.lookup_landuse_parameters_from_mongodb(cfg)

        logging.info('Decomposing landuse parameters excluding nodata to MongoDB...')
        inoutcfg = list()
        for k, v in lurecls_dict.items():
            inoutcfg.append([cfg.spatials.landuse, k,
                             DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE', v])
        mongoargs = [cfg.hostname, cfg.port, cfg.spatial_db, 'SPATIAL']
        mask_rasterio(cfg.seims_bin, inoutcfg, mongoargs=mongoargs,
                      maskfile=cfg.spatials.subbsn, cfgfile=cfg.logs.reclasslu_cfg,
                      include_nodata=False, mode='MASKDEC')
        if cfg.has_conceptual_subbasin:
            mask_rasterio(cfg.seims_bin, inoutcfg, mongoargs=mongoargs,
                          maskfile=cfg.spatials.hru_subbasin_id, cfgfile=cfg.logs.reclasslu_cfg,
                          include_nodata=False, mode='MASKDEC', abstraction_type=ParamAbstractionTypes.CONCEPTUAL)

        logging.info('Getting user-specific landcover parameters...')
        lcrecls_dict = LanduseUtilClass.lookup_specific_landcover_parameters(cfg)
        logging.info('Decomposing user-specific landcover parameters to MongoDB...')
        lcinoutcfg = list()
        for k, v in lcrecls_dict.items():
            lcinoutcfg.append([cfg.spatials.landuse, k,
                               DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE', v])
        mask_rasterio(cfg.seims_bin, lcinoutcfg, mongoargs=mongoargs,
                      maskfile=cfg.spatials.subbsn, cfgfile=cfg.logs.reclasslc_cfg,
                      include_nodata=False, mode='MASKDEC')
        if cfg.has_conceptual_subbasin:
            mask_rasterio(cfg.seims_bin, lcinoutcfg, mongoargs=mongoargs,
                          maskfile=cfg.spatials.hru_subbasin_id, cfgfile=cfg.logs.reclasslc_cfg,
                          include_nodata=False, mode='MASKDEC', abstraction_type=ParamAbstractionTypes.CONCEPTUAL)

        logging.info('Getting default landcover parameters...')
        lcrecls_dict2 = LanduseUtilClass.read_crop_lookup_table(cfg)
        logging.info('Decomposing default landcover parameters to MongoDB...')
        lcinoutcfg2 = list()
        for k, v in lcrecls_dict2.items():
            lcinoutcfg2.append(['0_LANDCOVER', k, DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE', v])
        mask_rasterio(cfg.seims_bin, lcinoutcfg2, mongoargs=mongoargs,
                      maskfile=cfg.spatials.subbsn, cfgfile=cfg.logs.reclasslc_def_cfg,
                      include_nodata=False, mode='MASKDEC')
        if cfg.has_conceptual_subbasin:
            mask_rasterio(cfg.seims_bin, lcinoutcfg2, mongoargs=mongoargs,
                          maskfile=cfg.spatials.hru_subbasin_id, cfgfile=cfg.logs.reclasslc_def_cfg,
                          include_nodata=False, mode='MASKDEC', abstraction_type=ParamAbstractionTypes.CONCEPTUAL)
        # other LUCC related parameters
        # To make use of old code, we have to export some raster from MongoDB
        mask_rasterio(cfg.seims_bin,
                      [['0_HYDRO_GROUP', cfg.spatials.hydro_group],
                       ['0_SOIL_TEXTURE', cfg.spatials.soil_texture]],
                      mongoargs=mongoargs, maskfile=cfg.spatials.subbsn,
                      include_nodata=False, mode='MASK')
        # if cfg.has_conceptual_subbasins():
        #     mask_rasterio(cfg.seims_bin,
        #                   [['0_HYDRO_GROUP', cfg.spatials.hydro_group_conceptual],
        #                    ['0_SOIL_TEXTURE', cfg.spatials.soil_texture_conceptual]],
        #                   mongoargs=mongoargs, maskfile=cfg.spatials.hru_subbasin_id,
        #                   include_nodata=False, mode='MASK', abstraction_type=ParamAbstractionTypes.CONCEPTUAL)

        logging.info('Calculating Curve Number according to landuse...')
        LanduseUtilClass.generate_cn2(cfg.maindb, cfg.spatials.landuse,
                                      cfg.spatials.hydro_group, cfg.spatials.cn2)

        logging.info('Calculating potential runoff coefficient...')
        LanduseUtilClass.generate_runoff_coefficient(cfg.maindb,
                                                     cfg.spatials.landuse,
                                                     cfg.spatials.slope,
                                                     cfg.spatials.soil_texture,
                                                     cfg.spatials.runoff_coef,
                                                     cfg.imper_perc_in_urban)
        logging.info('Landuse/Landcover related spatial parameters extracted done!')


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    LanduseUtilClass.parameters_extraction(seims_cfg)


if __name__ == '__main__':
    main()
