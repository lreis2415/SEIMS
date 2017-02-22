#! /usr/bin/env python
# coding=utf-8
# Identify depression storage capacity from slope, soil, and landuse. Algorithm from WetSpa.
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
# Date: 2016-7-6
# Note: Code optimization by using numpy.
# TODO: 1. Add stream order modification, according to depression.ave of WetSpa.
# TODO: 2. Add another depressional storage method according to SWAT, depstor.f
import sqlite3
import math
import sys

import numpy
import ogr
from gdal import GDT_Float32
from pygeoc.raster.raster import RasterUtilClass
from pygeoc.hydro.hydro import FlowDirectionCode

from config import *
from utility import LoadConfiguration, status_output
from utility import UTIL_ZERO, DEFAULT_NODATA

sys.setrecursionlimit(10000)


class TerrainUtilClass(object):
    """Terrain related spatial parameters algorithm."""

    def __init__(self):
        pass

    @staticmethod
    def flow_length_cell(i, j, ysize, xsize, fdir, cellsize, weight, length, flow_dir_code="TauDEM"):
        """Calculate flow length of cell."""
        flowcode = FlowDirectionCode(flow_dir_code)
        celllen = flowcode.get_cell_length()
        differ = flowcode.get_cell_shift()
        # print i,j, weight[i][j]
        if i < ysize and j < xsize:
            if length[i][j] == 0:
                if weight[i][j] > 0:
                    prei = i
                    prej = j
                    wt = weight[i][j]
                    fdirV = fdir[i][j]
                    di = differ[fdirV][0]
                    dj = differ[fdirV][1]
                    i = i + di
                    j = j + dj
                    relen = TerrainUtilClass.flow_length_cell(i, j, ysize, xsize, fdir, cellsize, weight, length,
                                                              flow_dir_code)
                    # print i,j,fdirV
                    length[prei][prej] = cellsize * celllen[fdirV] * wt + relen
                    return length[prei][prej]
                else:
                    return 0
            if length[i][j] > 0:
                return length[i][j]

            if length[i][j] < 0:
                print "Error in calculating flowlen_cell function! i,j:"
                print i, j
                return -1
        return 0

    @staticmethod
    def calculate_flow_length(flow_dir_file, weight, flow_dir_code="TauDEM"):
        """Generate flow length with weight."""
        flow_dir_raster = RasterUtilClass.ReadRaster(flow_dir_file)
        fdir_data = flow_dir_raster.data
        xsize = flow_dir_raster.nCols
        ysize = flow_dir_raster.nRows
        noDataValue = flow_dir_raster.noDataValue
        # geotransform = flow_dir_raster.srs
        cellsize = flow_dir_raster.dx
        length = numpy.zeros((ysize, xsize))

        for i in range(0, ysize):
            for j in range(0, xsize):
                if abs(fdir_data[i][j] - noDataValue) < UTIL_ZERO:
                    length[i][j] = noDataValue
                    continue
                TerrainUtilClass.flow_length_cell(i, j, ysize, xsize, fdir_data, cellsize, weight, length,
                                                  flow_dir_code)
        return length

    @staticmethod
    def depression_capacity(sqlite_file, landuse_file, slope_file, soiltexutre_file, depression_file):
        """Initialize depression capacity according to landuse, soil, and slope.
        TODO: 1. Add stream order modification, according to depression.ave of WetSpa.
        TODO: 2. Add another depressional storage method according to SWAT, depstor.f
        """
        # read landuselookup table from sqlite
        st_fields = ["DSC_ST%d" % (i,) for i in range(1, 13)]
        sql_landuse = 'select LANDUSE_ID,%s from LanduseLookup' % (','.join(st_fields),)

        conn = sqlite3.connect(sqlite_file)
        cursor = conn.cursor()
        cursor.execute(sql_landuse)

        dep_sd0 = {}
        for row in cursor:
            id = int(row[0])
            dep_sd0[id] = [float(item) for item in row[1:]]

        cursor.close()
        conn.close()
        # end read data

        landu_R = RasterUtilClass.ReadRaster(landuse_file)
        landu_data = landu_R.data
        geotrans = landu_R.geotrans
        srs = landu_R.srs
        xsize = landu_R.nCols
        ysize = landu_R.nRows
        landu_nodata = landu_R.noDataValue

        slo_data = RasterUtilClass.ReadRaster(slope_file).data
        soilTextureArray = RasterUtilClass.ReadRaster(soiltexutre_file).data

        idOmited = []

        def calDep(landu, soilTexture, slp):
            lastStid = 0
            if abs(landu - landu_nodata) < UTIL_ZERO:
                return DEFAULT_NODATA
            landuID = int(landu)
            if landuID not in dep_sd0:
                if landuID not in idOmited:
                    print 'The landuse ID: %d does not exist.' % (landuID,)
                    idOmited.append(landuID)
            stid = int(soilTexture) - 1
            try:
                depressionGrid0 = dep_sd0[landuID][stid]
                lastStid = stid
            except:
                depressionGrid0 = dep_sd0[landuID][lastStid]

            depressionGrid = math.exp(numpy.log(depressionGrid0 + 0.0001) + slp * (-9.5))
            # TODO, check if it is  (landuID >= 98)? By LJ
            if landuID == 106 or landuID == 107 or landuID == 105:
                return 0.5 * imperviousPercInUrbanCell + (1. - imperviousPercInUrbanCell) * depressionGrid
            else:
                return depressionGrid

        calDep_numpy = numpy.frompyfunc(calDep, 3, 1)
        depStorageCap = calDep_numpy(landu_data, soilTextureArray, slo_data)

        RasterUtilClass.WriteGTiffFile(depression_file, ysize, xsize, depStorageCap,
                                       geotrans, srs, DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def hydrological_radius(acc_file, radius_file, storm_probability="T2"):
        """Calculate hydrological radius."""
        acc_R = RasterUtilClass.ReadRaster(acc_file)
        xsize = acc_R.nCols
        ysize = acc_R.nRows
        noDataValue = acc_R.noDataValue
        cellsize = acc_R.dx
        data = acc_R.data
        coeTable = {"T2": [0.05, 0.48],
                    "T10": [0.12, 0.52],
                    "T100": [0.18, 0.55]}
        ap = coeTable[storm_probability][0]
        bp = coeTable[storm_probability][1]

        def radius_cal(acc):
            if abs(acc - noDataValue) < UTIL_ZERO:
                return DEFAULT_NODATA
            return numpy.power(ap * ((acc + 1) * cellsize * cellsize / 1000000.), bp)

        radius_cal_numpy = numpy.frompyfunc(radius_cal, 1, 1)
        radius = radius_cal_numpy(data)

        RasterUtilClass.WriteGTiffFile(radius_file, ysize, xsize, radius,
                                       acc_R.geotrans, acc_R.srs, DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def flow_velocity(slope_file, radius_file, manning_file, velocity_file):
        """velocity."""
        slp_R = RasterUtilClass.ReadRaster(slope_file)
        slo_data = slp_R.data
        xsize = slp_R.nCols
        ysize = slp_R.nRows
        noDataValue = slp_R.noDataValue

        rad_data = RasterUtilClass.ReadRaster(radius_file).data
        Man_data = RasterUtilClass.ReadRaster(manning_file).data

        vel_max = 3.0
        vel_min = 0.0001

        def velocity_cal(rad, man, slp):
            if abs(slp - noDataValue) < UTIL_ZERO:
                return DEFAULT_NODATA
            # print rad,man,slp
            tmp = numpy.power(man, -1) * numpy.power(rad, 2 / 3) * numpy.power(slp, 0.5)
            # print tmp
            if tmp < vel_min:
                return vel_min
            if tmp > vel_max:
                return vel_max
            return tmp

        velocity_cal_numpy = numpy.frompyfunc(velocity_cal, 3, 1)
        velocity = velocity_cal_numpy(rad_data, Man_data, slo_data)
        RasterUtilClass.WriteGTiffFile(velocity_file, ysize, xsize, velocity, slp_R.geotrans, slp_R.srs, DEFAULT_NODATA,
                                       GDT_Float32)

    @staticmethod
    def flow_time_to_stream(streamlink, velocity, flow_dir_file, t0_s_file, flow_dir_code="TauDEM"):
        """Calculate flow time to the main channel from each grid cell."""
        strlk_data = RasterUtilClass.ReadRaster(streamlink).data

        vel_R = RasterUtilClass.ReadRaster(velocity)
        vel_data = vel_R.data
        xsize = vel_R.nCols
        ysize = vel_R.nRows
        # noDataValue = vel_R.noDataValue

        weight = numpy.where(strlk_data <= 0, numpy.ones((ysize, xsize)), numpy.zeros((ysize, xsize)))
        traveltime = numpy.where(vel_R.validZone, numpy.zeros((ysize, xsize)), vel_data)
        flowlen = TerrainUtilClass.calculate_flow_length(flow_dir_file, weight, flow_dir_code)
        traveltime = numpy.where(vel_R.validZone, flowlen / (vel_data * 5. / 3.) / 3600., traveltime)
        RasterUtilClass.WriteGTiffFile(t0_s_file, ysize, xsize, traveltime, vel_R.geotrans, vel_R.srs, DEFAULT_NODATA,
                                       GDT_Float32)

    @staticmethod
    def std_of_flow_time_to_stream(streamlink, flow_dir_file, slope, radius, velocity, delta_s_file,
                                   flow_dir_code="TauDEM"):
        """Generate standard deviation of t0_s (flow time to the main channel from each cell)."""
        strlkR = RasterUtilClass.ReadRaster(streamlink)
        strlk_data = strlkR.data
        rad_data = RasterUtilClass.ReadRaster(radius).data
        slo_data = RasterUtilClass.ReadRaster(slope).data

        velR = RasterUtilClass.ReadRaster(velocity)
        vel_data = velR.data
        xsize = velR.nCols
        ysize = velR.nRows
        noDataValue = velR.noDataValue

        def initial_variables(vel, strlk, slp, rad):
            if abs(vel - noDataValue) < UTIL_ZERO:
                return DEFAULT_NODATA
            if strlk <= 0:
                tmp_weight = 1
            else:
                tmp_weight = 0
            # 0 is river
            if slp < 0.0005:
                slp = 0.0005
            # dampGrid = vel * rad / (slp / 100. * 2.) # No need to divide 100 in
            # my view. By LJ
            damp_grid = vel * rad / (slp * 2.)
            celerity = vel * 5. / 3.
            tmp_weight *= damp_grid * 2. / numpy.power(celerity, 3.)
            return tmp_weight

        initial_variables_numpy = numpy.frompyfunc(initial_variables, 4, 1)
        weight = initial_variables_numpy(vel_data, strlk_data, slo_data, rad_data)

        delta_s_sqr = TerrainUtilClass.calculate_flow_length(flow_dir_file, weight, flow_dir_code)

        def cal_delta_s(vel, sqr):
            if abs(vel - noDataValue) < UTIL_ZERO:
                return noDataValue
            else:
                return math.sqrt(sqr) / 3600.

        cal_delta_s_numpy = numpy.frompyfunc(cal_delta_s, 2, 1)
        delta_s = cal_delta_s_numpy(vel_data, delta_s_sqr)

        RasterUtilClass.WriteGTiffFile(delta_s_file, ysize, xsize, delta_s, strlkR.geotrans, strlkR.srs,
                                       DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def calculate_latitude_dependent_parameters(lat_file, min_dayl_file, dormhr_file):
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
        cellLatR = RasterUtilClass.ReadRaster(lat_file)
        latData = cellLatR.data
        # daylmnData = cellLatR.data
        zero = numpy.zeros((cellLatR.nRows, cellLatR.nCols))
        # nodata = numpy.ones((cellLatR.nRows, cellLatR.nCols)) * cellLatR.noDataValue
        # convert degrees to radians (2pi/360=1/57.296)
        daylmnData = 0.4348 * numpy.abs(numpy.tan(latData / 57.296))
        condition = daylmnData < 1.
        daylmnData = numpy.where(condition, numpy.arccos(daylmnData), zero)
        # condition2 = latData != cellLatR.noDataValue
        daylmnData *= 7.6394
        daylmnData = numpy.where(cellLatR.validZone, daylmnData, latData)
        RasterUtilClass.WriteGTiffFile(min_dayl_file, cellLatR.nRows, cellLatR.nCols,
                                       daylmnData, cellLatR.geotrans, cellLatR.srs, cellLatR.noDataValue, GDT_Float32)

        # calculate day length threshold for dormancy
        def calDormHr(lat):
            if lat == cellLatR.noDataValue:
                return cellLatR.noDataValue
            else:
                if 20. <= lat <= 40.:
                    return (numpy.abs(lat - 20.)) / 20.
                elif lat > 40.:
                    return 1.
                elif lat < 20.:
                    return -1.

        calDormHr_numpy = numpy.frompyfunc(calDormHr, 1, 1)

        # dormhrData = numpy.copy(latData)
        if dorm_hr < -UTIL_ZERO:
            dormhrData = calDormHr_numpy(latData)
        else:
            dormhrData = numpy.where(cellLatR.validZone, numpy.ones((cellLatR.nRows, cellLatR.nCols)) * dorm_hr,
                                     latData)
        RasterUtilClass.WriteGTiffFile(dormhr_file, cellLatR.nRows, cellLatR.nCols, dormhrData, cellLatR.geotrans,
                                       cellLatR.srs, cellLatR.noDataValue, GDT_Float32)

    @staticmethod
    def calculate_channel_width(acc_file, chwidth_file):
        """Calculate channel width."""
        accR = RasterUtilClass.ReadRaster(acc_file)
        xsize = accR.nCols
        ysize = accR.nRows
        noDataValue = accR.noDataValue
        dx = accR.dx
        cellArea = dx * dx

        # storm frequency   a      b
        # 2                 1      0.56
        # 10                1.2    0.56
        # 100               1.4    0.56
        a = 1.2
        b = 0.56
        # TODO: Figure out what's means, and move it to text.py or config.py. LJ

        tmpOnes = numpy.ones((ysize, xsize))
        width = tmpOnes * DEFAULT_NODATA
        validValues = numpy.where(accR.validZone, accR.data, tmpOnes)
        width = numpy.where(accR.validZone, numpy.power(
            (a * (validValues + 1) * cellArea / 1000000.), b), width)
        RasterUtilClass.WriteGTiffFile(chwidth_file, ysize, xsize, width, accR.geotrans,
                                       accR.srs, DEFAULT_NODATA, GDT_Float32)
        return width

    @staticmethod
    def add_channel_width_to_shp(reach_shp_file, stream_link_file, width_data):
        streamLink = RasterUtilClass.ReadRaster(stream_link_file)
        nRows = streamLink.nRows
        nCols = streamLink.nCols
        noDataValue = streamLink.noDataValue
        dataStream = streamLink.data

        chWidthDic = dict()
        chNumDic = dict()

        for i in range(nRows):
            for j in range(nCols):
                if abs(dataStream[i][j] - noDataValue) > UTIL_ZERO:
                    id = int(dataStream[i][j])
                    chNumDic.setdefault(id, 0)
                    chWidthDic.setdefault(id, 0)
                    chNumDic[id] += 1
                    chWidthDic[id] += width_data[i][j]

        for k in chNumDic:
            chWidthDic[k] /= chNumDic[k]

        # add channel width_data field to reach shp file
        dsReach = ogr.Open(reach_shp_file, update=True)
        layerReach = dsReach.GetLayer(0)
        layerDef = layerReach.GetLayerDefn()
        iLink = layerDef.GetFieldIndex(FLD_LINKNO)
        iWidth = layerDef.GetFieldIndex(REACH_WIDTH)
        iDepth = layerDef.GetFieldIndex(REACH_DEPTH)
        if iWidth < 0:
            new_field = ogr.FieldDefn(REACH_WIDTH, ogr.OFTReal)
            layerReach.CreateField(new_field)
        if iDepth < 0:
            new_field = ogr.FieldDefn(REACH_DEPTH, ogr.OFTReal)
            layerReach.CreateField(new_field)
            # grid_code:feature map
        # ftmap = {}
        layerReach.ResetReading()
        ft = layerReach.GetNextFeature()
        while ft is not None:
            id = ft.GetFieldAsInteger(iLink)
            w = 1
            if id in chWidthDic.keys():
                w = chWidthDic[id]
            ft.SetField(REACH_WIDTH, w)
            ft.SetField(REACH_DEPTH, default_reach_depth)
            layerReach.SetFeature(ft)
            ft = layerReach.GetNextFeature()

        layerReach.SyncToDisk()
        dsReach.Destroy()
        del dsReach


def terrain_parameters_extration():
    """Main entrance for terrain related spatial parameters extraction. Algorithm from WetSpa.
    """
    status_file = WORKING_DIR + os.sep + DIR_NAME_LOG + os.sep + FN_STATUS_EXTRACTTERRAINPARAM
    f = open(status_file, 'w')
    geodata2dbdir = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB
    # 1. Calculate initial channel width by accumulated area and add width to reach.shp.
    status_output("Calculate initial channel width and added to reach.shp...", 10, f)
    acc_file = geodata2dbdir + os.sep + accM
    channel_width_file = geodata2dbdir + os.sep + chwidthName
    channel_shp_file = WORKING_DIR + os.sep + DIR_NAME_GEOSHP + os.sep + reachesOut
    streamlink_file = geodata2dbdir + os.sep + streamLinkOut
    chwidth_data = TerrainUtilClass.calculate_channel_width(acc_file, channel_width_file)
    TerrainUtilClass.add_channel_width_to_shp(channel_shp_file, streamlink_file, chwidth_data)
    # 2. Initialize depression storage capacity
    status_output("Generating depression storage capacity...", 20, f)
    slope_file = geodata2dbdir + os.sep + slopeM
    soil_texture_file = geodata2dbdir + os.sep + soilTexture
    landuse_file = geodata2dbdir + os.sep + landuseMFile
    depression_file = geodata2dbdir + os.sep + depressionFile
    sqlite_dbfile = WORKING_DIR + os.sep + DIR_NAME_IMPORT2DB + os.sep + sqlite_file
    TerrainUtilClass.depression_capacity(sqlite_dbfile, landuse_file, soil_texture_file, slope_file,
                                         depression_file)
    # 2. Calculate IUH
    if genIUH:
        status_output("Prepare parameters for IUH...", 30, f)
        radius_file = geodata2dbdir + os.sep + radiusFile
        TerrainUtilClass.hydrological_radius(acc_file, radius_file, "T2")
        manning_file = geodata2dbdir + os.sep + ManningFile
        velocity_file = geodata2dbdir + os.sep + velocityFile
        TerrainUtilClass.flow_velocity(slope_file, radius_file, manning_file, velocity_file)
        flow_dir_file = geodata2dbdir + os.sep + flowDirOut
        t0_s_file = geodata2dbdir + os.sep + t0_sFile
        flow_model_code = "TauDEM"
        if not isTauDEM:
            flow_model_code = "ArcGIS"
        TerrainUtilClass.flow_time_to_stream(streamlink_file, velocity_file, flow_dir_file, t0_s_file, flow_model_code)
        delta_s_file = geodata2dbdir + os.sep + delta_sFile
        TerrainUtilClass.std_of_flow_time_to_stream(streamlink_file, flow_dir_file, slope_file, radius_file,
                                                    velocity_file, delta_s_file, flow_model_code)
        # IUH calculation and import to MongoDB are implemented in db_build_mongodb.py
    # 3. Calculate position (i.e. latitude) related parameters
    status_output("Calculate latitude dependent parameters...", 40, f)
    lat_file = geodata2dbdir + os.sep + cellLat
    min_dayl_file = geodata2dbdir + os.sep + daylMin
    dormhr_file = geodata2dbdir + os.sep + dormhr
    TerrainUtilClass.calculate_latitude_dependent_parameters(lat_file, min_dayl_file, dormhr_file)

    status_output("Terrain related spatial parameters extracted done!", 100, f)
    f.close()


if __name__ == '__main__':
    LoadConfiguration(getconfigfile())
    terrain_parameters_extration()
