# Example 1: basic raster operations
# Related modules: pygeoc.raster
# Created by Liang-Jun Zhu
# Date: 2016.11
#
import os
try:
    from pygeoc.raster import *
except ImportError:
    print ("ERROR: PyGeoC is not successfully installed, please check and retry!")


if __name__ == "__main__":
    inputdem = ".%sdata%sJamaica_dem.tif" % (os.sep, os.sep)
    demr = ReadRaster(inputdem)
    # metadata information
    print ("rows: %d, cols: %d" % (demr.nRows, demr.nCols))
    print ("LLCornerX: %f, LLCornerY: %f" % (demr.xMin, demr.yMin))
    print ("cell size: %f" % demr.dx)
    print ("Coornate system (if stated): %s" % demr.srs)
    # basic statistics, nodata is excluded
    print ("mean: %f, max: %f, min: %f" % (demr.GetAverage(), demr.GetMax(), demr.GetMin()))
    print ("std: %f, sum: %f" % (demr.GetSTD(), demr.GetSum()))

