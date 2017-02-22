#! /usr/bin/env python
# coding=utf-8
# @Extract landuse parameters
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#
import re
import sqlite3

import numpy
from gdal import GDT_Float32
from pygeoc.utils.utils import UtilClass, MathClass
from pygeoc.raster import RasterUtilClass

from config import *
from db_sqlite import reConstructSQLiteDB
from utility import LoadConfiguration, status_output, ReadDataItemsFromTxt
from utility import DEFAULT_NODATA, UTIL_ZERO


class LanduseUtilClass(object):
    """Landuse/Landcover related utility functions."""

    def __init__(self):
        pass

    @staticmethod
    def export_landuse_lookup_files(sqlite3_dbname, property_namelist, str_sql, dst_dir):
        property_map = {}
        conn = sqlite3.connect(sqlite3_dbname)
        cursor = conn.cursor()

        cursor.execute(str_sql)
        property_namelist.append("USLE_P")
        for row in cursor:
            # print row
            id = int(row[0])
            value_map = {}
            for i in range(len(property_namelist)):
                pName = property_namelist[i]
                if pName == "USLE_P":  # Currently, USLE_P is set as 1 for all landuse.
                    value_map[pName] = 1
                else:
                    if pName == "Manning":
                        value_map[pName] = row[i + 1] * 10
                    else:
                        value_map[pName] = row[i + 1]
            property_map[id] = value_map

        n = len(property_map)
        UtilClass.mkdir(dst_dir)
        os.chdir(dst_dir)
        for propertyName in property_namelist:
            if not os.path.exists(DIR_NAME_LOOKUP):
                os.mkdir(DIR_NAME_LOOKUP)
            f = open("%s/%s.txt" % (dst_dir, propertyName,), 'w')
            f.write("%d\n" % n)
            for id in property_map:
                s = "%d %f\n" % (id, property_map[id][propertyName])
                f.write(s)
            f.close()

    @staticmethod
    def lookup_landuse_parameters(config_file, dst_dir, landuse_file, lookup_dir, landuse_attr_list,
                                  default_landuse_id):
        """
        Reclassify landuse parameters by lookup table.
        TODO(LJ): this function should be replaced by replaceByDict() function!
        """
        # prepare reclassify configuration file
        fReclassLu = open(config_file, 'w')
        fReclassLu.write("%s\t%d\n" % (landuse_file, default_landuse_id))
        fReclassLu.write("%s\n" % lookup_dir)
        fReclassLu.write(dst_dir + "\n")
        n = len(landuse_attr_list)
        fReclassLu.write("%d\n" % n)
        fReclassLu.write("\n".join(landuse_attr_list))
        fReclassLu.close()
        s = '"%s/reclassify" %s' % (CPP_PROGRAM_DIR, config_file)
        UtilClass.runcommand(s)

    @staticmethod
    def initialize_landcover_parameters(landcover_file, landcover_initial_fields_file, dst_dir):
        LC_dataItems = ReadDataItemsFromTxt(landcover_initial_fields_file)
        # print LC_dataItems
        fieldNames = LC_dataItems[0]
        LUID = -1
        for i in range(len(fieldNames)):
            if StringClass.stringmatch(fieldNames[i], 'LANDUSE_ID'):
                LUID = i
                break
        dataItems = LC_dataItems[1:]
        replaceDicts = {}
        for item in dataItems:
            for i in range(len(item)):
                if i != LUID:
                    if fieldNames[i].upper() not in replaceDicts.keys():
                        replaceDicts[fieldNames[i].upper()] = {float(item[LUID]): float(item[i])}
                    else:
                        replaceDicts[fieldNames[i].upper()][float(item[LUID])] = float(item[i])
        # print replaceDicts

        # Generate GTIFF
        for item in replaceDicts.keys():
            filename = dst_dir + os.sep + item + '.tif'
            print (filename)
            RasterUtilClass.RasterReclassify(landcover_file, replaceDicts[item], filename)
        return replaceDicts['LANDCOVER'].values()

    @staticmethod
    def read_crop_lookup_table(crop_lookup_file):
        FileClass.checkfileexists(crop_lookup_file)
        f = open(crop_lookup_file)
        lines = f.readlines()
        f.close()
        attrDic = {}
        fields = [item.replace('"', '')
                  for item in re.split('\t|\n', lines[0]) if item is not '']
        n = len(fields)
        for i in range(n):
            attrDic[fields[i]] = {}
        for line in lines[2:]:
            items = [item.replace('"', '')
                     for item in re.split('\t', line) if item is not '']
            id = int(items[0])

            for i in range(n):
                dic = attrDic[fields[i]]
                try:
                    dic[id] = float(items[i])
                except ValueError:
                    dic[id] = items[i]
        return attrDic

    @staticmethod
    def lookup_landcover_parameters(landcover_file, landcover_initial_fields_file, landcover_lookup_file, attr_names,
                                    dst_dir):
        LandCoverCodes = LanduseUtilClass.initialize_landcover_parameters(landcover_file,
                                                                          landcover_initial_fields_file, dst_dir)
        attrMap = LanduseUtilClass.read_crop_lookup_table(landcover_lookup_file)
        n = len(attr_names)
        replaceDicts = []
        dstCropTifs = []
        for i in range(n):
            curAttr = attr_names[i]
            curDict = {}
            dic = attrMap[curAttr]
            for code in LandCoverCodes:
                if code == DEFAULT_NODATA:
                    continue
                if code not in curDict.keys():
                    curDict[code] = dic[code]
            replaceDicts.append(curDict)
            dstCropTifs.append(dst_dir + os.sep + curAttr + '.tif')
        # print replaceDicts
        # print(len(replaceDicts))
        # print dstCropTifs
        # print(len(dstCropTifs))
        # Generate GTIFF
        for i in range(len(dstCropTifs)):
            # print dstCropTifs[i]
            RasterUtilClass.RasterReclassify(dst_dir + os.sep + cropMFile, replaceDicts[i], dstCropTifs[i])

    @staticmethod
    def generate_cn2(dbname, landuse_file, hydrogroup_file, cn2_filename):
        """Generate CN raster."""
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
        luR = RasterUtilClass.ReadRaster(landuse_file)
        dataLanduse = luR.data
        xsize = luR.nCols
        ysize = luR.nRows
        noDataValue = luR.noDataValue

        hgR = RasterUtilClass.ReadRaster(hydrogroup_file)
        dataHg = hgR.data

        def cal_cn2(landuseID, hg):
            landuseID = int(landuseID)
            if landuseID < 0 or MathClass.floatequal(landuseID, noDataValue):
                return DEFAULT_NODATA
            else:
                hg = int(hg) - 1
                return cn2_map[landuseID][hg]

        calCN2_numpy = numpy.frompyfunc(cal_cn2, 2, 1)
        data_prop = calCN2_numpy(dataLanduse, dataHg)
        RasterUtilClass.WriteGTiffFile(cn2_filename, ysize, xsize, data_prop, luR.geotrans, luR.srs, noDataValue,
                                       GDT_Float32)

    @staticmethod
    def generate_runoff_coefficent(sqliteFile, landuse_file, slope_file, soil_texture_file, runoff_coeff_file):
        """Generate potential runoff coefficient."""
        # read landuselookup table from sqlite
        prcFields = ["PRC_ST%d" % (i,) for i in range(1, 13)]
        scFields = ["SC_ST%d" % (i,) for i in range(1, 13)]
        sqlLanduse = 'select LANDUSE_ID, %s, %s from LanduseLookup' % (','.join(prcFields), ','.join(scFields))

        conn = sqlite3.connect(sqliteFile)
        cursor = conn.cursor()
        cursor.execute(sqlLanduse)

        runoff_c0 = {}
        runoff_s0 = {}
        for row in cursor:
            id = int(row[0])
            runoff_c0[id] = [float(item) for item in row[1:13]]
            runoff_s0[id] = [float(item) for item in row[13:25]]

        cursor.close()
        conn.close()
        # end read data

        landu_raster = RasterUtilClass.ReadRaster(landuse_file)
        landu_data = landu_raster.data
        noDataValue1 = landu_raster.noDataValue
        xsize = landu_raster.nCols
        ysize = landu_raster.nRows
        noDataValue2 = landu_raster.noDataValue

        slo_data = RasterUtilClass.ReadRaster(slope_file).data
        soilTextureArray = RasterUtilClass.ReadRaster(soil_texture_file).data
        idOmited = []

        def coefCal(landuID, soilTexture, slope):
            if abs(landuID - noDataValue1) < UTIL_ZERO or int(landuID) < 0:
                return noDataValue2
            if not int(landuID) in runoff_c0.keys():
                if not int(landuID) in idOmited:
                    print 'The landuse ID: %d does not exist.' % (int(landuID),)
                    idOmited.append(int(landuID))
            stid = int(soilTexture) - 1
            c0 = runoff_c0[int(landuID)][stid]
            s0 = runoff_s0[int(landuID)][stid] / 100.
            slp = slope

            if slp + s0 < 0.0001:
                return c0
            coef1 = (1 - c0) * slp / (slp + s0)
            coef2 = c0 + coef1
            # TODO, Check if it is (landuID >= 98), by lj
            if int(landuID) == 106 or int(landuID) == 107 or int(landuID) == 105:
                return coef2 * (1 - imperviousPercInUrbanCell) + imperviousPercInUrbanCell
            else:
                return coef2

        coefCal_numpy = numpy.frompyfunc(coefCal, 3, 1)
        coef = coefCal_numpy(landu_data, soilTextureArray, slo_data)

        RasterUtilClass.WriteGTiffFile(runoff_coeff_file, ysize, xsize, coef,
                                       landu_raster.geotrans, landu_raster.srs, noDataValue2, GDT_Float32)


def landuse_parameters_extraction():
    """Landuse spatial parameters extraction."""
    geodata2dbdir = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB
    status_file = WORKING_DIR + os.sep + DIR_NAME_LOG + os.sep + FN_STATUS_EXTRACTLANDUSEPARAM
    landuse_file = geodata2dbdir + os.sep + landuseMFile
    f = open(status_file, 'w')
    # 1. Generate landuse lookup tables
    status_output("Generating landuse lookup tables from Sqlite database...", 10, f)
    str_sql = 'select landuse_id, ' + ','.join(LANDUSE_ATTR_LIST) + ' from LanduseLookup'
    sqlite3db = WORKING_DIR + os.sep + DIR_NAME_IMPORT2DB + os.sep + sqlite_file
    if not FileClass.isfileexists(sqlite3db):
        reConstructSQLiteDB()
    lookup_dir = WORKING_DIR + os.sep + DIR_NAME_LOOKUP
    LanduseUtilClass.export_landuse_lookup_files(sqlite3db, LANDUSE_ATTR_LIST, str_sql, lookup_dir)
    # 2. Reclassify landuse parameters by lookup tables
    status_output("Generating landuse attributes...", 20, f)
    lookup_lu_config_file = WORKING_DIR + os.sep + DIR_NAME_LOG + os.sep + FN_CONFIG_RECLASSIFYLU
    LanduseUtilClass.lookup_landuse_parameters(lookup_lu_config_file, geodata2dbdir, landuse_file, lookup_dir,
                                               LANDUSE_ATTR_LIST, defaultLanduse)
    # 3. Generate crop parameters
    if genCrop:
        status_output("Generating crop/landcover attributes...", 30, f)
        crop_lookup_file = TXT_DB_DIR + os.sep + CROP_FILE
        LanduseUtilClass.lookup_landcover_parameters(landuse_file, landcoverInitFile, crop_lookup_file, CROP_ATTR_LIST,
                                                     geodata2dbdir)
    # 4. Generate Curve Number according to landuse
    if genCN:
        status_output("Calculating CN numbers...", 40, f)
        hg_file = geodata2dbdir + os.sep + hydroGroup
        cn2_filename = geodata2dbdir + os.sep + CN2File
        LanduseUtilClass.generate_cn2(sqlite3db, landuse_file, hg_file, cn2_filename)
    # 5. Generate runoff coefficient
    if genRunoffCoef:
        status_output("Calculating potential runoff coefficient...", 50, f)
        slope_file = geodata2dbdir + os.sep + slopeM
        soil_texture_raster = geodata2dbdir + os.sep + soilTexture
        runoff_coef_file = geodata2dbdir + os.sep + runoff_coefFile
        LanduseUtilClass.generate_runoff_coefficent(sqlite3db, landuse_file, slope_file, soil_texture_raster,
                                                    runoff_coef_file)
    status_output("Landuse/Landcover related spatial parameters extracted done!", 100, f)
    f.close()


if __name__ == '__main__':
    LoadConfiguration(getconfigfile())
    landuse_parameters_extraction()
