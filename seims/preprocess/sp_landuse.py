#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Extract landuse parameters
    @author   : Liangjun Zhu, Junzhi Liu
    @changelog: 13-01-10  jz - initial implementation
                16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize as basic class
"""
import os
import re
import sqlite3

import numpy
from gdal import GDT_Float32

from seims.preprocess.db_sqlite import reconstruct_sqlite_db_file
from seims.preprocess.utility import status_output, read_data_items_from_txt, \
    DEFAULT_NODATA, UTIL_ZERO
from seims.pygeoc.pygeoc.raster.raster import RasterUtilClass
from seims.pygeoc.pygeoc.utils.utils import UtilClass, MathClass, FileClass, StringClass


class LanduseUtilClass(object):
    """Landuse/Landcover related utility functions."""
    # CROP, LANDUSE, and SOIL attribute are imported to mongoDB
    # Match to the new lookup table of SWAT 2012 rev.637. lj
    _CROP_ATTR_LIST = ["IDC", "BIO_E", "HVSTI", "BLAI", "FRGRW1", "LAIMX1", "FRGRW2",
                       "LAIMX2", "DLAI", "CHTMX", "RDMX", "T_OPT", "T_BASE", "CNYLD",
                       "CPYLD", "BN1", "BN2", "BN3", "BP1", "BP2", "BP3", "WSYF",
                       "USLE_C", "GSI", "VPDFR", "FRGMAX", "WAVP", "CO2HI", "BIOEHI",
                       "RSDCO_PL", "OV_N", "CN2A", "CN2B", "CN2C", "CN2D", "FERTFIELD",
                       "ALAI_MIN", "BIO_LEAF", "MAT_YRS", "BMX_TREES", "EXT_COEF", "BM_DIEOFF"]
    _LANDUSE_ATTR_LIST = ["CN2A", "CN2B", "CN2C", "CN2D", "ROOTDEPTH", "MANNING",
                          "INTERC_MAX", "INTERC_MIN", "SHC", "SOIL_T10",
                          "PET_FR", "PRC_ST1", "PRC_ST2", "PRC_ST3", "PRC_ST4",
                          "PRC_ST5", "PRC_ST6", "PRC_ST7", "PRC_ST8", "PRC_ST9",
                          "PRC_ST10", "PRC_ST11", "PRC_ST12", "SC_ST1", "SC_ST2",
                          "SC_ST3", "SC_ST4", "SC_ST5", "SC_ST6", "SC_ST7", "SC_ST8",
                          "SC_ST9", "SC_ST10", "SC_ST11", "SC_ST12", "DSC_ST1", "DSC_ST2",
                          "DSC_ST3", "DSC_ST4", "DSC_ST5", "DSC_ST6", "DSC_ST7", "DSC_ST8",
                          "DSC_ST9", "DSC_ST10", "DSC_ST11", "DSC_ST12"]

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def export_landuse_lookup_files(sqlite3_dbname, property_namelist, str_sql, dst_dir):
        """export landuse lookup tables to txt file from SQLite database.
        Args:
            sqlite3_dbname: SQLite file
            property_namelist: properties be exported
            str_sql: sql query sentence
            dst_dir: lookup tables directory
        """
        property_map = {}
        conn = sqlite3.connect(sqlite3_dbname)
        cursor = conn.cursor()

        cursor.execute(str_sql)
        property_namelist.append("USLE_P")
        for row in cursor:
            # print row
            prop_id = int(row[0])
            value_map = {}
            for i, p_name in enumerate(property_namelist):
                if p_name == "USLE_P":  # Currently, USLE_P is set as 1 for all landuse.
                    value_map[p_name] = 1
                else:
                    if p_name == "Manning":
                        value_map[p_name] = row[i + 1] * 10
                    else:
                        value_map[p_name] = row[i + 1]
            property_map[prop_id] = value_map

        n = len(property_map)
        UtilClass.rmmkdir(dst_dir)
        for propertyName in property_namelist:
            f = open("%s/%s.txt" % (dst_dir, propertyName,), 'w')
            f.write("%d\n" % n)
            for prop_id in property_map:
                s = "%d %f\n" % (prop_id, property_map[prop_id][propertyName])
                f.write(s)
            f.close()

    @staticmethod
    def reclassify_landuse_parameters(bin_dir, config_file, dst_dir, landuse_file, lookup_dir,
                                      landuse_attr_list, default_landuse_id):
        """
        Reclassify landuse parameters by lookup table.
        TODO(LJ): this function should be replaced by replaceByDict() function!
        """
        # prepare reclassify configuration file
        f_reclass_lu = open(config_file, 'w')
        f_reclass_lu.write("%s\t%d\n" % (landuse_file, default_landuse_id))
        f_reclass_lu.write("%s\n" % lookup_dir)
        f_reclass_lu.write(dst_dir + "\n")
        n = len(landuse_attr_list)
        f_reclass_lu.write("%d\n" % n)
        f_reclass_lu.write("\n".join(landuse_attr_list))
        f_reclass_lu.close()
        s = '"%s/reclassify" %s' % (bin_dir, config_file)
        UtilClass.run_command(s)

    @staticmethod
    def initialize_landcover_parameters(landcover_file, landcover_initial_fields_file, dst_dir):
        """generate initial landcover_init_param parameters"""
        lc_data_items = read_data_items_from_txt(landcover_initial_fields_file)
        # print lc_data_items
        field_names = lc_data_items[0]
        lu_id = -1
        for i, v in enumerate(field_names):
            if StringClass.string_match(v, 'LANDUSE_ID'):
                lu_id = i
                break
        data_items = lc_data_items[1:]
        replace_dicts = {}
        for item in data_items:
            for i, v in enumerate(item):
                if i != lu_id:
                    if field_names[i].upper() not in replace_dicts.keys():
                        replace_dicts[field_names[i].upper()] = {float(item[lu_id]): float(v)}
                    else:
                        replace_dicts[field_names[i].upper()][float(item[lu_id])] = float(v)
        # print replace_dicts

        # Generate GTIFF
        for item, v in replace_dicts.items():
            filename = dst_dir + os.sep + item + '.tif'
            print (filename)
            RasterUtilClass.raster_reclassify(landcover_file, v, filename)
        return replace_dicts['LANDCOVER'].values()

    @staticmethod
    def read_crop_lookup_table(crop_lookup_file):
        """read crop lookup table"""
        FileClass.check_file_exists(crop_lookup_file)
        f = open(crop_lookup_file)
        lines = f.readlines()
        f.close()
        attr_dic = {}
        fields = [item.replace('"', '')
                  for item in re.split('\t|\n|\r\n|\r', lines[0]) if item is not '']
        n = len(fields)
        for i in range(n):
            attr_dic[fields[i]] = {}
        for line in lines[2:]:
            items = [item.replace('"', '')
                     for item in re.split('\t', line) if item is not '']
            cur_id = int(items[0])

            for i in range(n):
                dic = attr_dic[fields[i]]
                try:
                    dic[cur_id] = float(items[i])
                except ValueError:
                    dic[cur_id] = items[i]
        return attr_dic

    @staticmethod
    def reclassify_landcover_parameters(landuse_file, landcover_file, landcover_initial_fields_file,
                                        landcover_lookup_file, attr_names, dst_dir):
        """relassify landcover_init_param parameters"""
        land_cover_codes = LanduseUtilClass.initialize_landcover_parameters(
                landuse_file, landcover_initial_fields_file, dst_dir)
        attr_map = LanduseUtilClass.read_crop_lookup_table(landcover_lookup_file)
        n = len(attr_names)
        replace_dicts = []
        dst_crop_tifs = []
        for i in range(n):
            cur_attr = attr_names[i]
            cur_dict = {}
            dic = attr_map[cur_attr]
            for code in land_cover_codes:
                if code == DEFAULT_NODATA:
                    continue
                if code not in cur_dict.keys():
                    cur_dict[code] = dic[code]
            replace_dicts.append(cur_dict)
            dst_crop_tifs.append(dst_dir + os.sep + cur_attr + '.tif')
        # print replace_dicts
        # print(len(replace_dicts))
        # print dst_crop_tifs
        # print(len(dst_crop_tifs))
        # Generate GTIFF
        for i, v in enumerate(dst_crop_tifs):
            # print dst_crop_tifs[i]
            RasterUtilClass.raster_reclassify(landcover_file, replace_dicts[i], v)

    @staticmethod
    def generate_cn2(dbname, landuse_file, hydrogroup_file, cn2_filename):
        """Generate CN2 raster."""
        str_sql_lu = 'select LANDUSE_ID, CN2A, CN2B, CN2C, CN2D from LanduseLookup'
        conn = sqlite3.connect(dbname)
        cursor = conn.cursor()
        # cn2 list for each landuse type and hydrological soil group
        cn2_map = {}
        cursor.execute(str_sql_lu)
        for row in cursor:
            lu_id = int(row[0])
            cn2_list = []
            for i in range(4):
                cn2_list.append(float(row[i + 1]))
            cn2_map[lu_id] = cn2_list
        # print cn2Map
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
            if lucc_id < 0 or MathClass.floatequal(lucc_id, nodata_value):
                return DEFAULT_NODATA
            else:
                hg = int(hg) - 1
                return cn2_map[lucc_id][hg]

        cal_cn2_numpy = numpy.frompyfunc(cal_cn2, 2, 1)
        data_prop = cal_cn2_numpy(data_landuse, data_hg)
        RasterUtilClass.write_gtiff_file(cn2_filename, ysize, xsize, data_prop, lu_r.geotrans,
                                         lu_r.srs, nodata_value, GDT_Float32)

    @staticmethod
    def generate_runoff_coefficent(sqlite_file, landuse_file, slope_file, soil_texture_file,
                                   runoff_coeff_file, imper_perc=0.3):
        """Generate potential runoff coefficient."""
        # read landuselookup table from sqlite
        prc_fields = ["PRC_ST%d" % (i,) for i in range(1, 13)]
        sc_fields = ["SC_ST%d" % (i,) for i in range(1, 13)]
        sql_landuse = 'select LANDUSE_ID, %s, %s from LanduseLookup' % (
            ','.join(prc_fields), ','.join(sc_fields))

        conn = sqlite3.connect(sqlite_file)
        cursor = conn.cursor()
        cursor.execute(sql_landuse)

        runoff_c0 = {}
        runoff_s0 = {}
        for row in cursor:
            tmpid = int(row[0])
            runoff_c0[tmpid] = [float(item) for item in row[1:13]]
            runoff_s0[tmpid] = [float(item) for item in row[13:25]]

        cursor.close()
        conn.close()
        # end read data

        landu_raster = RasterUtilClass.read_raster(landuse_file)
        landu_data = landu_raster.data
        nodata_value1 = landu_raster.noDataValue
        xsize = landu_raster.nCols
        ysize = landu_raster.nRows
        nodata_value2 = landu_raster.noDataValue

        slo_data = RasterUtilClass.read_raster(slope_file).data
        soil_texture_array = RasterUtilClass.read_raster(soil_texture_file).data
        id_omited = []

        def coef_cal(lu_id, soil_texture, slope):
            """Calculate runoff coefficient by landuse, soil texture and slope."""
            if abs(lu_id - nodata_value1) < UTIL_ZERO or int(lu_id) < 0:
                return nodata_value2
            if int(lu_id) not in runoff_c0.keys():
                if int(lu_id) not in id_omited:
                    print ('The landuse ID: %d does not exist.' % int(lu_id))
                    id_omited.append(int(lu_id))
            stid = int(soil_texture) - 1
            c0 = runoff_c0[int(lu_id)][stid]
            s0 = runoff_s0[int(lu_id)][stid] / 100.
            slp = slope

            if slp + s0 < 0.0001:
                return c0
            coef1 = (1 - c0) * slp / (slp + s0)
            coef2 = c0 + coef1
            # TODO, Check if it is (lu_id >= 98), by lj
            if int(lu_id) == 106 or int(lu_id) == 107 or int(lu_id) == 105:
                return coef2 * (1 - imper_perc) + imper_perc
            else:
                return coef2

        coef_cal_numpy = numpy.frompyfunc(coef_cal, 3, 1)
        coef = coef_cal_numpy(landu_data, soil_texture_array, slo_data)

        RasterUtilClass.write_gtiff_file(runoff_coeff_file, ysize, xsize, coef,
                                         landu_raster.geotrans, landu_raster.srs, nodata_value2,
                                         GDT_Float32)

    @staticmethod
    def parameters_extraction(cfg):
        """Landuse spatial parameters extraction."""
        f = open(cfg.logs.extract_soil, 'w')
        # 1. Generate landuse lookup tables
        status_output("Generating landuse lookup tables from Sqlite database...", 10, f)
        str_sql = 'select landuse_id, ' + ','.join(LanduseUtilClass._LANDUSE_ATTR_LIST) \
                  + ' from LanduseLookup'
        sqlite3db = cfg.sqlitecfgs.sqlite_file
        if not FileClass.is_file_exists(sqlite3db):
            reconstruct_sqlite_db_file(cfg)
        lookup_dir = cfg.dirs.lookup
        LanduseUtilClass.export_landuse_lookup_files(sqlite3db, LanduseUtilClass._LANDUSE_ATTR_LIST,
                                                     str_sql, lookup_dir)
        # 2. Reclassify landuse parameters by lookup tables
        status_output("Generating landuse attributes...", 20, f)
        lookup_lu_config_file = cfg.logs.reclasslu_cfg
        LanduseUtilClass.reclassify_landuse_parameters(cfg.seims_bin, lookup_lu_config_file,
                                                       cfg.dirs.geodata2db,
                                                       cfg.landuse, lookup_dir,
                                                       LanduseUtilClass._LANDUSE_ATTR_LIST,
                                                       cfg.default_landuse)
        # 3. Generate crop parameters
        if cfg.gen_crop:
            status_output("Generating crop/landcover_init_param attributes...", 30, f)
            crop_lookup_file = cfg.sqlitecfgs.crop_file
            LanduseUtilClass.reclassify_landcover_parameters(cfg.spatials.landuse,
                                                             cfg.spatials.crop,
                                                             cfg.landcover_init_param,
                                                             crop_lookup_file,
                                                             LanduseUtilClass._CROP_ATTR_LIST,
                                                             cfg.dirs.geodata2db)
        # 4. Generate Curve Number according to landuse
        if cfg.gen_cn:
            status_output("Calculating CN numbers...", 40, f)
            hg_file = cfg.spatials.hydro_group
            cn2_filename = cfg.spatials.cn2
            LanduseUtilClass.generate_cn2(sqlite3db, cfg.spatials.landuse, hg_file, cn2_filename)
        # 5. Generate runoff coefficient
        if cfg.gen_runoff_coef:
            status_output("Calculating potential runoff coefficient...", 50, f)
            slope_file = cfg.spatials.slope
            soil_texture_raster = cfg.spatials.soil_texture
            runoff_coef_file = cfg.spatials.runoff_coef
            LanduseUtilClass.generate_runoff_coefficent(sqlite3db, cfg.spatials.landuse, slope_file,
                                                        soil_texture_raster,
                                                        runoff_coef_file, cfg.imper_perc_in_urban)
        status_output("Landuse/Landcover related spatial parameters extracted done!", 100, f)
        f.close()


def main():
    """TEST CODE"""
    from seims.preprocess.config import parse_ini_configuration
    seims_cfg = parse_ini_configuration()
    LanduseUtilClass.parameters_extraction(seims_cfg)


if __name__ == '__main__':
    main()
