
# -*- coding: utf-8 -*-
"""Raster Utility Class.
用于创建栅格数据对象并进行简单操作，如另存为ASCII格式栅格、栅格重分类等。

    author: Liangjun Zhu

    changlog:

     - 12-04-12 jz - origin version.
     - 16-07-01 lj - reorganized for pygeoc.
     - 17-06-25 lj - check by pylint and reformat by Google style.
     - 17-07-20 lj - add GDALDataType dict, and WhiteBox GAT D8 code.
     - 17-11-21 yw - add raster_binarization, raster_erosion, raster_dilation, openning, closing.
"""
from __future__ import absolute_import, unicode_literals

from future.utils import iteritems
from builtins import range

import os
import subprocess
from io import open

import numpy
from osgeo.gdal import GDT_CInt16, GDT_CInt32, GDT_CFloat32, GDT_CFloat64
from osgeo.gdal import GDT_UInt32, GDT_Int32, GDT_Float32, GDT_Float64
from osgeo.gdal import GDT_Unknown, GDT_Byte, GDT_UInt16, GDT_Int16
from osgeo.gdal import GetDriverByName as gdal_GetDriverByName
from osgeo.gdal import Open as gdal_Open
from osgeo.ogr import Open as ogr_Open
from osgeo.osr import SpatialReference as osr_SpatialReference

from pygeoc.utils import MathClass, UtilClass, FileClass, DEFAULT_NODATA, DELTA
from pygeoc.utils import is_string

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
"""dict: GDAL DataType

    +--------------+----------------+---------------------------------+
    | Type         | GDAL Datatype  | Description                     |
    +==============+================+=================================+
    | 0            | GDT_Unknown    | Unknown or unspecified type     |
    +--------------+----------------+---------------------------------+
    | 1            | GDT_Byte       | Eight bit unsigned integer      |
    +--------------+----------------+---------------------------------+
    | 2            | GDT_UInt16     | Sixteen bit unsigned integer    |
    +--------------+----------------+---------------------------------+
    | 3            | GDT_Int16      | Sixteen bit signed integer      |
    +--------------+----------------+---------------------------------+
    | 4            | GDT_UInt32     | Thirty two bit unsigned integer |
    +--------------+----------------+---------------------------------+
    | 5            | GDT_Int32      | Thirty two bit signed integer   |
    +--------------+----------------+---------------------------------+
    | 6            | GDT_Float32    | Thirty two bit floating point   |
    +--------------+----------------+---------------------------------+
    | 7            | GDT_Float64    | Sixty four bit floating point   |
    +--------------+----------------+---------------------------------+
    | 8            | GDT_CInt16     | Complex Int16                   |
    +--------------+----------------+---------------------------------+
    | 9            | GDT_CInt32     | Complex Int32                   |
    +--------------+----------------+---------------------------------+
    | 10           | GDT_CFloat32   | Complex Float32                 |
    +--------------+----------------+---------------------------------+
    | 11           | GDT_CFloat64   | Complex Float64                 |
    +--------------+----------------+---------------------------------+
    
"""


class Raster(object):
    """Basic Raster Class.

    Args:
        n_rows: row count.
        n_cols: col count.
        data: 2D array data.
        nodata_value: NODATA value, None as default.
        geotransform: geographic transformation, None as default.
        srs: coordinate system, None as default.
        datatype(:obj:`pygeoc.raster.GDALDataType`): Raster datatype.

    Attributes:
        nRows (int): Row number.
        nCols (int): Column number.
        data (:obj:`numpy.array`): 2D array raster data.
        noDataValue (float): NoData value.
        geotrans (list): geographic transformation list.
        srs (:obj:`osgeo.osr.SpatialReference`): Spatial reference.
        dataType (:obj:`pygeoc.raster.GDALDataType`): Raster datatype.
        dx (float): cell size.
        xMin (float): left X coordinate.
        xMax (float): right X coordinate.
        yMin (float): lower Y coordinate.
        yMax (float): upper Y coordinate.
        validZone (:obj:`numpy.array`): 2D boolean array that NoDataValue is False.
        validValues (:obj:`numpy.array`): 2D raster array with None in NoDataValue.

    Examples:
        The common usage is read raster data from a raster file (e.g., geotiff) and get the
        Raster instance.

        >>> from pygeoc.raster import RasterUtilClass
        >>> rst_file = r'tests/data/Jamaica_dem.tif'
        >>> rst_obj = RasterUtilClass.read_raster(rst_file)
        >>> print(rst_obj)
        <pygeoc.raster.Raster object at 0x...>

    See Also:
        :func:`pygeoc.raster.RasterUtilClass.read_raster`
    """

    def __init__(self, n_rows, n_cols, data, nodata_value=None, geotransform=None,
                 srs=None, datatype=GDT_Float32):
        """Constructor."""
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
        """get datatype as GDALDataType.

        Returns:
            dataType

        See Also:
            :obj:`pygeoc.raster.GDALDataType`
        """
        assert self.dataType in GDALDataType
        return GDALDataType.get(self.dataType)

    def get_average(self):
        """Get average exclude NODATA."""
        return numpy.nanmean(self.validValues)

    def get_max(self):
        """Get maximum exclude NODATA."""
        return numpy.nanmax(self.validValues)

    def get_min(self):
        """Get minimum exclude NODATA."""
        return numpy.nanmin(self.validValues)

    def get_std(self):
        """Get Standard Deviation exclude NODATA."""
        return numpy.nanstd(self.validValues)

    def get_sum(self):
        """Get sum exclude NODATA."""
        return numpy.nansum(self.validValues)

    def get_sum_non_negative(self):
        """Get sum of values greater than or equal to 0."""

        validValues_non_negative = self.validValues[self.validValues >= 0]
        return numpy.nansum(validValues_non_negative)
    def get_value_by_row_col(self, row, col):
        """Get raster value by (row, col).

        Args:
            row: row number.
            col: col number.

        Returns:
            raster value, None if the input are invalid.
        """
        if row < 0 or row >= self.nRows or col < 0 or col >= self.nCols:
            raise ValueError("The row or col must be >=0 and less than "
                             "nRows (%d) or nCols (%d)!" % (self.nRows, self.nCols))
        else:
            value = self.data[int(round(row))][int(round(col))]
            if MathClass.floatequal(value, self.noDataValue):
                return None
            else:
                return value

    def get_value_by_xy(self, x, y):
        """Get raster value by xy coordinates.

        Args:
            x: X Coordinate.
            y: Y Coordinate.

        Returns:
            raster value, None if the input are invalid.
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
        """Get the coordinates of central grid.

        Args:
            row: row number, range from 0 to (nRows - 1).
            col: col number, range from 0 to (nCols - 1).

        Returns:
            XY coordinates. If the row or col are invalid, raise ValueError.
        """
        if row < 0 or row >= self.nRows or col < 0 or col >= self.nCols:
            raise ValueError("The row (%d) or col (%d) must be >=0 and less than "
                             "nRows (%d) or nCols (%d)!" % (row, col, self.nRows, self.nCols))
        else:
            tmpx = self.xMin + (col + 0.5) * self.dx
            tmpy = self.yMax - (row + 0.5) * self.dx
            return tmpx, tmpy

    def is_nodata(self, row, col):
        """Check nodata

        Args:
            row: row number, range from 0 to (nRows - 1).
            col: col number, range from 0 to (nCols - 1).

        Returns:
            True if the cell equals nodata.
        """
        return True if self.get_value_by_row_col(row, col) is None else False


class RasterUtilClass(object):
    """Utility function to handle raster data.

    See Also:
        :class:`pygeoc.raster.raster.Raster`.
    """

    def __init__(self):
        """Empty."""
        pass

    @staticmethod
    def read_raster(raster_file):
        """Read raster by GDAL.

        Args:
            raster_file: raster file path.

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
        # print(srs.ExportToProj4())
        if nodata_value is None:
            nodata_value = DEFAULT_NODATA
        band = None
        ds = None
        return Raster(ysize, xsize, data, nodata_value, geotrans, srs, dttype)

    @staticmethod
    def get_mask_from_raster(rasterfile, outmaskfile, keep_nodata=False):
        """Generate mask data from a given raster data.

        Args:
            rasterfile: raster file path.
            outmaskfile: output mask file path.

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

        if not keep_nodata:
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

            # print(i_min, i_max, j_min, j_max)
            y_size_mask = i_max - i_min + 1
            x_size_mask = j_max - j_min + 1
            x_min_mask = x_min + j_min * dx
            y_max_mask = y_max - i_min * dx
        else:
            y_size_mask = ysize
            x_size_mask = xsize
            x_min_mask = x_min
            y_max_mask = y_max
            i_min = 0
            j_min = 0
        print('%dx%d -> %dx%d' % (xsize, ysize, x_size_mask, y_size_mask))

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
            srcfile: source raster file.
            v_dict: classifier dict.
            dstfile: destination file path.
            gdaltype (:obj:`pygeoc.raster.GDALDataType`): GDT_Float32 as default.
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

        for (k, v) in iteritems(v_dict):
            dst_data[src_data == k] = v
        RasterUtilClass.write_gtiff_file(dstfile, src_r.nRows, src_r.nCols, dst_data,
                                         src_r.geotrans, src_r.srs, no_data, gdaltype)

    @staticmethod
    def write_gtiff_file(f_name, n_rows, n_cols, data, geotransform, srs, nodata_value,
                         gdal_type=GDT_Float32):
        """Output Raster to GeoTiff format file.

        Args:
            f_name: output gtiff file name.
            n_rows: Row count.
            n_cols: Col count.
            data: 2D array data.
            geotransform: geographic transformation.
            srs: coordinate system.
            nodata_value: nodata value.
            gdal_type (:obj:`pygeoc.raster.GDALDataType`): output raster data type,
                                                                  GDT_Float32 as default.
        """
        UtilClass.mkdir(os.path.dirname(FileClass.get_file_fullpath(f_name)))
        driver = gdal_GetDriverByName(str('GTiff'))
        try:
            ds = driver.Create(f_name, n_cols, n_rows, 1, gdal_type)
        except Exception:
            print('Cannot create output file %s' % f_name)
            return
        ds.SetGeoTransform(geotransform)
        try:
            ds.SetProjection(srs.ExportToWkt())
        except AttributeError or Exception:
            ds.SetProjection(srs)
        ds.GetRasterBand(1).SetNoDataValue(nodata_value)
        # if data contains numpy.nan, then replaced by nodata_value
        if isinstance(data, numpy.ndarray) and data.dtype in [numpy.dtype('int'),
                                                              numpy.dtype('float')]:
            data = numpy.where(numpy.isnan(data), nodata_value, data)
        ds.GetRasterBand(1).WriteArray(data)
        ds = None

    @staticmethod
    def write_asc_file(filename, data, xsize, ysize, geotransform, nodata_value):
        """Output Raster to ASCII file.

        Args:
            filename: output ASCII filename.
            data: 2D array data.
            xsize: Col count.
            ysize: Row count.
            geotransform: geographic transformation.
            nodata_value: nodata_flow value.
        """
        UtilClass.mkdir(os.path.dirname(FileClass.get_file_fullpath(filename)))
        header = 'NCOLS %d\n' \
                 'NROWS %d\n' \
                 'XLLCENTER %f\n' \
                 'YLLCENTER %f\n' \
                 'CELLSIZE %f\n' \
                 'NODATA_VALUE %f' % (xsize, ysize, geotransform[0] + 0.5 * geotransform[1],
                                      geotransform[3] - (ysize - 0.5) * geotransform[1],
                                      geotransform[1], nodata_value)

        with open(filename, 'w', encoding='utf-8') as f:
            f.write(header)
            for i in range(0, ysize):
                for j in range(0, xsize):
                    f.write('%s\t' % repr(data[i][j]))
                f.write('\n')
        f.close()

    @staticmethod
    def raster_to_gtiff(tif, geotif, change_nodata=False, change_gdal_type=False):
        """Converting Raster format to GeoTIFF.

        Args:
            tif: source raster file path.
            geotif: output raster file path.
            change_nodata: change NoDataValue to -9999 or not.
            gdal_type (:obj:`pygeoc.raster.GDALDataType`): GDT_Float32 as default.
            change_gdal_type: If True, output the Float32 data type.
        """
        rst_file = RasterUtilClass.read_raster(tif)
        nodata = rst_file.noDataValue
        if change_nodata:
            if not MathClass.floatequal(rst_file.noDataValue, DEFAULT_NODATA):
                nodata = DEFAULT_NODATA
                nodata_array = numpy.ones((rst_file.nRows, rst_file.nCols)) * rst_file.noDataValue
                nodata_check = numpy.isclose(rst_file.data, nodata_array)
                rst_file.data[nodata_check] = DEFAULT_NODATA
                # rst_file.data[rst_file.data == rst_file.noDataValue] = DEFAULT_NODATA
        gdal_type = rst_file.dataType
        if change_gdal_type:
            gdal_type = GDT_Float32
        RasterUtilClass.write_gtiff_file(geotif, rst_file.nRows, rst_file.nCols, rst_file.data,
                                         rst_file.geotrans, rst_file.srs, nodata,
                                         gdal_type)

    @staticmethod
    def raster_to_asc(raster_f, asc_f):
        """Converting Raster format to ASCII raster.

        Args:
            raster_f: raster file.
            asc_f: output ASCII file.
        """
        raster_r = RasterUtilClass.read_raster(raster_f)
        RasterUtilClass.write_asc_file(asc_f, raster_r.data, raster_r.nCols, raster_r.nRows,
                                       raster_r.geotrans, raster_r.noDataValue)

    @staticmethod
    def raster_statistics(raster_file):
        """Get basic statistics of raster data.

        Args:
            raster_file: raster file path.

        Returns:
            min, max, mean, std.
        """
        ds = gdal_Open(raster_file)
        band = ds.GetRasterBand(1)
        minv, maxv, meanv, std = band.ComputeStatistics(False)
        return minv, maxv, meanv, std

    @staticmethod
    def split_raster(rs, split_shp, field_name, temp_dir):
        """Split raster by given shapefile and field name.

        Args:
            rs: origin raster file.
            split_shp: boundary (ESRI Shapefile) used to spilt raster.
            field_name: field name identify the spilt value.
            temp_dir: directory to store the spilt rasters.
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

    @staticmethod
    def mask_raster(in_raster, mask, out_raster):
        """
        Mask raster data.
        Args:
            in_raster: list or one raster
            mask: Mask raster data
            out_raster: list or one raster

        """
        if is_string(in_raster) and is_string(out_raster):
            in_raster = [str(in_raster)]
            out_raster = [str(out_raster)]
        if len(in_raster) != len(out_raster):
            raise RuntimeError('input raster and output raster must have the same size.')

        maskr = RasterUtilClass.read_raster(mask)
        rows = maskr.nRows
        cols = maskr.nCols
        maskdata = maskr.data
        temp = maskdata == maskr.noDataValue
        for inr, outr in zip(in_raster, out_raster):
            origin = RasterUtilClass.read_raster(inr)
            if origin.nRows == rows and origin.nCols == cols:
                masked = numpy.where(temp, origin.noDataValue, origin.data)
            else:
                masked = numpy.ones((rows, cols)) * origin.noDataValue
                # TODO, the following loop should be optimized by numpy or numba
                for i in range(rows):
                    for j in range(cols):
                        if maskdata[i][j] == maskr.noDataValue:
                            continue
                        # get the center point coordinate of current cell
                        tempx, tempy = maskr.get_central_coors(i, j)
                        tempv = origin.get_value_by_xy(tempx, tempy)
                        if tempv is None:
                            continue
                        masked[i][j] = tempv
            RasterUtilClass.write_gtiff_file(outr, maskr.nRows, maskr.nCols, masked,
                                             maskr.geotrans, maskr.srs,
                                             origin.noDataValue, origin.dataType)

    @staticmethod
    def raster_binarization(given_value, rasterfilename):
        """Make the raster into binarization.

        The opening and closing are based on binary image. Therefore we need to
        make the raster into binarization.

        Args:
            given_value: The given value's pixels will be value in 1,
            other pixels will be value in 0.
            rasterfilename: The initial rasterfilena,e.

        Returns:
            binary_raster: Raster after binarization.
        """
        origin_raster = RasterUtilClass.read_raster(rasterfilename)
        binary_raster = numpy.where(origin_raster.data == given_value, 1, 0)
        return binary_raster

    @staticmethod
    def raster_erosion(rasterfile):
        """Erode the raster image.

         Find the min pixel's value in 8-neighborhood. Then change the compute
         pixel's value into the min pixel's value.

        Args:
            rasterfile: input original raster image, type can be filename(string,
            like "test1.tif"), rasterfile(class Raster) or numpy.ndarray.

        Returns:
            erosion_raster: raster image after erosion, type is numpy.ndarray.
        """
        if is_string(rasterfile):
            origin_raster = RasterUtilClass.read_raster(str(rasterfile))
        elif isinstance(rasterfile, Raster):
            origin_raster = rasterfile.data
        elif isinstance(rasterfile, numpy.ndarray):
            origin_raster = rasterfile
        else:
            return "Your rasterfile has a wrong type. Type must be string or " \
                   "numpy.array or class Raster in pygeoc."
        max_value_raster = origin_raster.max()
        erosion_raster = numpy.zeros((origin_raster.shape[0], origin_raster.shape[1]))
        # In order to compute the raster edges, we need to expand the original
        # raster's rows and cols. We need to add the edges whose pixels' value is
        # the max pixel's value in raster.
        add_row = numpy.full((1, origin_raster.shape[1]), max_value_raster)
        temp_origin_raster = numpy.vstack((numpy.vstack((add_row, origin_raster)), add_row))
        add_col = numpy.full((origin_raster.shape[0] + 2, 1), max_value_raster)
        expand_origin_raster = numpy.hstack((numpy.hstack((add_col, temp_origin_raster)), add_col))
        # Erode the raster.
        for i in range(origin_raster.shape[0]):
            for j in range(origin_raster.shape[1]):
                min_pixel_value = max_value_raster
                # Find the min pixel value in the 8-neighborhood.
                for k in range(3):
                    for l in range(3):
                        if expand_origin_raster[i + k, j + l] <= min_pixel_value:
                            min_pixel_value = expand_origin_raster[i + k, j + l]
                            # After this loop, we get the min pixel's value of the
                            # 8-neighborhood. Then we change the compute pixel's value into
                            # the min pixel's value.
                    erosion_raster[i, j] = min_pixel_value
        # Return the result.
        return erosion_raster

    @staticmethod
    def raster_dilation(rasterfile):
        """Dilate the raster image.

         Find the max pixel's value in 8-neighborhood. Then change the compute
         pixel's value into the max pixel's value.

        Args:
            rasterfile: input original raster image, type can be filename(string,
            like "test1.tif"), rasterfile(class Raster) or numpy.ndarray.

        Returns:
            dilation_raster: raster image after dilation, type is numpy.ndarray.
        """
        if is_string(rasterfile):
            origin_raster = RasterUtilClass.read_raster(str(rasterfile))
        elif isinstance(rasterfile, Raster):
            origin_raster = rasterfile.data
        elif isinstance(rasterfile, numpy.ndarray):
            origin_raster = rasterfile
        else:
            return 'Your rasterfile has a wrong type. Type must be string or ' \
                   'numpy.array or class Raster in pygeoc.'
        min_value_raster = origin_raster.min()
        dilation_raster = numpy.zeros((origin_raster.shape[0], origin_raster.shape[1]))
        # In order to compute the raster edges, we need to expand the original
        # raster's rows and cols. We need to add the edges whose pixels' value is
        # the max pixel's value in raster.
        add_row = numpy.full((1, origin_raster.shape[1]), min_value_raster)
        temp_origin_raster = numpy.vstack((numpy.vstack((add_row, origin_raster)), add_row))
        add_col = numpy.full((origin_raster.shape[0] + 2, 1), min_value_raster)
        expand_origin_raster = numpy.hstack((numpy.hstack((add_col, temp_origin_raster)), add_col))
        # Dilate the raster.
        for i in range(origin_raster.shape[0]):
            for j in range(origin_raster.shape[1]):
                max_pixel_value = min_value_raster
                # Find the max pixel value in the 8-neighborhood.
                for k in range(3):
                    for l in range(3):
                        if expand_origin_raster[i + k, j + l] >= max_pixel_value:
                            max_pixel_value = expand_origin_raster[i + k, j + l]
                            # After this loop, we get the max pixel's value of the
                            # 8-neighborhood. Then we change the compute pixel's value into
                            # the max pixel's value.
                    dilation_raster[i, j] = max_pixel_value
        # Return the result.
        return dilation_raster

    @staticmethod
    def openning(input_rasterfilename, times):
        """Do openning.

        Openning: Erode firstly, then Dilate.

        Args:
            input_rasterfilename: input original raster image filename.
            times: Erode and Dilate times.

        Returns:
            openning_raster: raster image after open.
        """
        input_raster = RasterUtilClass.read_raster(input_rasterfilename)
        openning_raster = input_raster
        for i in range(times):
            openning_raster = RasterUtilClass.raster_erosion(openning_raster)
        for i in range(times):
            openning_raster = RasterUtilClass.raster_dilation(openning_raster)
        return openning_raster

    @staticmethod
    def closing(input_rasterfilename, times):
        """Do closing.

        Closing: Dilate firstly, then Erode.

        Args:
            input_rasterfilename: input original raster image filename.
            times: Erode and Dilate times.

        Returns:
            closing_raster: raster image after close.
        """
        input_raster = RasterUtilClass.read_raster(input_rasterfilename)
        closing_raster = input_raster
        for i in range(times):
            closing_raster = RasterUtilClass.raster_dilation(closing_raster)
        for i in range(times):
            closing_raster = RasterUtilClass.raster_erosion(closing_raster)
        return closing_raster


if __name__ == '__main__':
    # Run doctest in docstrings of Google code style
    # python -m doctest raster.py (only when doctest.ELLIPSIS is not specified)
    # or python raster.py -v
    # or py.test --doctest-modules raster.py
    import doctest

    doctest.testmod(optionflags=doctest.ELLIPSIS)
