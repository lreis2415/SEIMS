#! /usr/bin/env python
# coding=utf-8
# @Subbasin delineation based on TauDEM, as well as calculation of latitude dependent parameters
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
# @Note: Improve calculation efficiency by numpy
#
import shutil

from gdal import GDT_Int32, GDT_Float32
import osr
import ogr
import numpy
from pygeoc.raster.raster import RasterUtilClass
from pygeoc.vector.vector import VectorUtilClass
from pygeoc.hydro.TauDEM import TauDEM
from pygeoc.hydro.postTauDEM import D8Util, DinfUtil, StreamnetUtil
from pygeoc.utils.utils import UtilClass

from utility import LoadConfiguration, status_output, DEFAULT_NODATA
from config import *
from sd_hillslope import delineate_hillslopes_downstream_method


def output_wgs84_geojson():
    """Convert ESRI shapefile to GeoJson based on WGS84 coordinate."""
    src_srs = RasterUtilClass.ReadRaster(dem).srs
    proj_srs = src_srs.ExportToProj4()
    if not proj_srs:
        raise ValueError("The source raster %s has not coordinate, which is required!" % dem)
    # print proj_srs
    wgs84_srs = "EPSG:4326"
    shp_dir = WORKING_DIR + os.sep + DIR_NAME_GEOSHP
    geo_json_dict = {GEOJSON_REACH : shp_dir + os.sep + reachesOut,
                     GEOJSON_SUBBSN: shp_dir + os.sep + subbasinVec,
                     GEOJSON_BASIN : shp_dir + os.sep + basinVec,
                     GEOJSON_OUTLET: shp_dir + os.sep + outletVec}
    for jsonName in geo_json_dict:
        jsonfile = shp_dir + os.sep + jsonName
        VectorUtilClass.convert2geojson(jsonfile, proj_srs, wgs84_srs, geo_json_dict.get(jsonName))


def original_delineation():
    """Original Delineation subbasin by calling TauDEM functions"""
    # Check directories
    UtilClass.mkdir(WORKING_DIR)
    log_dir = WORKING_DIR + os.sep + DIR_NAME_LOG
    UtilClass.mkdir(log_dir)
    status_file = log_dir + os.sep + FN_STATUS_DELINEATION
    f_status = open(status_file, 'w')
    tau_dir = WORKING_DIR + os.sep + DIR_NAME_TAUDEM
    UtilClass.mkdir(tau_dir)
    # begin calling TauDEM
    status_output("Fill DEM...", 10, f_status)
    TauDEM.Fill(np, tau_dir, dem, filledDem, MPIEXEC_DIR, CPP_PROGRAM_DIR)
    status_output("Calculating D8 and Dinf flow direction...", 20, f_status)
    TauDEM.FlowDirD8(np, tau_dir, filledDem, flowDir, slope, MPIEXEC_DIR, CPP_PROGRAM_DIR)
    TauDEM.FlowDirDinf(np, tau_dir, filledDem, flowDirDinf, slopeDinf, MPIEXEC_DIR, CPP_PROGRAM_DIR)
    DinfUtil.outputcompresseddinf(flowDirDinf, dirCodeDinf, weightDinf)
    status_output("D8 flow accumulation...", 40, f_status)
    TauDEM.FlowAccD8(np, tau_dir, flowDir, acc, None, None, MPIEXEC_DIR, CPP_PROGRAM_DIR)
    status_output("Generating stream raster initially...", 50, f_status)
    if D8AccThreshold > 0:
        TauDEM.StreamRaster(np, tau_dir, acc, streamRaster, D8AccThreshold, MPIEXEC_DIR, CPP_PROGRAM_DIR)
    else:
        accD8 = tau_dir + os.sep + acc
        minAccum, maxAccum, meanAccum, STDAccum = RasterUtilClass.RasterStatistics(accD8)
        TauDEM.StreamRaster(np, tau_dir, acc, streamRaster, meanAccum, MPIEXEC_DIR, CPP_PROGRAM_DIR)
    status_output("Moving outlet to stream...", 60, f_status)
    TauDEM.MoveOutlet(np, tau_dir, flowDir, streamRaster, outlet_file, modifiedOutlet, MPIEXEC_DIR, CPP_PROGRAM_DIR)

    if D8AccThreshold <= 0:
        status_output("Generating stream skeleton...", 65, f_status)
        TauDEM.StreamSkeleton(np, tau_dir, filledDem, streamSkeleton, MPIEXEC_DIR, CPP_PROGRAM_DIR)
        status_output("Flow accumulation with outlet...", 70, f_status)
        TauDEM.FlowAccD8(np, tau_dir, flowDir, accWithWeight, modifiedOutlet, streamSkeleton, MPIEXEC_DIR,
                         CPP_PROGRAM_DIR)
        status_output("Drop analysis to select optimal threshold...", 75, f_status)
        minAccum, maxAccum, meanAccum, STDAccum = RasterUtilClass.RasterStatistics(accWithWeight)
        if meanAccum - STDAccum < 0:
            minthresh = meanAccum
        else:
            minthresh = meanAccum - STDAccum
        maxthresh = meanAccum + STDAccum
        numthresh = 20
        logspace = 'true'
        drpFile = 'drp.txt'
        TauDEM.DropAnalysis(np, tau_dir, filledDem, flowDir, accWithWeight, accWithWeight, modifiedOutlet, minthresh,
                            maxthresh, numthresh, logspace, drpFile, MPIEXEC_DIR, CPP_PROGRAM_DIR)
        drpf = open(drpFile, "r")
        tempContents = drpf.read()
        (beg, Threshold) = tempContents.rsplit(' ', 1)
        print (Threshold)
        drpf.close()
        status_output("Generating stream raster...", 80, f_status)
        TauDEM.StreamRaster(np, tau_dir, accWithWeight, streamRaster, float(Threshold), MPIEXEC_DIR, CPP_PROGRAM_DIR)
    else:
        status_output("Flow accumulation with outlet...", 70, f_status)
        TauDEM.FlowAccD8(np, tau_dir, flowDir, acc, modifiedOutlet, None, MPIEXEC_DIR, CPP_PROGRAM_DIR)
        status_output("Generating stream raster...", 80, f_status)
        TauDEM.StreamRaster(np, tau_dir, acc, streamRaster, D8AccThreshold, MPIEXEC_DIR, CPP_PROGRAM_DIR)
    status_output("Generating stream net...", 90, f_status)
    if D8AccThreshold <= 0:
        tmpAcc = accWithWeight
    else:
        tmpAcc = acc
    TauDEM.StreamNet(np, tau_dir, filledDem, flowDir, tmpAcc, streamRaster, modifiedOutlet, streamOrder, chNetwork,
                     chCoord, streamNet, subbasin, MPIEXEC_DIR, CPP_PROGRAM_DIR)
    status_output("Calculating distance to stream (D8)...", 95, f_status)
    TauDEM.D8DistDownToStream(np, tau_dir, flowDir, filledDem, streamRaster, dist2StreamD8, D8DownMethod, 1,
                              MPIEXEC_DIR, CPP_PROGRAM_DIR)
    status_output("Original subbasin delineation is finished!", 100, f_status)
    f_status.close()


def mask_raster_cpp(maskfile, originalfiles, outputfiles, default_values, configfile):
    """Call mask_raster program (cpp version) to mask raster"""
    # write mask configuration file
    n = len(originalfiles)
    # write mask config file
    f = open(configfile, 'w')
    f.write(maskfile + "\n")
    f.write("%d\n" % (n,))
    for i in range(n):
        s = "%s\t%d\t%s\n" % (originalfiles[i], default_values[i], outputfiles[i])
        f.write(s)
    f.close()
    # run command
    UtilClass.runcommand('"%s/mask_raster" %s' % (CPP_PROGRAM_DIR, configfile))


def mask_origin_delineated_data():
    """Mask the original delineated data by Subbasin raster."""
    tau_dir = WORKING_DIR + os.sep + DIR_NAME_TAUDEM
    subbasin_tau_file = tau_dir + os.sep + subbasin
    geodata2dbdir = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB
    UtilClass.mkdir(geodata2dbdir)
    mask_file = geodata2dbdir + os.sep + mask_to_ext
    RasterUtilClass.GetMaskFromRaster(subbasin_tau_file, mask_file)
    # Total 12 raster files
    original_files = [subbasin, flowDir, streamRaster, slope, filledDem, acc, streamOrder,
                      flowDirDinf, dirCodeDinf, slopeDinf, weightDinf, dist2StreamD8]
    original_files = [(tau_dir + os.sep + item) for item in original_files]

    # output masked files
    output_files = [subbasinM, flowDirM, streamRasterM]
    output_files = [(tau_dir + os.sep + item) for item in output_files]

    geodata2dbdir = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB
    UtilClass.mkdir(geodata2dbdir)
    terrain_files = [slopeM, filleddemM, accM, streamOrderM,
                     flowDirDinfM, dirCodeDinfM, slopeDinfM, weightDinfM, dist2StreamD8M]
    terrain_files = [(geodata2dbdir + os.sep + item) for item in terrain_files]

    output_files += terrain_files

    default_values = []
    for i in range(len(original_files)):
        default_values.append(DEFAULT_NODATA)

    # other input rasters need to be masked
    # soil and landuse
    FileClass.checkfileexists(soilSEQNFile)
    FileClass.checkfileexists(landuseOriginFile)
    original_files.append(soilSEQNFile)
    output_files.append(geodata2dbdir + os.sep + soiltypeMFile)
    default_values.append(defaultSoil)
    original_files.append(landuseOriginFile)
    output_files.append(geodata2dbdir + os.sep + landuseMFile)
    default_values.append(defaultLanduse)
    # management fields
    if mgtFieldFile is not None and FileClass.isfileexists(mgtFieldFile):
        original_files.append(mgtFieldFile)
        output_files.append(geodata2dbdir + os.sep + mgtFieldMFile)
        default_values.append(DEFAULT_NODATA)

    config_file = WORKING_DIR + os.sep + DIR_NAME_LOG + os.sep + FN_CONFIG_MASKRASTERS
    # run mask operation
    print ("Mask original delineated data by Subbasin raster...")
    mask_raster_cpp(mask_file, original_files, output_files, default_values, config_file)


def post_process_of_delineated_data():
    """Do some necessary transfer for subbasin, stream, and flow direction raster."""
    # inputs
    tau_dir = WORKING_DIR + os.sep + DIR_NAME_TAUDEM
    stream_net_file = tau_dir + os.sep + streamNet
    subbasin_file = tau_dir + os.sep + subbasinM
    flow_dir_file_tau = tau_dir + os.sep + flowDirM
    stream_raster_file = tau_dir + os.sep + streamRasterM
    # outputs
    # -- shapefile
    shp_dir = WORKING_DIR + os.sep + DIR_NAME_GEOSHP
    UtilClass.mkdir(shp_dir)
    # ---- outlet, copy from DIR_NAME_TAUDEM
    FileClass.copyfiles(WORKING_DIR + os.sep + DIR_NAME_TAUDEM + os.sep + modifiedOutlet,
                        shp_dir + os.sep + outletVec)
    # ---- reaches
    output_reach_file = shp_dir + os.sep + reachesOut
    # ---- subbasins
    subbasin_vector_file = shp_dir + os.sep + subbasinVec
    # -- raster file
    geodata2dbdir = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB
    output_subbasin_file = geodata2dbdir + os.sep + subbasinOut
    output_flow_dir_file = geodata2dbdir + os.sep + flowDirOut
    output_stream_link_file = geodata2dbdir + os.sep + streamLinkOut
    output_hillslope_file = geodata2dbdir + os.sep + hillSlopeOut

    id_map = StreamnetUtil.serializestreamnet(stream_net_file, output_reach_file)
    RasterUtilClass.RasterReclassify(subbasin_file, id_map, output_subbasin_file, GDT_Int32)
    StreamnetUtil.assignstreamidraster(stream_raster_file, output_subbasin_file, output_stream_link_file)

    # Convert D8 encoding rule to ArcGIS
    if isTauDEM:
        shutil.copy(flow_dir_file_tau, output_flow_dir_file)
    else:
        D8Util.converttoarcgiscode(flow_dir_file_tau, output_flow_dir_file)

    # convert raster to shapefile (for subbasin and basin)
    print "Generating subbasin vector..."
    VectorUtilClass.raster2shp(output_subbasin_file, subbasin_vector_file, "subbasin", FLD_SUBBASINID)
    mask_file = geodata2dbdir + os.sep + mask_to_ext
    basin_vector = shp_dir + os.sep + basinVec
    print "Generating basin vector..."
    VectorUtilClass.raster2shp(mask_file, basin_vector, "basin", FLD_BASINID)
    # delineate hillslope
    delineate_hillslopes_downstream_method(output_stream_link_file, flow_dir_file_tau, output_hillslope_file)


def generate_lat_raster():
    geodata2dbdir = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB
    dem_file = geodata2dbdir + os.sep + filleddemM
    ds = RasterUtilClass.ReadRaster(dem_file)
    src_srs = ds.srs
    if not src_srs.ExportToProj4():
        raise ValueError("The source raster %s has not coordinate, which is required!" % dem_file)
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

    def cal_cell_lat(row, col):
        return upLat - (row + 0.5) * deltaLat

    dataLat = numpy.fromfunction(cal_cell_lat, (rows, cols))
    dataLat = numpy.where(ds.validZone, dataLat, ds.data)
    RasterUtilClass.WriteGTiffFile(geodata2dbdir + os.sep + cellLat, rows, cols, dataLat, ds.geotrans, ds.srs,
                                   ds.noDataValue, GDT_Float32)


def SubbasinDelineation():
    """Subbasin delineation workflow"""
    # 1. Originally delineated by TauDEM
    original_delineation()
    # 2. Mask delineated raster by subbasin
    mask_origin_delineated_data()
    # 3. Post processing, such as serialize strean ID, mask dataset etc.
    post_process_of_delineated_data()
    # 4. Convert current coordinate to WGS84 and convert shapefile to GeoJson.
    output_wgs84_geojson()
    # 5. Convert to WGS84 coordinate and output latitude raster.
    generate_lat_raster()


if __name__ == "__main__":
    LoadConfiguration(getconfigfile())
    # SubbasinDelineation()
    # generate_lat_raster()
    # mask_origin_delineated_data()
    post_process_of_delineated_data()
