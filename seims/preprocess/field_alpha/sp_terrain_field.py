"""Terrain related spatial parameters extraction (extend to field version of SEIMS)

    @author   : Liangjun Zhu, Junzhi Liu

    @changelog:

"""
from __future__ import absolute_import

from math import exp, sqrt
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import numpy
from osgeo.gdal import GDT_Float32
from osgeo.ogr import FieldDefn as ogr_FieldDefn
from osgeo.ogr import OFTReal
from osgeo.ogr import Open as ogr_Open
from pygeoc.hydro import FlowModelConst
from pygeoc.raster import RasterUtilClass

from preprocess.db_import_stream_parameters import ImportReaches2Mongo
from preprocess.utility import status_output, UTIL_ZERO, DEFAULT_NODATA

sys.setrecursionlimit(10000)


class TerrainUtilClass(object):
    """Terrain related spatial parameters algorithms."""

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def flow_length_cell(i, j, ysize, xsize, fdir, cellsize, weight, length,
                         flow_dir_code="TauDEM"):
        """Calculate flow length of cell."""
        celllen = FlowModelConst.get_cell_length(flow_dir_code)
        differ = FlowModelConst.get_cell_shift(flow_dir_code)
        # print(i,j, weight[i][j])
        if i < ysize and j < xsize:
            if length[i][j] == 0:
                if weight[i][j] > 0:
                    prei = i
                    prej = j
                    wt = weight[i][j]
                    fdir_v = fdir[i][j]
                    di = differ[fdir_v][0]
                    dj = differ[fdir_v][1]
                    i = i + di
                    j = j + dj
                    relen = TerrainUtilClass.flow_length_cell(i, j, ysize, xsize, fdir, cellsize,
                                                              weight, length,
                                                              flow_dir_code)
                    # print(i, j, fdir_v)
                    length[prei][prej] = cellsize * celllen[fdir_v] * wt + relen
                    return length[prei][prej]
                else:
                    return 0
            if length[i][j] > 0:
                return length[i][j]

            if length[i][j] < 0:
                print("Error in calculating flowlen_cell function! i,j:")
                print(i, j)
                return -1
        return 0

    @staticmethod
    def calculate_flow_length(flow_dir_file, weight, flow_dir_code="TauDEM"):
        """Generate flow length with weight."""
        flow_dir_raster = RasterUtilClass.read_raster(flow_dir_file)
        fdir_data = flow_dir_raster.data
        xsize = flow_dir_raster.nCols
        ysize = flow_dir_raster.nRows
        nodata_value = flow_dir_raster.noDataValue
        # geotransform = flow_dir_raster.srs
        cellsize = flow_dir_raster.dx
        length = numpy.zeros((ysize, xsize))

        for i in range(0, ysize):
            for j in range(0, xsize):
                if abs(fdir_data[i][j] - nodata_value) < UTIL_ZERO:
                    length[i][j] = nodata_value
                    continue
                TerrainUtilClass.flow_length_cell(i, j, ysize, xsize, fdir_data, cellsize, weight,
                                                  length, flow_dir_code)
        return length

    @staticmethod
    def depression_capacity(maindb, landuse_file, slope_file, soil_texture_file,
                            depression_file, landuse_shp, imper_perc=0.3):
        """Initialize depression capacity according to landuse, soil, and slope.
        Args:
            maindb: main MongoDatabase
            landuse_file: landuse raster file
            slope_file: slope raster file
            soil_texture_file: soil texture file
            depression_file: resulted depression raster file
            imper_perc: impervious percent in urban cell, 0.3 as default
        """
        # read landuselookup table from MongoDB
        st_fields = ['DSC_ST%d' % (i,) for i in range(1, 13)]
        query_result = maindb['LANDUSELOOKUP'].find()
        if query_result is None:
            raise RuntimeError('LanduseLoop Collection is not existed or empty!')
        dep_sd0 = dict()
        for row in query_result:
            tmpid = row.get('LANDUSE_ID')
            dep_sd0[tmpid] = [float(row.get(item)) for item in st_fields]

        landu_r = RasterUtilClass.read_raster(landuse_file)
        landu_data = landu_r.data
        geotrans = landu_r.geotrans
        srs = landu_r.srs
        xsize = landu_r.nCols
        ysize = landu_r.nRows
        landu_nodata = landu_r.noDataValue

        slo_data = RasterUtilClass.read_raster(slope_file).data
        soil_texture_array = RasterUtilClass.read_raster(soil_texture_file).data

        id_omited = []

        def cal_dep(landu, soil_texture, slp):
            """Calculate depression"""
            last_stid = 0
            if abs(landu - landu_nodata) < UTIL_ZERO:
                return DEFAULT_NODATA
            landu_id = int(landu)
            if landu_id not in dep_sd0:
                if landu_id not in id_omited:
                    print('The landuse ID: %d does not exist.' % (landu_id,))
                    id_omited.append(landu_id)
            stid = int(soil_texture) - 1
            try:
                depression_grid0 = dep_sd0[landu_id][stid]
                last_stid = stid
            except Exception:
                depression_grid0 = dep_sd0[landu_id][last_stid]

            depression_grid = exp(numpy.log(depression_grid0 + 0.0001) + slp * (-9.5))
            # TODO, check if it is  (landu_id >= 98)? By LJ
            if landu_id == 106 or landu_id == 107 or landu_id == 105:
                return 0.5 * imper_perc + (1. - imper_perc) * depression_grid
            else:
                return depression_grid

        cal_dep_numpy = numpy.frompyfunc(cal_dep, 3, 1)
        dep_storage_cap = cal_dep_numpy(landu_data, soil_texture_array, slo_data)

        RasterUtilClass.write_gtiff_file(depression_file, ysize, xsize, dep_storage_cap,
                                         geotrans, srs, DEFAULT_NODATA, GDT_Float32)
        depression_csv = r'D:\SEIMS\data\zts\data_prepare\spatial\test\depression.csv'
        RasterUtilClass.count_raster_moist(depression_file, landuse_shp, depression_csv, True)

    @staticmethod
    def hydrological_radius(acc_file, radius_file, storm_probability="T2"):
        """Calculate hydrological radius."""
        acc_r = RasterUtilClass.read_raster(acc_file)
        xsize = acc_r.nCols
        ysize = acc_r.nRows
        nodata_value = acc_r.noDataValue
        cellsize = acc_r.dx
        data = acc_r.data
        coe_table = {"T2": [0.05, 0.48],
                     "T10": [0.12, 0.52],
                     "T100": [0.18, 0.55]}
        ap = coe_table[storm_probability][0]
        bp = coe_table[storm_probability][1]

        def radius_cal(acc):
            """Calculate hydrological radius"""
            if abs(acc - nodata_value) < UTIL_ZERO:
                return DEFAULT_NODATA
            return numpy.power(ap * ((acc + 1) * cellsize * cellsize / 1000000.), bp)

        radius_cal_numpy = numpy.frompyfunc(radius_cal, 1, 1)
        radius = radius_cal_numpy(data)

        RasterUtilClass.write_gtiff_file(radius_file, ysize, xsize, radius,
                                         acc_r.geotrans, acc_r.srs,
                                         DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def flow_velocity(slope_file, radius_file, manning_file, velocity_file):
        """velocity."""
        slp_r = RasterUtilClass.read_raster(slope_file)
        slp_data = slp_r.data
        xsize = slp_r.nCols
        ysize = slp_r.nRows
        nodata_value = slp_r.noDataValue

        rad_data = RasterUtilClass.read_raster(radius_file).data
        man_data = RasterUtilClass.read_raster(manning_file).data

        vel_max = 3.0
        vel_min = 0.0001

        def velocity_cal(rad, man, slp):
            """Calculate velocity"""
            if abs(slp - nodata_value) < UTIL_ZERO:
                return DEFAULT_NODATA
            # print(rad, man, slp)
            tmp = numpy.power(man, -1) * numpy.power(rad, 2. / 3.) * numpy.power(slp, 0.5)
            if tmp < vel_min:
                return vel_min
            if tmp > vel_max:
                return vel_max
            return tmp

        velocity_cal_numpy = numpy.frompyfunc(velocity_cal, 3, 1)
        velocity = velocity_cal_numpy(rad_data, man_data, slp_data)
        RasterUtilClass.write_gtiff_file(velocity_file, ysize, xsize, velocity, slp_r.geotrans,
                                         slp_r.srs, DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def flow_time_to_stream(streamlink, velocity, flow_dir_file, t0_s_file,
                            flow_dir_code="TauDEM"):
        """Calculate flow time to the workflow channel from each grid cell."""
        strlk_data = RasterUtilClass.read_raster(streamlink).data

        vel_r = RasterUtilClass.read_raster(velocity)
        vel_data = vel_r.data
        xsize = vel_r.nCols
        ysize = vel_r.nRows
        # noDataValue = vel_r.noDataValue

        weight = numpy.where(strlk_data <= 0, numpy.ones((ysize, xsize)),
                             numpy.zeros((ysize, xsize)))
        traveltime = numpy.where(vel_r.validZone, numpy.zeros((ysize, xsize)), vel_data)
        flowlen = TerrainUtilClass.calculate_flow_length(flow_dir_file, weight, flow_dir_code)
        traveltime = numpy.where(vel_r.validZone, flowlen / (vel_data * 5. / 3.) / 3600.,
                                 traveltime)
        RasterUtilClass.write_gtiff_file(t0_s_file, ysize, xsize, traveltime, vel_r.geotrans,
                                         vel_r.srs, DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def std_of_flow_time_to_stream(streamlink, flow_dir_file, slope, radius, velocity, delta_s_file,
                                   flow_dir_code="TauDEM"):
        """Generate standard deviation of t0_s (flow time to the workflow channel from each cell).
        """
        strlk_r = RasterUtilClass.read_raster(streamlink)
        strlk_data = strlk_r.data
        rad_data = RasterUtilClass.read_raster(radius).data
        slo_data = RasterUtilClass.read_raster(slope).data

        vel_r = RasterUtilClass.read_raster(velocity)
        vel_data = vel_r.data
        xsize = vel_r.nCols
        ysize = vel_r.nRows
        nodata_value = vel_r.noDataValue

        def initial_variables(vel, strlk, slp, rad):
            """initial variables"""
            if abs(vel - nodata_value) < UTIL_ZERO:
                return DEFAULT_NODATA
            if strlk <= 0:
                tmp_weight = 1
            else:
                tmp_weight = 0
            # 0 is river
            if slp < 0.0005:
                slp = 0.0005
            # dampGrid = vel * rad / (slp / 100. * 2.) # No need to divide 100
            # in my view. By LJ
            damp_grid = vel * rad / (slp * 2.)
            celerity = vel * 5. / 3.
            tmp_weight *= damp_grid * 2. / numpy.power(celerity, 3.)
            return tmp_weight

        initial_variables_numpy = numpy.frompyfunc(initial_variables, 4, 1)
        weight = initial_variables_numpy(vel_data, strlk_data, slo_data, rad_data)

        delta_s_sqr = TerrainUtilClass.calculate_flow_length(flow_dir_file, weight, flow_dir_code)

        def cal_delta_s(vel, sqr):
            """Calculate delta s"""
            if abs(vel - nodata_value) < UTIL_ZERO:
                return nodata_value
            else:
                return sqrt(sqr) / 3600.

        cal_delta_s_numpy = numpy.frompyfunc(cal_delta_s, 2, 1)
        delta_s = cal_delta_s_numpy(vel_data, delta_s_sqr)

        RasterUtilClass.write_gtiff_file(delta_s_file, ysize, xsize, delta_s, strlk_r.geotrans,
                                         strlk_r.srs, DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def calculate_latitude_dependent_parameters(lat_file, min_dayl_file, dormhr_file, dorm_hr):
        """
        Calculate latitude dependent parameters, include:
           1. minimum daylength (daylmn), 2. day length threshold for dormancy (dormhr)
        """
        # calculate minimum daylength, from readwgn.f of SWAT
        # daylength=2*acos(-tan(sd)*tan(lat))/omega
        # where solar declination, sd, = -23.5 degrees for minimum daylength in
        # northern hemisphere and -tan(sd) = .4348
        # absolute value is taken of tan(lat) to convert southern hemisphere
        # values to northern hemisphere
        # the angular velocity of the earth's rotation, omega, = 15 deg/hr or
        # 0.2618 rad/hr and 2/0.2618 = 7.6394
        cell_lat_r = RasterUtilClass.read_raster(lat_file)
        lat_data = cell_lat_r.data
        # daylmn_data = cell_lat_r.data
        zero = numpy.zeros((cell_lat_r.nRows, cell_lat_r.nCols))
        # nodata = numpy.ones((cell_lat_r.nRows, cell_lat_r.nCols)) * cell_lat_r.noDataValue
        # convert degrees to radians (2pi/360=1/57.296)
        daylmn_data = 0.4348 * numpy.abs(numpy.tan(lat_data / 57.296))
        condition = daylmn_data < 1.
        daylmn_data = numpy.where(condition, numpy.arccos(daylmn_data), zero)
        # condition2 = lat_data != cell_lat_r.noDataValue
        daylmn_data *= 7.6394
        daylmn_data = numpy.where(cell_lat_r.validZone, daylmn_data, lat_data)
        RasterUtilClass.write_gtiff_file(min_dayl_file, cell_lat_r.nRows, cell_lat_r.nCols,
                                         daylmn_data, cell_lat_r.geotrans, cell_lat_r.srs,
                                         cell_lat_r.noDataValue, GDT_Float32)

        def cal_dorm_hr(lat):
            """calculate day length threshold for dormancy"""
            if lat == cell_lat_r.noDataValue:
                return cell_lat_r.noDataValue
            else:
                if 20. <= lat <= 40:
                    return (numpy.abs(lat - 20.)) / 20.
                elif lat > 40.:
                    return 1.
                elif lat < 20.:
                    return -1.

        cal_dorm_hr_numpy = numpy.frompyfunc(cal_dorm_hr, 1, 1)

        # dormhr_data = numpy.copy(lat_data)
        if dorm_hr < -UTIL_ZERO:
            dormhr_data = cal_dorm_hr_numpy(lat_data)
        else:
            dormhr_data = numpy.where(cell_lat_r.validZone,
                                      numpy.ones((cell_lat_r.nRows, cell_lat_r.nCols)) * dorm_hr,
                                      lat_data)
        RasterUtilClass.write_gtiff_file(dormhr_file, cell_lat_r.nRows, cell_lat_r.nCols,
                                         dormhr_data, cell_lat_r.geotrans, cell_lat_r.srs,
                                         cell_lat_r.noDataValue, GDT_Float32)

    @staticmethod
    def calculate_channel_width(acc_file, chwidth_file):
        """Calculate channel width."""
        acc_r = RasterUtilClass.read_raster(acc_file)
        xsize = acc_r.nCols
        ysize = acc_r.nRows
        dx = acc_r.dx
        cell_area = dx * dx

        # storm frequency   a      b
        # 2                 1      0.56
        # 10                1.2    0.56
        # 100               1.4    0.56
        a = 1.2
        b = 0.56
        # TODO: Figure out what's means, and move it to text.py or config.py. LJ

        tmp_ones = numpy.ones((ysize, xsize))
        width = tmp_ones * DEFAULT_NODATA
        valid_values = numpy.where(acc_r.validZone, acc_r.data, tmp_ones)
        width = numpy.where(acc_r.validZone, numpy.power((a * (valid_values + 1)
                                                          * cell_area / 1000000.), b), width)
        RasterUtilClass.write_gtiff_file(chwidth_file, ysize, xsize, width, acc_r.geotrans,
                                         acc_r.srs, DEFAULT_NODATA, GDT_Float32)
        return width

    @staticmethod
    def add_channel_width_to_shp(reach_shp_file, stream_link_file,
                                 width_data, default_depth=1.5):
        """Add channel/reach width and default depth to ESRI shapefile"""
        stream_link = RasterUtilClass.read_raster(stream_link_file)
        n_rows = stream_link.nRows
        n_cols = stream_link.nCols
        nodata_value = stream_link.noDataValue
        data_stream = stream_link.data

        ch_width_dic = dict()
        ch_num_dic = dict()

        for i in range(n_rows):
            for j in range(n_cols):
                if abs(data_stream[i][j] - nodata_value) > UTIL_ZERO:
                    tmpid = int(data_stream[i][j])
                    ch_num_dic.setdefault(tmpid, 0)
                    ch_width_dic.setdefault(tmpid, 0)
                    ch_num_dic[tmpid] += 1
                    ch_width_dic[tmpid] += width_data[i][j]

        for k in ch_num_dic:
            ch_width_dic[k] /= ch_num_dic[k]

        # add channel width_data field to reach shp file
        ds_reach = ogr_Open(reach_shp_file, update=True)
        layer_reach = ds_reach.GetLayer(0)
        layer_def = layer_reach.GetLayerDefn()
        i_link = layer_def.GetFieldIndex(ImportReaches2Mongo._LINKNO)
        i_width = layer_def.GetFieldIndex(ImportReaches2Mongo._WIDTH)
        i_depth = layer_def.GetFieldIndex(ImportReaches2Mongo._DEPTH)
        if i_width < 0:
            new_field = ogr_FieldDefn(ImportReaches2Mongo._WIDTH, OFTReal)
            layer_reach.CreateField(new_field)
        if i_depth < 0:
            new_field = ogr_FieldDefn(ImportReaches2Mongo._DEPTH, OFTReal)
            layer_reach.CreateField(new_field)
            # grid_code:feature map
        # ftmap = {}
        layer_reach.ResetReading()
        ft = layer_reach.GetNextFeature()
        while ft is not None:
            tmpid = ft.GetFieldAsInteger(i_link)
            w = 1
            if tmpid in list(ch_width_dic.keys()):
                w = ch_width_dic[tmpid]
            ft.SetField(ImportReaches2Mongo._WIDTH, w)
            ft.SetField(ImportReaches2Mongo._DEPTH, default_depth)
            layer_reach.SetFeature(ft)
            ft = layer_reach.GetNextFeature()

        layer_reach.SyncToDisk()
        ds_reach.Destroy()
        del ds_reach

    @staticmethod
    def parameters_extraction(cfg, maindb):
        """Main entrance for terrain related spatial parameters extraction."""
        f = cfg.logs.extract_terrain
        landuse_shp = cfg.landuse_shp
        # 1. Calculate initial channel width by accumulated area and add width to reach.shp.
        status_output("Calculate initial channel width and added to reach.shp...", 10, f)
        acc_file = cfg.spatials.d8acc
        channel_width_file = cfg.spatials.chwidth
        channel_shp_file = cfg.vecs.reach
        streamlink_file = cfg.spatials.stream_link
        chwidth_data = TerrainUtilClass.calculate_channel_width(acc_file, channel_width_file)
        TerrainUtilClass.add_channel_width_to_shp(channel_shp_file, streamlink_file, chwidth_data,
                                                  cfg.default_reach_depth)
        # 2. Initialize depression storage capacity
        status_output("Generating depression storage capacity...", 20, f)
        slope_file = cfg.spatials.slope
        soil_texture_file = cfg.spatials.soil_texture
        landuse_file = cfg.spatials.landuse
        depression_file = cfg.spatials.depression
        TerrainUtilClass.depression_capacity(maindb, landuse_file, soil_texture_file,
                                             slope_file, depression_file, landuse_shp,
                                             cfg.imper_perc_in_urban)
        # 2. Calculate inputs for IUH
        status_output("Prepare parameters for IUH...", 30, f)
        radius_file = cfg.spatials.radius
        TerrainUtilClass.hydrological_radius(acc_file, radius_file, "T2")
        manning_file = cfg.spatials.manning
        velocity_file = cfg.spatials.velocity
        TerrainUtilClass.flow_velocity(slope_file, radius_file, manning_file, velocity_file)
        flow_dir_file = cfg.spatials.d8flow
        t0_s_file = cfg.spatials.t0_s
        flow_model_code = "ArcGIS"
        TerrainUtilClass.flow_time_to_stream(streamlink_file, velocity_file, flow_dir_file,
                                             t0_s_file, flow_model_code)
        delta_s_file = cfg.spatials.delta_s
        TerrainUtilClass.std_of_flow_time_to_stream(streamlink_file, flow_dir_file, slope_file,
                                                    radius_file, velocity_file, delta_s_file,
                                                    flow_model_code)
        # IUH calculation and import to MongoDB are implemented in db_build_mongodb.py
        # 3. Calculate position (i.e. latitude) related parameters
        status_output("Calculate latitude dependent parameters...", 40, f)
        lat_file = cfg.spatials.cell_lat
        min_dayl_file = cfg.spatials.dayl_min
        dormhr_file = cfg.spatials.dorm_hr
        TerrainUtilClass.calculate_latitude_dependent_parameters(lat_file, min_dayl_file,
                                                                 dormhr_file, cfg.dorm_hr)

        status_output("Terrain related spatial parameters extracted done!", 100, f)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    # from .db_mongodb import ConnectMongoDB
    from db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    main_db = conn[seims_cfg.spatial_db]

    TerrainUtilClass.parameters_extraction(seims_cfg, main_db)


if __name__ == '__main__':
    import time

    start_time = time.time()
    main()
    end_time = time.time()
    print('SEIMS preprocess done, time-consuming: %.2f seconds.' % (end_time - start_time))
