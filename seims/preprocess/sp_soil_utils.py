"""Extract spatial soil parameters

    @author   : Liangjun Zhu, Junzhi Liu, Huiran Gao, Fang Shen

    @changelog:
    - 13-01-10  jz - initial implementation
    - 16-07-22  lj - Names and units of soil physical parameter are referred to
                     readsol.f, soil_par.f, and soil_phys.f in SWAT.
                     Data validation checking is also conducted.
    - 16-12-07  lj - rewrite for version 2.0
    - 17-06-23  lj - reorganize as basic class
    - 18-02-08  lj - compatible with Python3.
    - 22-06-08  lj - use mask_rasterio to reclassify soil and landuse parameters
"""

from __future__ import absolute_import, unicode_literals

import math
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import numpy
from osgeo.gdal import GDT_Float32
from pygeoc.raster import RasterUtilClass

from utility import DEFAULT_NODATA, UTIL_ZERO, MINI_SLOPE


class SoilUtilClass(object):

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def get_soil_texture_usda(clay, silt, sand):
        """The soil texture code system is from WetSpa Extension and SWAT model which is
        based on the soil texture triangle developed by USDA.
        The unit is percentage, silt + sand + clay [+ Rock] = 100.
            The corresponding default soil parameters (e.g. Ks, porosity) are stored in
        `seims/database/SoilLookup.csv`.
        Args:
            clay: clay content percentage
            silt: silt content percentage
            sand: sand content percentage

        Returns:
            [Soil texture ID, Hydrological soil group, USLE K factor]
        """
        if clay >= 40 >= silt and sand <= 45:
            return [12, 1, 0.22]  # clay / nian tu
        elif clay >= 40 and silt >= 40:
            return [11, 1, 0.26]  # silt caly / fen nian tu
        elif clay >= 35 and sand >= 45:
            return [10, 1, 0.28]  # sandy clay / sha nian tu
        elif clay >= 25 and 20 <= sand <= 45:
            return [9, 2, 0.3]  # clay loam / nian rang tu
        elif clay >= 25 and sand <= 20:
            return [8, 2, 0.32]  # silt clay loam / fen zhi nian rang tu
        elif clay >= 20 and silt <= 30 and sand >= 45:
            return [7, 2, 0.2]  # sandy clay loam / sha zhi nian rang tu
        elif clay >= 10 and 30 <= silt <= 50 and sand <= 50:
            return [6, 3, 0.3]  # loam / rang tu
        elif 50 <= silt <= 80 or clay >= 15 and silt >= 80:
            return [4, 3, 0.38]  # silt loam / fen zhi rang tu
        elif silt >= 80 and clay <= 15:
            return [5, 3, 0.34]  # silt / fen tu
        elif clay <= 10 and sand <= 50 or 50 <= sand <= 80:
            return [3, 4, 0.13]  # sandy loam / sha zhi rang tu
        elif sand <= 90:
            return [2, 4, 0.04]  # loamy sand / rang zhi sha tu
        else:
            return [1, 4, 0.02]  # sand / sha tu

    @staticmethod
    def usle_k_epic(sand, silt, clay, om):
        """Calculate USLE_K factor according to EPIC (Erosion Productivity Impact Calculator).

        References:
            1. Sharply, A. N., & Williams, J. R. (1990).
               EPIC-erosion/productivity impact calculator I, Model documentation.
               U.S. Department of Agriculture Technical Bulletin, No. 1768, page 26, Eq. 2.96.
            2. Equation. 4:1.1.5 - 4:1.1.9 in SWAT Theory 2009.
               Note that one number is wrong, i.e., 0.0256 rather than 0.256 in SWAT theory doc.

        TODO: Add more improved algorithms such as Wang et al. (2016, ISWCR)
        """
        cbn = om * 0.58
        sn = 1. - sand * 0.01
        a = (0.2 + 0.3 * math.exp(-0.0256 * sand * (1. - silt * 0.01)))
        b = math.pow(silt / (clay + silt), 0.3)
        c = (1. - 0.25 * cbn / (cbn + math.exp(3.72 - 2.95 * cbn)))
        d = (1. - 0.7 * sn / (sn + math.exp(-5.51 + 22.9 * sn)))
        k = a * b * c * d
        return k

    @staticmethod
    def initial_soil_moisture(acc_file, slope_file, out_file):
        """Initialize soil moisture fraction of field capacity, based on TWI"""
        acc_r = RasterUtilClass.read_raster(acc_file)
        data_acc = acc_r.data
        xsize = acc_r.nCols
        ysize = acc_r.nRows
        nodata_value = acc_r.noDataValue
        srs = acc_r.srs
        geotrans = acc_r.geotrans
        dx = acc_r.dx
        data_slope = RasterUtilClass.read_raster(slope_file).data
        cell_area = dx * dx

        def wi_grid_cal(accvalue, slpvalue):
            """TWI, ln(acc_file/tan(slp))"""
            if abs(accvalue - nodata_value) < UTIL_ZERO:
                return DEFAULT_NODATA
            else:
                if abs(slpvalue) < MINI_SLOPE:
                    slpvalue = MINI_SLOPE
                return math.log((accvalue + 1.) * cell_area / slpvalue)

        wi_grid_cal_numpy = numpy.frompyfunc(wi_grid_cal, 2, 1)
        wi_grid = wi_grid_cal_numpy(data_acc, data_slope)
        # wiGrid_valid = numpy.where(acc_r.validZone, wi_grid, numpy.nan)
        # wi_max = numpy.nanmax(wiGrid_valid)
        # wi_min = numpy.nanmin(wiGrid_valid)
        # WARNING: numpy.nanmax and numpy.nanmin are un-stable in Linux, so
        # replaced by the for loops. By LJ
        wi_max = -numpy.inf
        wi_min = numpy.inf
        for i in range(0, ysize):
            for j in range(0, xsize):
                if wi_max < wi_grid[i][j]:
                    wi_max = wi_grid[i][j]
                if DEFAULT_NODATA != wi_grid[i][j] < wi_min:
                    wi_min = wi_grid[i][j]
        # print('TWIMax:%f, TWIMin:%f' % (wi_max, wi_min))
        soil_mois_fr_min = 0.6  # minimum relative saturation
        soil_mois_fr_max = 1.0

        wi_uplimit = wi_max
        a = (soil_mois_fr_max - soil_mois_fr_min) / (wi_uplimit - wi_min)
        b = soil_mois_fr_min - a * wi_min

        def moisture_cal(acc, wigrid):
            """calculate soil moisture"""
            if abs(acc - nodata_value) < UTIL_ZERO:
                return DEFAULT_NODATA
            else:
                tmp = a * wigrid + b
                if tmp > soil_mois_fr_max:
                    return soil_mois_fr_max
                elif tmp < soil_mois_fr_min:
                    return soil_mois_fr_min
                else:
                    return tmp

        moisture_cal_numpy = numpy.frompyfunc(moisture_cal, 2, 1)
        moisture = moisture_cal_numpy(data_acc, wi_grid)
        RasterUtilClass.write_gtiff_file(out_file, ysize, xsize, moisture, geotrans, srs,
                                         DEFAULT_NODATA, GDT_Float32)
