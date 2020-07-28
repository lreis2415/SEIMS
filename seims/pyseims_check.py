"""Check the requirements of pySEIMS.
"""
from __future__ import absolute_import, unicode_literals

# 1. pygeoc
try:
    import pygeoc
    from pygeoc.raster import *
    from pygeoc.vector import *
    from pygeoc.hydro import *
    from pygeoc.utils import *
except ImportError:
    print('ERROR: PyGeoC is not successfully installed, please check and retry!')
else:
    print('PyGeoC-%s has been installed!' % pygeoc.__version__)
# 2. gdal
try:
    import osgeo
    from osgeo import ogr
    from osgeo import osr
    from osgeo import gdalconst
    from osgeo import gdal_array
    from osgeo import gdal
except ImportError:
    print('ERROR: GDAL is not successfully installed, please check and retry!')
else:
    print('GDAL-%s has been installed!' % osgeo.__version__)
# 3. numpy
try:
    import numpy
except ImportError:
    print('ERROR: NumPy is not successfully installed, please check and retry!')
else:
    print('NumPy-%s has been installed!' % numpy.__version__)
# 4. pymongo
try:
    import pymongo
    from pymongo import MongoClient
except ImportError:
    print('ERROR: pymongo is not successfully installed, please check and retry!')
else:
    print('pymongo-%s has been installed!' % pymongo.__version__)
# 5. networkx
try:
    import networkx
except ImportError:
    print('ERROR: networkx is not successfully installed, please check and retry!')
else:
    print('networkx-%s has been installed!' % networkx.__version__)
# 6. shapely
try:
    import shapely
except ImportError:
    print('ERROR: shapely is not successfully installed, please check and retry!')
else:
    print('shapely-%s has been installed!' % shapely.__version__)
# 7. matplotlib
try:
    import matplotlib
except ImportError:
    print('ERROR: matplotlib is not successfully installed, please check and retry!')
else:
    print('matplotlib-%s has been installed!' % matplotlib.__version__)
# 8. deap
try:
    import deap
except ImportError:
    print('ERROR: deap is not successfully installed, please check and retry!')
else:
    print('deap-%s has been installed!' % deap.__version__)
# 9. scoop
try:
    import scoop
except ImportError:
    print('ERROR: scoop is not successfully installed, please check and retry!')
else:
    print('scoop-%s.%s has been installed!' % (scoop.__version__, scoop.__revision__))
