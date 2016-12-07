#! /usr/bin/env python
# coding=utf-8
# @Subbasin delineation based on TauDEM, as well as calculation of latitude dependent parameters
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
# @Note: Improve calculation efficiency by numpy
#


import numpy

from config import *
from gen_dinf import GenerateDinf
from gen_subbasins import GenerateSubbasins
from TauDEM import *
from util import *
from osgeo import ogr


def GenerateCellLatRaster():
    ds = ReadRaster(WORKING_DIR + os.sep +
                    DIR_NAME_TAUDEM + os.sep + filledDem)
    src_srs = ds.srs

    dst_srs = osr.SpatialReference()
    dst_srs.ImportFromEPSG(4326)  # WGS84
    # dst_wkt = dst_srs.ExportToWkt()

    transform = osr.CoordinateTransformation(src_srs, dst_srs)

    point_ll = ogr.CreateGeometryFromWkt("POINT (%f %f)" % (ds.xMin, ds.yMin))
    point_ur = ogr.CreateGeometryFromWkt("POINT (%f %f)" % (ds.xMax, ds.yMax))

    point_ll.Transform(transform)
    point_ur.Transform(transform)

    lowerLat = point_ll.GetY()
    upLat = point_ur.GetY()

    rows = ds.nRows
    cols = ds.nCols

    deltaLat = (upLat - lowerLat) / float(rows)

    def calCellLat(row, col):
        return upLat - (row + 0.5) * deltaLat

    dataLat = numpy.fromfunction(calCellLat, (rows, cols))
    dataLat = numpy.where(ds.validZone, dataLat, ds.data)
    # for row in range(rows):
    #     for col in range(cols):
    #         if dataLat[row][col] != ds.noDataValue:
    #             dataLat[row][col] = upLat - (row + 0.5) * deltaLat
    WriteGTiffFile(WORKING_DIR + os.sep + DIR_NAME_TAUDEM + os.sep + cellLat, rows, cols, dataLat,
                   ds.geotrans, ds.srs, ds.noDataValue, GDT_Float32)
    # print lowerLat,upLat


def CalLatDependParas():
    '''
    Calculate latitude dependent parameters, include:
       1. minimum daylength (daylmn), 2. day length threshold for dormancy (dormhr)
    :return: GeoTIFF
    '''
    # calculate minimum daylength, from readwgn.f of SWAT
    # daylength=2*acos(-tan(sd)*tan(lat))/omega
    # where solar declination, sd, = -23.5 degrees for minimum daylength in
    # northern hemisphere and -tan(sd) = .4348
    # absolute value is taken of tan(lat) to convert southern hemisphere
    # values to northern hemisphere
    # the angular velocity of the earth's rotation, omega, = 15 deg/hr or
    # 0.2618 rad/hr and 2/0.2618 = 7.6394
    cellLatR = ReadRaster(WORKING_DIR + os.sep + DIR_NAME_TAUDEM + os.sep + cellLat)
    latData = cellLatR.data
    # daylmnData = cellLatR.data
    zero = numpy.zeros((cellLatR.nRows, cellLatR.nCols))
    # nodata = numpy.ones((cellLatR.nRows, cellLatR.nCols)) * cellLatR.noDataValue
    # convert degrees to radians (2pi/360=1/57.296)
    daylmnData = 0.4348 * numpy.abs(numpy.tan(latData / 57.296))
    condition = daylmnData < 1.
    daylmnData = numpy.where(condition, numpy.arccos(daylmnData), zero)
    # condition2 = latData != cellLatR.noDataValue
    daylmnData = daylmnData * 7.6394
    daylmnData = numpy.where(cellLatR.validZone, daylmnData, latData)
    WriteGTiffFile(WORKING_DIR + os.sep + DIR_NAME_TAUDEM + os.sep + daylMin, cellLatR.nRows, cellLatR.nCols,
                   daylmnData,
                   cellLatR.geotrans, cellLatR.srs, cellLatR.noDataValue, GDT_Float32)

    # calculate day length threshold for dormancy
    def calDormHr(lat):
        if lat == cellLatR.noDataValue:
            return cellLatR.noDataValue
        else:
            if lat <= 40. and lat >= 20.:
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
        dormhrData = numpy.where(cellLatR.validZone,
                                 numpy.ones((cellLatR.nRows, cellLatR.nCols)) * dorm_hr, latData)
    WriteGTiffFile(WORKING_DIR + os.sep + DIR_NAME_TAUDEM + os.sep + dormhr, cellLatR.nRows, cellLatR.nCols, dormhrData,
                   cellLatR.geotrans, cellLatR.srs, cellLatR.noDataValue, GDT_Float32)


def SubbasinDelineation():
    if not os.path.exists(WORKING_DIR):
        os.mkdir(WORKING_DIR)
    statusFile = WORKING_DIR + os.sep + FN_STATUS_DELINEATION
    fStatus = open(statusFile, 'w')
    tauDir = WORKING_DIR + os.sep + DIR_NAME_TAUDEM
    if not os.path.exists(tauDir):
        os.mkdir(tauDir)
    # print MPIEXEC_DIR
    status = "Fill DEM..."
    fStatus.write("%d,%s\n" % (10, status))
    fStatus.flush()
    print (Fill(np, tauDir, dem, filledDem, mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR))
    print ("[Output], %s, %s" % (WORKING_DIR, status))

    status = "Calculating D8 and Dinf flow direction..."
    fStatus.write("%d,%s\n" % (20, status))
    fStatus.flush()
    print (FlowDirD8(np, tauDir, filledDem, flowDir, slope, mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR))
    print (GenerateDinf(np, tauDir, filledDem, flowDirDinf, slopeDinf, dirCodeDinf, weightDinf,
                        mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR))
    print ("[Output], %s, %s" % (WORKING_DIR, status))

    status = "D8 flow accumulation..."
    fStatus.write("%d,%s\n" % (40, status))
    fStatus.flush()
    print (FlowAccD8(np, tauDir, flowDir, acc, outlet=None, streamSkeleton=None, mpiexeDir=MPIEXEC_DIR,
                    exeDir=CPP_PROGRAM_DIR))
    print ("[Output], %s, %s" % (WORKING_DIR, status))

    status = "Generating stream raster initially..."
    fStatus.write("%d,%s\n" % (50, status))
    fStatus.flush()
    if D8AccThreshold > 0:
        print (StreamRaster(np, tauDir, acc, streamRaster, D8AccThreshold, mpiexeDir=MPIEXEC_DIR,
                           exeDir=CPP_PROGRAM_DIR))
    else:
        accD8 = tauDir + os.sep + acc
        maxAccum, minAccum, meanAccum, STDAccum = GetRasterStat(accD8)
        print (StreamRaster(np, tauDir, acc, streamRaster, meanAccum, mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR))
        print ("[Output], %s, %s" % (WORKING_DIR, status))

    status = "Moving outlet to stream..."
    fStatus.write("%d,%s\n" % (60, status))
    fStatus.flush()
    print (MoveOutlet(np, tauDir, flowDir, streamRaster, outlet_file, modifiedOutlet, mpiexeDir=MPIEXEC_DIR,
                     exeDir=CPP_PROGRAM_DIR))
    print ("[Output], %s, %s" % (WORKING_DIR, status))

    if D8AccThreshold <= 0:
        status = "Generating stream skeleton..."
        fStatus.write("%d,%s\n" % (30, status))
        fStatus.flush()
        print (StreamSkeleton(np, tauDir, filledDem, streamSkeleton, mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR))
        print ("[Output], %s, %s" % (WORKING_DIR, status))
        status = "Flow accumulation with outlet..."
        fStatus.write("%d,%s\n" % (70, status))
        fStatus.flush()
        print (FlowAccD8(np, tauDir, flowDir, accWithWeight, modifiedOutlet, streamSkeleton, mpiexeDir=MPIEXEC_DIR,
                        exeDir=CPP_PROGRAM_DIR))
        print ("[Output], %s, %s" % (WORKING_DIR, status))
        status = "Drop analysis to select optimal threshold..."
        fStatus.write("%d,%s\n" % (75, status))
        fStatus.flush()
        maxAccum, minAccum, meanAccum, STDAccum = GetRasterStat(accWithWeight)
        if meanAccum - STDAccum < 0:
            minthresh = meanAccum
        else:
            minthresh = meanAccum - STDAccum
        maxthresh = meanAccum + STDAccum
        numthresh = 20
        logspace = 'true'
        drpFile = 'drp.txt'
        print (DropAnalysis(np, tauDir, filledDem, flowDir, accWithWeight, accWithWeight, modifiedOutlet, minthresh,
                           maxthresh, numthresh, logspace, drpFile,
                           mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR))
        drpf = open(drpFile, "r")
        tempContents = drpf.read()
        (beg, Threshold) = tempContents.rsplit(' ', 1)
        print (Threshold)
        drpf.close()
        print ("[Output], %s, %s" % (WORKING_DIR, status))

        status = "Generating stream raster..."
        fStatus.write("%d,%s\n" % (80, status))
        fStatus.flush()
        print (StreamRaster(np, tauDir, accWithWeight, streamRaster, float(Threshold), mpiexeDir=MPIEXEC_DIR,
                           exeDir=CPP_PROGRAM_DIR))
        print ("[Output], %s, %s" % (WORKING_DIR, status))
    else:
        status = "Flow accumulation with outlet..."
        fStatus.write("%d,%s\n" % (70, status))
        fStatus.flush()
        print (FlowAccD8(np, tauDir, flowDir, acc, modifiedOutlet, streamSkeleton=None,
                        mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR))
        print ("[Output], %s, %s" % (WORKING_DIR, status))
        status = "Generating stream raster..."
        fStatus.write("%d,%s\n" % (80, status))
        fStatus.flush()
        print (StreamRaster(np, tauDir, acc, streamRaster, D8AccThreshold, mpiexeDir=MPIEXEC_DIR,
                           exeDir=CPP_PROGRAM_DIR))
        print ("[Output], %s, %s" % (WORKING_DIR, status))
    status = "Generating stream net..."
    fStatus.write("%d,%s\n" % (90, status))
    fStatus.flush()
    if D8AccThreshold <= 0:
        tmpAcc = accWithWeight
    else:
        tmpAcc = acc
    print (StreamNet(np, tauDir, filledDem, flowDir, tmpAcc, streamRaster, modifiedOutlet, streamOrder, chNetwork,
                    chCoord, streamNet, subbasin, mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR))
    print ("[Output], %s, %s" % (WORKING_DIR, status))

    status = "Calculating distance to stream (D8)..."
    fStatus.write("%d,%s\n" % (95, status))
    fStatus.flush()
    print (D8DistDownToStream(np, tauDir, flowDir, filledDem, streamRaster, dist2StreamD8, D8DownMethod, 1,
                             mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR))
    print ("[Output], %s, %s" % (WORKING_DIR, status))

    fStatus.write("100, subbasin delineation is finished!")
    fStatus.close()

    # There is no need to write projection config to file. By LJ
    # Get spatial reference from Source DEM file
    # ds = gdal.Open(dem)
    # Write Projection Configuration file
    # configFile = WORKING_DIR + os.sep + 'ProjConfig.txt'
    # f = open(configFile, 'w')
    # f.write(dem + "\n")
    # f.write(str(D8AccThreshold) + "\n")
    # projWkt = ds.GetProjection()
    # srs = osr.SpatialReference()
    # srs.ImportFromWkt(projWkt)
    # proj4Str = srs.ExportToProj4()
    # f.write(proj4Str + "\n")
    # f.close()

    # Convert to WGS84 (EPSG:4326)
    GenerateCellLatRaster()
    # Calculate parameters dependent on latitude
    CalLatDependParas()
    # Mask, export JSON, import subbasin IDs to MongoDB, etc.
    GenerateSubbasins()


if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    SubbasinDelineation()
