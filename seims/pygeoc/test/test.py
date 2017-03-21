from pygeoc.raster.raster import RasterUtilClass


if __name__ == '__main__':
    dem = r'd:/test/dem_30m.tif'
    demR = RasterUtilClass.ReadRaster(dem)
    print RasterUtilClass.RasterStatistics(dem)
