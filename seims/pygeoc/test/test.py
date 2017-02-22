from pygeoc.hydro import *
from pygeoc.raster import *
from pygeoc.utils import *
from pygeoc.vector import *

if __name__ == '__main__':
    print D8DIR_TD_VALUES
    dem = r'd:/test/dem_30m.tif'
    demR = ReadRaster(dem)
    print RasterStatistics(dem)
