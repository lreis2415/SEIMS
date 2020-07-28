"""Extract landuse parameters
    @author   : Liangjun Zhu, Junzhi Liu
    @changelog: 13-01-10  jz - initial implementation
                16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize as basic class
                17-07-07  lj - remove SQLite database file as intermediate file
                18-02-08  lj - compatible with Python3.\n
"""
from __future__ import absolute_import, unicode_literals

from io import open
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from numpy import frompyfunc as np_frompyfunc
from osgeo.gdal import GDT_Float32
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import UtilClass, MathClass, FileClass, StringClass, is_string

from utility import status_output, read_data_items_from_txt, DEFAULT_NODATA, UTIL_ZERO
from preprocess.text import ModelParamDataUtils


class LanduseUtilClass(object):
    """Landuse/Landcover related utility functions."""

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def export_landuse_lookup_files_from_mongodb(cfg, maindb):
        """export landuse lookup tables to txt file from MongoDB."""
        lookup_dir = cfg.dirs.lookup
        property_namelist = ModelParamDataUtils.landuse_fields
        property_map = dict()
        property_namelist.append('USLE_P')
        query_result = maindb['LANDUSELOOKUP'].find()
        if query_result is None:
            raise RuntimeError('LanduseLoop Collection is not existed or empty!')
        count = 0
        for row in query_result:
            # print(row)
            value_map = dict()
            for i, p_name in enumerate(property_namelist):
                if StringClass.string_match(p_name, 'USLE_P'):
                    # Currently, USLE_P is set as 1 for all landuse.
                    value_map[p_name] = 1
                else:
                    # I do not know why manning * 10 here. Just uncommented now. lj
                    # if StringClass.string_match(p_name, "Manning"):
                    #     value_map[p_name] = row.get(p_name) * 10
                    # else:
                    v = row.get(p_name)
                    if is_string(v):
                        v = StringClass.extract_numeric_values_from_string(v)[0]
                    value_map[p_name] = v
            count += 1
            property_map[count] = value_map

        n = len(property_map)
        UtilClass.rmmkdir(lookup_dir)
        for propertyName in property_namelist:
            with open('%s/%s.txt' % (lookup_dir, propertyName,), 'w', encoding='utf-8') as f:
                f.write('%d\n' % n)
                for prop_id in property_map:
                    s = '%d %f\n' % (int(property_map[prop_id]['LANDUSE_ID']),
                                     property_map[prop_id][propertyName])
                    f.write('%s' % s)

    @staticmethod
    def reclassify_landuse_parameters(bin_dir, config_file, dst_dir, landuse_file, lookup_dir,
                                      landuse_attr_list, default_landuse_id):
        """
        Reclassify landuse parameters by lookup table.
        TODO(LJ): this function should be replaced by replaceByDict() function!
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
    def initialize_landcover_parameters(landcover_file, landcover_initial_fields_file, dst_dir):
        """generate initial landcover_init_param parameters"""
        lc_data_items = read_data_items_from_txt(landcover_initial_fields_file)
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
                if i != lu_id:
                    if field_names[i].upper() not in list(replace_dicts.keys()):
                        replace_dicts[field_names[i].upper()] = {float(item[lu_id]): float(v)}
                    else:
                        replace_dicts[field_names[i].upper()][float(item[lu_id])] = float(v)
        # print(replace_dicts)

        # Generate GTIFF
        for item, v in list(replace_dicts.items()):
            filename = dst_dir + os.path.sep + item + '.tif'
            print(filename)
            RasterUtilClass.raster_reclassify(landcover_file, v, filename)
        return list(replace_dicts['LANDCOVER'].values())

    @staticmethod
    def read_crop_lookup_table(crop_lookup_file):
        """read crop lookup table"""
        FileClass.check_file_exists(crop_lookup_file)
        data_items = read_data_items_from_txt(crop_lookup_file)
        attr_dic = dict()
        fields = data_items[0]
        n = len(fields)
        for i in range(n):
            attr_dic[fields[i]] = dict()
        for items in data_items[1:]:
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
        """reclassify landcover_init_param parameters"""
        land_cover_codes = LanduseUtilClass.initialize_landcover_parameters(
                landuse_file, landcover_initial_fields_file, dst_dir)
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
            raise RuntimeError("LanduseLoop Collection is not existed or empty!")
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
            if lucc_id < 0 or MathClass.floatequal(lucc_id, nodata_value):
                return DEFAULT_NODATA
            else:
                hg = int(hg) - 1
                if lucc_id not in cn2_map:
                    print("lucc %d not existed in cn2 lookup table!" % lucc_id)
                    return DEFAULT_NODATA
                return cn2_map[lucc_id][hg]

        cal_cn2_numpy = np_frompyfunc(cal_cn2, 2, 1)
        data_prop = cal_cn2_numpy(data_landuse, data_hg)
        RasterUtilClass.write_gtiff_file(cn2_filename, ysize, xsize, data_prop, lu_r.geotrans,
                                         lu_r.srs, nodata_value, GDT_Float32)

    @staticmethod
    def generate_runoff_coefficent(maindb, landuse_file, slope_file, soil_texture_file,
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
        soil_texture_array = RasterUtilClass.read_raster(soil_texture_file).data
        id_omited = list()

        def coef_cal(lu_id, soil_texture, slope):
            """Calculate runoff coefficient by landuse, soil texture and slope."""
            if abs(lu_id - nodata_value1) < UTIL_ZERO or int(lu_id) < 0:
                return nodata_value2
            if int(lu_id) not in list(runoff_c0.keys()):
                if int(lu_id) not in id_omited:
                    print('The landuse ID: %d does not exist.' % int(lu_id))
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

        coef_cal_numpy = np_frompyfunc(coef_cal, 3, 1)
        coef = coef_cal_numpy(landu_data, soil_texture_array, slo_data)

        RasterUtilClass.write_gtiff_file(runoff_coeff_file, ysize, xsize, coef,
                                         landu_raster.geotrans, landu_raster.srs, nodata_value2,
                                         GDT_Float32)

    @staticmethod
    def parameters_extraction(cfg, maindb):
        """Landuse spatial parameters extraction."""
        f = cfg.logs.extract_lu
        # 1. Generate landuse lookup tables
        status_output("Generating landuse lookup tables from MongoDB...", 10, f)
        LanduseUtilClass.export_landuse_lookup_files_from_mongodb(cfg, maindb)
        # 2. Reclassify landuse parameters by lookup tables
        status_output("Generating landuse attributes...", 20, f)
        lookup_lu_config_file = cfg.logs.reclasslu_cfg
        LanduseUtilClass.reclassify_landuse_parameters(cfg.seims_bin, lookup_lu_config_file,
                                                       cfg.dirs.geodata2db,
                                                       cfg.spatials.landuse, cfg.dirs.lookup,
                                                       ModelParamDataUtils.landuse_fields,
                                                       cfg.default_landuse)
        # 3. Generate crop parameters
        status_output("Generating crop/landcover_init_param attributes...", 30, f)
        crop_lookup_file = cfg.paramcfgs.crop_file
        LanduseUtilClass.reclassify_landcover_parameters(cfg.spatials.landuse,
                                                         cfg.spatials.crop,
                                                         cfg.landcover_init_param,
                                                         crop_lookup_file,
                                                         ModelParamDataUtils.crop_fields,
                                                         cfg.dirs.geodata2db)
        # 4. Generate Curve Number according to landuse
        status_output("Calculating CN numbers...", 40, f)
        hg_file = cfg.spatials.hydro_group
        cn2_filename = cfg.spatials.cn2
        LanduseUtilClass.generate_cn2(maindb, cfg.spatials.landuse, hg_file, cn2_filename)
        # 5. Generate runoff coefficient
        status_output("Calculating potential runoff coefficient...", 50, f)
        slope_file = cfg.spatials.slope
        soil_texture_raster = cfg.spatials.soil_texture
        runoff_coef_file = cfg.spatials.runoff_coef
        LanduseUtilClass.generate_runoff_coefficent(maindb, cfg.spatials.landuse, slope_file,
                                                    soil_texture_raster,
                                                    runoff_coef_file, cfg.imper_perc_in_urban)
        status_output("Landuse/Landcover related spatial parameters extracted done!", 100, f)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    from preprocess.db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    main_db = conn[seims_cfg.spatial_db]

    LanduseUtilClass.parameters_extraction(seims_cfg, main_db)

    client.close()


if __name__ == '__main__':
    main()
