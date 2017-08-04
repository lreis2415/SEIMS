#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Raster Utility Class

    author: Liangjun Zhu
    changlog: 12-04-12 jz - origin version
              16-07-01 lj - reorganized for pygeoc
              17-06-25 lj - check by pylint and reformat by Google style
              17-07-20 lj - add GDALDataType dict, and WhiteBox GAT D8 code
"""
import os
import subprocess

import numpy
from osgeo.gdal import GDT_CInt16, GDT_CInt32, GDT_CFloat32, GDT_CFloat64
from osgeo.gdal import GDT_UInt32, GDT_Int32, GDT_Float32, GDT_Float64
from osgeo.gdal import GDT_Unknown, GDT_Byte, GDT_UInt16, GDT_Int16
from osgeo.gdal import GetDriverByName as gdal_GetDriverByName
from osgeo.gdal import Open as gdal_Open
from osgeo.ogr import Open as ogr_Open
from osgeo.osr import SpatialReference as osr_SpatialReference

from ..utils.utils import MathClass, UtilClass, DEFAULT_NODATA, DELTA

GDALDataType = {0: GDT_Unknown,  # Unknown or unspecified type
                1: GDT_Byte,  # Eight bit unsigned integer
                2: GDT_UInt16,  # Sixteen bit unsigned integer
                3: GDT_Int16,  # Sixteen bit signed integer
                4: GDT_UInt32,  # Thirty two bit unsigned integer
                5: GDT_Int32,  # Thirty two bit signed integer
                6: GDT_Float32,  # Thirty two bit floating point
                7: GDT_Float64,  # Sixty four bit floating point
                8: GDT_CInt16,  # Complex Int16
                9: GDT_CInt32,  # Complex Int32
                10: GDT_CFloat32,  # Complex Float32
                11: GDT_CFloat64  # Complex Float64
                }


class Raster(object):
    """
    Basic Raster Class
    Build-in functions:
        1. get_average()
        2. get_max()
        3. get_min()
        4. get_std()
        5. get_sum()
        6. get_value_by_row_col(row, col)
        7. get_value_by_xy(x, y)
    """

    def __init__(self, n_rows, n_cols, data, nodata_value=None, geotransform=None,
                 srs=None, datatype=GDT_Float32):
        """Constructor
        Args:
            n_rows: row count
            n_cols: col count
            data: 2D array data
            nodata_value: NODATA value, None as default
            geotransform: geographic transformation, None as default
            srs: coordinate system, None as default
            datatype: Raster datatype
        """
        self.nRows = n_rows
        self.nCols = n_cols
        self.data = numpy.copy(data)
        self.noDataValue = nodata_value
        self.geotrans = geotransform
        self.srs = srs
        self.dataType = datatype

        self.dx = geotransform[1]
        self.xMin = geotransform[0]
        self.xMax = geotransform[0] + n_cols * geotransform[1]
        self.yMax = geotransform[3]
        self.yMin = geotransform[3] + n_rows * geotransform[5]
        self.validZone = self.data != self.noDataValue
        self.validValues = numpy.where(self.validZone, self.data, numpy.nan)

    def get_type(self):
        """get datatype as GDALDataType"""
        assert self.dataType in GDALDataType
        return GDALDataType.get(self.dataType)

    def get_average(self):
        """Get average exclude NODATA"""
        return numpy.nanmean(self.validValues)

    def get_max(self):
        """Get maximum exclude NODATA"""
        return numpy.nanmax(self.validValues)

    def get_min(self):
        """Get minimum exclude NODATA"""
        return numpy.nanmin(self.validValues)

    def get_std(self):
        """Get Standard Deviation exclude NODATA"""
        return numpy.nanstd(self.validValues)

    def get_sum(self):
        """Get sum exclude NODATA"""
        return numpy.nansum(self.validValues)

    def get_value_by_row_col(self, row, col):
        """Get raster value by (row, col)
        Args:
            row: row number
            col: col number

        Returns:
            raster value, None if the input are invalid
        """
        if row < 0 or row >= self.nRows or col < 0 or col >= self.nCols:
            raise ValueError("The row or col must be >=0 and less than "
                             "nRows (%d) or nCols (%d)!" % (self.nRows, self.nCols))
        else:
            value = self.data[int(round(row))][int(round(col))]
            if value == self.noDataValue:
                return None
            else:
                return value

    def get_value_by_xy(self, x, y):
        """Get raster value by xy coordinates
        Args:
            x: X Coordinate
            y: Y Coordinate

        Returns:
            raster value, None if the input are invalid
        """
        if x < self.xMin or x > self.xMax or y < self.yMin or y > self.yMax:
            return None
            # raise ValueError("The x or y value must be within the Min and Max!")
        else:
            row = self.nRows - int(numpy.ceil((y - self.yMin) / self.dx))
            col = int(numpy.floor((x - self.xMin) / self.dx))
            value = self.data[row][col]
            if value == self.noDataValue:
                return None
            else:
                return value

    def get_central_coors(self, row, col):
        """Get the coordinates of central grid
        Args:
            row: row number
            col: col number

        Returns:
            XY coordinates. If the row or col are invalid, raise ValueError.
        """
        if row < 0 or row >= self.nRows or col < 0 or col >= self.nCols:
            raise ValueError("The row or col must be >=0 and less than "
                             "nRows (%d) or nCols (%d)!" % (self.nRows, self.nCols))
        else:
            tmpx = self.xMin + (col - 0.5) * self.dx
            tmpy = self.yMax - (row - 0.5) * self.dx
            return tmpx, tmpy


class RasterUtilClass(object):
    """Utility function to handle raster data."""

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def read_raster(raster_file):
        """Read raster by GDAL.
        Args:
            raster_file: raster file path

        Returns:
            Raster object.
        """
        ds = gdal_Open(raster_file)
        band = ds.GetRasterBand(1)
        data = band.ReadAsArray()
        xsize = band.XSize
        ysize = band.YSize

        nodata_value = band.GetNoDataValue()
        geotrans = ds.GetGeoTransform()
        dttype = band.DataType

        srs = osr_SpatialReference()
        srs.ImportFromWkt(ds.GetProjection())
        # print srs.ExportToProj4()
        if nodata_value is None:
            nodata_value = DEFAULT_NODATA
        band = None
        ds = None
        return Raster(ysize, xsize, data, nodata_value, geotrans, srs, dttype)

    @staticmethod
    def get_mask_from_raster(rasterfile, outmaskfile):
        """Generate mask data from a given raster data.
        Args:
            rasterfile: raster file path
            outmaskfile: output mask file path

        Returns:
            Raster object of mask data.
        """
        raster_r = RasterUtilClass.read_raster(rasterfile)
        xsize = raster_r.nCols
        ysize = raster_r.nRows
        nodata_value = raster_r.noDataValue
        srs = raster_r.srs
        x_min = raster_r.xMin
        y_max = raster_r.yMax
        dx = raster_r.dx
        data = raster_r.data

        i_min = ysize - 1
        i_max = 0
        j_min = xsize - 1
        j_max = 0

        for i in range(ysize):
            for j in range(xsize):
                if abs(data[i][j] - nodata_value) > DELTA:
                    i_min = min(i, i_min)
                    i_max = max(i, i_max)
                    j_min = min(j, j_min)
                    j_max = max(j, j_max)

        # print i_min, i_max, j_min, j_max
        y_size_mask = i_max - i_min + 1
        x_size_mask = j_max - j_min + 1
        x_min_mask = x_min + j_min * dx
        y_max_mask = y_max - i_min * dx
        print ("%dx%d -> %dx%d" % (xsize, ysize, x_size_mask, y_size_mask))

        mask = numpy.zeros((y_size_mask, x_size_mask))

        for i in range(y_size_mask):
            for j in range(x_size_mask):
                if abs(data[i + i_min][j + j_min] - nodata_value) > DELTA:
                    mask[i][j] = 1
                else:
                    mask[i][j] = DEFAULT_NODATA

        mask_geotrans = [x_min_mask, dx, 0, y_max_mask, 0, -dx]
        RasterUtilClass.write_gtiff_file(outmaskfile, y_size_mask, x_size_mask, mask,
                                         mask_geotrans, srs, DEFAULT_NODATA, GDT_Int32)
        return Raster(y_size_mask, x_size_mask, mask, DEFAULT_NODATA, mask_geotrans, srs)

    @staticmethod
    def raster_reclassify(srcfile, v_dict, dstfile, gdaltype=GDT_Float32):
        """Reclassify raster by given classifier dict.
        Args:
            srcfile: source raster file
            v_dict: classifier dict
            dstfile: destination file path
            gdaltype: GDT_Float32 as default. Also, it can be integer, e.g., 4 ==> GDT_UInt32
        """
        src_r = RasterUtilClass.read_raster(srcfile)
        src_data = src_r.data
        dst_data = numpy.copy(src_data)
        if gdaltype == GDT_Float32 and src_r.dataType != GDT_Float32:
            gdaltype = src_r.dataType
        no_data = src_r.noDataValue
        new_no_data = DEFAULT_NODATA
        if gdaltype in [GDT_Unknown, GDT_Byte, GDT_UInt16, GDT_UInt32]:
            new_no_data = 0
        if not MathClass.floatequal(new_no_data, src_r.noDataValue):
            if src_r.noDataValue not in v_dict:
                v_dict[src_r.noDataValue] = new_no_data
                no_data = new_no_data

        for k, v in v_dict.iteritems():
            dst_data[src_data == k] = v
        RasterUtilClass.write_gtiff_file(dstfile, src_r.nRows, src_r.nCols, dst_data,
                                         src_r.geotrans, src_r.srs, no_data, gdaltype)

    @staticmethod
    def write_gtiff_file(f_name, n_rows, n_cols, data, geotransform, srs, nodata_value,
                         gdal_type=GDT_Float32):
        """Output Raster to GeoTiff format file.
        Args:
            f_name: output gtiff file name
            n_rows: Row count
            n_cols: Col count
            data: 2D array data
            geotransform: geographic transformation
            srs: coordinate system
            nodata_value: nodata value
            gdal_type: output raster data type, GDT_Float32 as default
        """
        driver = gdal_GetDriverByName("GTiff")
        ds = driver.Create(f_name, n_cols, n_rows, 1, gdal_type)
        ds.SetGeoTransform(geotransform)
        ds.SetProjection(srs.ExportToWkt())
        ds.GetRasterBand(1).SetNoDataValue(nodata_value)
        ds.GetRasterBand(1).WriteArray(data)
        ds = None

    @staticmethod
    def write_asc_file(filename, data, xsize, ysize, geotransform, nodata_value):
        """Output Raster to ASCII file.
        Args:
            filename: output ASCII filename
            data: 2D array data
            xsize: Col count
            ysize: Row count
            geotransform: geographic transformation
            nodata_value: nodata value
        """
        header = """NCOLS %d
    NROWS %d
    XLLCENTER %f
    YLLCENTER %f
    CELLSIZE %f
    NODATA_VALUE %f
    """ % (xsize, ysize, geotransform[0] + 0.5 * geotransform[1],
           geotransform[3] - (ysize - 0.5) * geotransform[1],
           geotransform[1], nodata_value)

        f = open(filename, 'w')
        f.write(header)
        for i in range(0, ysize):
            for j in range(0, xsize):
                f.write(str(data[i][j]) + "\t")
            f.write("\n")
        f.close()

    @staticmethod
    def raster_to_gtiff(tif, geotif, gdal_type=GDT_Float32):
        """Converting Raster format to GeoTIFF.
        Args:
            tif: source raster file path
            geotif: output raster file path
            gdal_type: GDT_Float32 as default
        """
        rst_file = RasterUtilClass.read_raster(tif)
        if gdal_type != rst_file.dataType and rst_file.dataType in GDALDataType:
            gdal_type = rst_file.dataType
        RasterUtilClass.write_gtiff_file(geotif, rst_file.nRows, rst_file.nCols, rst_file.data,
                                         rst_file.geotrans, rst_file.srs, rst_file.noDataValue,
                                         gdal_type)

    @staticmethod
    def raster_to_asc(raster_f, asc_f):
        """Converting Raster format to ASCII raster.
        Args:
            raster_f: raster file
            asc_f: output ASCII file
        """
        raster_r = RasterUtilClass.read_raster(raster_f)
        RasterUtilClass.write_asc_file(asc_f, raster_r.data, raster_r.nCols, raster_r.nRows,
                                       raster_r.geotrans, raster_r.noDataValue)

    @staticmethod
    def raster_statistics(raster_file):
        """Get basic statistics of raster data.
        Args:
            raster_file: raster file path

        Returns:
            min, max, mean, std
        """
        ds = gdal_Open(raster_file)
        band = ds.GetRasterBand(1)
        minv, maxv, meanv, std = band.ComputeStatistics(False)
        return minv, maxv, meanv, std

    @staticmethod
    def split_raster(rs, split_shp, field_name, temp_dir):
        """Split raster by given shapefile and field name.
        Args:
            rs: origin raster file
            split_shp: boundary (ESRI Shapefile) used to spilt raster
            field_name: field name identify the spilt value
            temp_dir: directory to store the spilt rasters
        """
        UtilClass.rmmkdir(temp_dir)
        ds = ogr_Open(split_shp)
        lyr = ds.GetLayer(0)
        lyr.ResetReading()
        ft = lyr.GetNextFeature()
        while ft:
            cur_field_name = ft.GetFieldAsString(field_name)
            for r in rs:
                cur_file_name = r.split(os.sep)[-1]
                outraster = temp_dir + os.sep + \
                            cur_file_name.replace('.tif', '_%s.tif' %
                                                  cur_field_name.replace(' ', '_'))
                subprocess.call(['gdalwarp', r, outraster, '-cutline', split_shp,
                                 '-crop_to_cutline', '-cwhere',
                                 "'%s'='%s'" % (field_name, cur_field_name), '-dstnodata',
                                 '-9999'])
            ft = lyr.GetNextFeature()
        ds = None

    @staticmethod
    def get_negative_dem(raw_dem, neg_dem):
        """Get negative DEM data."""
        origin = RasterUtilClass.read_raster(raw_dem)
        max_v = numpy.max(origin.data)
        temp = origin.data < 0
        neg = numpy.where(temp, origin.noDataValue, max_v - origin.data)
        RasterUtilClass.write_gtiff_file(neg_dem, origin.nRows, origin.nCols, neg, origin.geotrans,
                                         origin.srs, origin.noDataValue, origin.dataType)
