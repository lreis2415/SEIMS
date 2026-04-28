"""Generate weight data for interpolate of hydroclimate data

    @author   : Liangjun Zhu, Junzhi Liu

    @changelog:
    - 16-12-07  - lj - rewrite for version 2.0
    - 17-06-26  - lj - reorganize according to pylint and google style
    - 18-02-08  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
from io import open

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from math import sqrt, pow
import copy
from struct import unpack

from gridfs import GridFS
from numpy import zeros as np_zeros

from preprocess.db_mongodb import MongoQuery
from preprocess.text import DBTableNames, RasterMetadata, FieldNames, \
    DataType, StationFields, DataValueFields, SubbsnStatsName
from preprocess.utils import dump_values, load_values, StringToPackDType
from utility import UTIL_ZERO


class ImportWeightData(object):
    """Spatial weight and its related data"""

    @staticmethod
    def cal_dis(x1, y1, x2, y2):
        """calculate distance between two points"""
        dx = x2 - x1
        dy = y2 - y1
        return sqrt(dx * dx + dy * dy)

    @staticmethod
    def idw(x, y, loc_list, fltfmt='f'):
        """IDW method for weight
        This function is not used currently"""
        ex = 2
        coef_list = list()
        sum_dist = 0
        for pt in loc_list:
            dis = ImportWeightData.cal_dis(x, y, pt[0], pt[1])
            coef = pow(dis, -ex)
            coef_list.append(coef)
            sum_dist += coef
        weight_list = []
        for coef in coef_list:
            weight_list.append(coef / sum_dist)
        # fmt = '%df' % (len(weight_list))
        # s = pack(fmt, *weight_list)
        return dump_values(weight_list, fltfmt)

    @staticmethod
    def thiessen(x, y, loc_list, fltfmt='f'):
        """Thiessen polygon method for weights"""
        i_min = 0
        coef_list = list()
        if len(loc_list) <= 1:
            coef_list.append(1)
            # fmt = '%df' % 1
            # return pack(fmt, *coef_list), i_min
            return dump_values(coef_list, fltfmt), i_min

        dis_min = ImportWeightData.cal_dis(x, y, loc_list[0][0], loc_list[0][1])

        coef_list.append(0)
        for i in range(1, len(loc_list)):
            coef_list.append(0)
            dis = ImportWeightData.cal_dis(x, y, loc_list[i][0], loc_list[i][1])
            # print(x, y, loc_list[i][0], loc_list[i][1], dis)
            if dis < dis_min:
                i_min = i
                dis_min = dis
        coef_list[i_min] = 1
        # fmt = '%df' % (len(coef_list))
        # s = pack(fmt, *coef_list)
        # return s, i_min
        return dump_values(coef_list, fltfmt), i_min

    @staticmethod
    def generate_weight_dependent_parameters(conn, maindb, subbsn_id, fltpt='FLOAT'):
        """Generate some parameters dependent on weight data and only should be calculated once.
            Such as PHU0 (annual average total potential heat units)
                TMEAN0 (annual average temperature)
        """
        spatial_gfs = GridFS(maindb, DBTableNames.gridfs_spatial)
        fltfmt = StringToPackDType(fltpt)
        # read mask file from mongodb
        mask_name = '%d_SUBBASIN' % subbsn_id
        mask_query = {'filename': mask_name, 'metadata.%s' % RasterMetadata.inc_nodata: 'TRUE'}
        # is MASK existed in Database?
        if not spatial_gfs.exists(mask_query):
            raise RuntimeError('%s is not existed in MongoDB!' % mask_name)
        mask = maindb[DBTableNames.gridfs_spatial].files.find(mask_query)[0]
        # read WEIGHT_M file from mongodb
        weight_m_name = '%d_WEIGHT_M_DAILY' % subbsn_id
        weight_m = maindb[DBTableNames.gridfs_spatial].files.find({'filename': weight_m_name})[0]
        num_cells = int(weight_m['metadata'][RasterMetadata.cellnum])
        num_sites = int(weight_m['metadata'][RasterMetadata.site_num])
        # read meteorology sites
        site_lists = maindb[DBTableNames.main_sitelist].find({FieldNames.subbasin_id: subbsn_id,
                                                              FieldNames.type: 'M',
                                                              FieldNames.mode: 'DAILY'})
        site_list = next(site_lists)
        db_name = site_list[FieldNames.db]
        m_list = site_list.get(FieldNames.list)
        hydro_clim_db = conn[db_name]

        site_list = m_list.split(',')
        site_list = [int(item) for item in site_list]

        q_dic = {StationFields.id: {'$in': site_list},
                 StationFields.type: DataType.phu0}
        cursor = hydro_clim_db[DBTableNames.annual_stats].find(q_dic).sort(StationFields.id, 1)

        q_dic2 = {StationFields.id: {'$in': site_list},
                  StationFields.type: DataType.mean_tmp0}
        cursor2 = hydro_clim_db[DBTableNames.annual_stats].find(q_dic2).sort(StationFields.id, 1)

        id_list = list()
        phu_list = list()
        for site in cursor:
            id_list.append(site[StationFields.id])
            phu_list.append(site[DataValueFields.value])

        id_list2 = list()
        tmean_list = list()
        for site in cursor2:
            id_list2.append(site[StationFields.id])
            tmean_list.append(site[DataValueFields.value])

        weight_m_data = spatial_gfs.get(weight_m['_id'])
        total_len = num_cells * num_sites
        # print(total_len)
        # fmt = '%df' % (total_len,)
        # weight_m_data = unpack(fmt, weight_m_data.read())
        weight_m_data = load_values(weight_m_data.read(), total_len, fltfmt)
        if weight_m_data is None:
            return False

        # calculate PHU0
        phu0_data = np_zeros(num_cells)
        # calculate TMEAN0
        tmean0_data = np_zeros(num_cells)
        for i in range(num_cells):
            for j in range(num_sites):
                phu0_data[i] += phu_list[j] * weight_m_data[i * num_sites + j]
                tmean0_data[i] += tmean_list[j] * weight_m_data[i * num_sites + j]
        ysize = int(mask['metadata'][RasterMetadata.nrows])
        xsize = int(mask['metadata'][RasterMetadata.ncols])
        nodata_value = mask['metadata'][RasterMetadata.nodata]
        maskgfs_data = spatial_gfs.get(mask['_id'])
        total_len = xsize * ysize  # INCLUDE_NODATA: TRUE
        # fmt = '%df' % (total_len,)
        # mask_data = unpack(fmt, maskgfs_data.read())
        mask_data = load_values(maskgfs_data.read(), total_len, dtype='i')
        if mask_data is None:
            return False
        fname = '%d_%s' % (subbsn_id, DataType.phu0)
        fname2 = '%d_%s' % (subbsn_id, DataType.mean_tmp0)
        if spatial_gfs.exists(filename=fname):
            x = spatial_gfs.get_version(filename=fname)
            spatial_gfs.delete(x._id)
        if spatial_gfs.exists(filename=fname2):
            x = spatial_gfs.get_version(filename=fname2)
            spatial_gfs.delete(x._id)
        meta_dic = copy.deepcopy(mask['metadata'])
        meta_dic['TYPE'] = DataType.phu0
        meta_dic['ID'] = fname
        meta_dic['DESCRIPTION'] = DataType.phu0
        meta_dic['INCLUDE_NODATA'] = 'FALSE'
        meta_dic['CELLSNUM'] = num_cells
        meta_dic['DATATYPE_OUT'] = fltpt

        meta_dic2 = copy.deepcopy(mask['metadata'])
        meta_dic2['TYPE'] = DataType.mean_tmp0
        meta_dic2['ID'] = fname2
        meta_dic2['DESCRIPTION'] = DataType.mean_tmp0
        meta_dic2['INCLUDE_NODATA'] = 'FALSE'
        meta_dic2['CELLSNUM'] = num_cells
        meta_dic2['DATATYPE_OUT'] = fltpt

        myfile = spatial_gfs.new_file(filename=fname, metadata=meta_dic)
        myfile2 = spatial_gfs.new_file(filename=fname2, metadata=meta_dic2)
        vaild_count = 0
        cur_row = list()
        cur_row2 = list()
        for i in range(0, ysize):
            for j in range(0, xsize):
                index = i * xsize + j
                if abs(mask_data[index] - nodata_value) > UTIL_ZERO:
                    cur_row.append(phu0_data[vaild_count])
                    cur_row2.append(tmean0_data[vaild_count])
                    vaild_count += 1
                else:
                    # cur_row.append(nodata_value)
                    # cur_row2.append(nodata_value)
                    continue
        # fmt = '%df' % vaild_count
        # myfile.write(pack(fmt, *cur_row))
        # myfile2.write(pack(fmt, *cur_row2))
        myfile.write(dump_values(cur_row, fltfmt))
        myfile2.write(dump_values(cur_row2, fltfmt))
        myfile.close()
        myfile2.close()
        print('Valid Cell Number of subbasin %d is: %d' % (subbsn_id, vaild_count))
        return True

    @staticmethod
    def climate_itp_weight_thiessen(conn, db_model, subbsn_id, geodata2dbdir, fltpt='FLOAT'):
        """Generate and import weight information using Thiessen polygon method.

        Args:
            conn:
            db_model: workflow database object
            subbsn_id: subbasin id
            geodata2dbdir: directory to store weight data as txt file
        """
        spatial_gfs = GridFS(db_model, DBTableNames.gridfs_spatial)
        fltfmt = StringToPackDType(fltpt)
        # read mask file from mongodb
        mask_name = '%d_SUBBASIN' % subbsn_id
        mask_query = {'filename': mask_name, 'metadata.%s' % RasterMetadata.inc_nodata: 'TRUE'}
        if not spatial_gfs.exists(mask_query):
            raise RuntimeError('%s is not existed in MongoDB!' % mask_name)
        mask = db_model[DBTableNames.gridfs_spatial].files.find(mask_query)[0]
        ysize = int(mask['metadata'][RasterMetadata.nrows])
        xsize = int(mask['metadata'][RasterMetadata.ncols])
        nodata_value = mask['metadata'][RasterMetadata.nodata]
        dx = mask['metadata'][RasterMetadata.cellsize]
        xll = mask['metadata'][RasterMetadata.xll]
        yll = mask['metadata'][RasterMetadata.yll]
        if 'DATATYPE_OUT' in mask['metadata']:
            dtype = mask['metadata']['DATATYPE_OUT']
            if dtype != 'INT32':
                print('The %d_SUBBASIN stored in MongoDB has wrong datatype!' % subbsn_id)
                return

        gfsdata = spatial_gfs.get(mask['_id'])

        total_len = xsize * ysize
        data = load_values(gfsdata.read(), total_len, dtype='i')
        if data is None:
            return

        # count number of valid cells
        num = 0
        for type_i in range(0, total_len):
            if abs(data[type_i] - nodata_value) > UTIL_ZERO:
                num += 1

        metadic = {RasterMetadata.subbasin: subbsn_id,
                   RasterMetadata.cellnum: num,
                   RasterMetadata.inc_nodata: 'FALSE'}
        # read stations information from database, collection SITELIST
        for site_lists in db_model[DBTableNames.main_sitelist].find(
            {FieldNames.subbasin_id: subbsn_id}):
            # print(site_lists)
            clim_db_name = site_lists[FieldNames.db]
            type_name = site_lists[FieldNames.type]
            clim_mode = site_lists[FieldNames.mode]
            hydro_clim_db = conn[clim_db_name]
            fname = '%d_WEIGHT_%s_%s' % (subbsn_id, type_name, clim_mode)
            if spatial_gfs.exists(filename=fname):
                x = spatial_gfs.get_version(filename=fname)
                spatial_gfs.delete(x._id)
            site_list_str = site_lists[FieldNames.list]
            if site_list_str is None or site_list_str == '':
                continue
            site_list = site_list_str.split(',')
            # print(site_list)
            site_list = [int(item) for item in site_list]
            metadic[RasterMetadata.site_num] = len(site_list)
            # print(site_list)
            q_dic = {StationFields.id: {'$in': site_list},
                     StationFields.type: type_name,
                     StationFields.mode: clim_mode}
            cursor = hydro_clim_db[DBTableNames.sites].find(q_dic).sort(StationFields.id, 1)
            # get site locations
            id_list = list()
            loc_list = list()
            for site in cursor:
                if site[StationFields.id] in site_list:
                    id_list.append(site[StationFields.id])
                    loc_list.append([site[StationFields.x], site[StationFields.y]])
            # print('loclist', locList)
            # interpolate using the locations
            metadic[StationFields.type] = type_name
            metadic[StationFields.mode] = clim_mode
            metadic["DATATYPE_OUT"] = fltpt
            myfile = spatial_gfs.new_file(filename=fname, metadata=metadic)
            txtfile = '%s/weight_%d_%s_%s.txt' % (geodata2dbdir, subbsn_id, type_name, clim_mode)
            with open(txtfile, 'w', encoding='utf-8') as f_test:
                for y in range(0, ysize):
                    for x in range(0, xsize):
                        index = int(y * xsize + x)
                        if abs(data[index] - nodata_value) > UTIL_ZERO:
                            x_coor = xll + x * dx
                            y_coor = yll + (ysize - y - 1) * dx
                            line, near_index = ImportWeightData.thiessen(x_coor, y_coor,
                                                                         loc_list, fltfmt)
                            myfile.write(line)
                            vals = load_values(line, len(loc_list), fltfmt)
                            f_test.write('%d %d (%s)\n' % (x, y, ','.join('%f' % v for v in vals)))
            myfile.close()
            # print(txtfile)

    @staticmethod
    def workflow(cfg, n_subbasins):
        """Workflow"""
        subbasin_start_id = 0  # default is for OpenMP version
        if n_subbasins > 0:
            subbasin_start_id = 1

        for subbsn_id in range(subbasin_start_id, n_subbasins + 1):
            ImportWeightData.climate_itp_weight_thiessen(cfg.conn, cfg.maindb, subbsn_id,
                                                         cfg.dirs.geodata2db, cfg.floattype)

            ImportWeightData.generate_weight_dependent_parameters(cfg.conn, cfg.maindb, subbsn_id,
                                                                  cfg.floattype)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    ImportWeightData.workflow(seims_cfg, 0)


if __name__ == "__main__":
    main()
