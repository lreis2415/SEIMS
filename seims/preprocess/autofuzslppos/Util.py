"""Utility Classes and Functions

    @author   : Liangjun Zhu

    @changelog:
    - 15-07-31  - lj - initial implementation
    - 17-07-21  - lj - reorganize and incorporate with pygeoc
"""
from __future__ import absolute_import, unicode_literals
import os
from io import open

import numpy
from pygeoc.raster import RasterUtilClass


def rpi_calculation(distdown, distup, rpi_outfile):
    """Calculate Relative Position Index (RPI)."""
    down = RasterUtilClass.read_raster(distdown)
    up = RasterUtilClass.read_raster(distup)
    temp = down.data < 0
    rpi_data = numpy.where(temp, down.noDataValue, down.data / (down.data + up.data))
    RasterUtilClass.write_gtiff_file(rpi_outfile, down.nRows, down.nCols, rpi_data, down.geotrans,
                                     down.srs, down.noDataValue, down.dataType)


def slope_rad_to_deg(tanslp, slp):
    """Convert slope from radius to slope."""
    origin = RasterUtilClass.read_raster(tanslp)
    temp = origin.data == origin.noDataValue
    slpdata = numpy.where(temp, origin.noDataValue, numpy.arctan(origin.data) * 180. / numpy.pi)
    RasterUtilClass.write_gtiff_file(slp, origin.nRows, origin.nCols, slpdata, origin.geotrans,
                                     origin.srs, origin.noDataValue, origin.dataType)


def write_log(logfile, contentlist):
    """Write string or string list to log file."""
    if os.path.exists(logfile):
        log_status = open(logfile, 'a', encoding='utf-8')
    else:
        log_status = open(logfile, 'w', encoding='utf-8')
    if isinstance(contentlist, list) or isinstance(contentlist, tuple):
        for content in contentlist:
            log_status.write('%s\n' % content)
    else:
        log_status.write('%s\n' % contentlist)
    log_status.flush()
    log_status.close()


def main():
    """TEST CODE"""
    inf = r'C:\z_data_m\SEIMS2017\fuzslppos_ywz10m\slope_position_units\SLOPPOSITION.tif'
    # inr = RasterUtilClass.read_raster(inf)
    # inr.data[inr.data > 0] = 1.
    # RasterUtilClass.write_gtiff_file(inf, inr.nRows, inr.nCols, inr.data,
    #                                  inr.geotrans, inr.srs, inr.noDataValue,
    #                                  inr.dataType)
    RasterUtilClass.raster_to_gtiff(inf, inf, True, True)


if __name__ == '__main__':
    main()
