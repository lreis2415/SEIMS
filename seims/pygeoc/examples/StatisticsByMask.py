from pygeoc.raster import *
from pygeoc.utils.utils import FloatEqual

def StatisticsByMask(simf, obsf, maskf):
    maskr = ReadRaster(maskf)
    rows = maskr.nRows
    cols = maskr.nCols
    # print "MASK size: rows %d, cols %d" % (rows, cols)
    nodata = maskr.noDataValue
    maskdata = maskr.data

    simr = ReadRaster(simf)
    obsr = ReadRaster(obsf)
    simNodata = simr.noDataValue
    obsNodata = obsr.noDataValue
    n = 0
    E = 0.
    E2 = 0.
    for row in range(rows):
        for col in range(cols):
            if not maskdata[row][col] == nodata:
                tmpx, tmpy = maskr.GetCentralCoors(row + 1, col + 1)
                simV = simr.GetValueByXY(tmpx, tmpy)
                obsV = obsr.GetValueByXY(tmpx, tmpy)
                if simV is not None and obsV is not None and \
                        not simV == simNodata and not obsV == obsNodata:
                    n += 1
                    E += (simV - obsV)
                    E2 += (simV - obsV) * (simV - obsV)
    print n, E, E2
    ME = E / n
    RMSE = math.sqrt(E2 / n)
    return ME, RMSE
if __name__ == "__main__":
    MaskRasterFile = r'C:\Users\ZhuLJ\Desktop\ME_RMSE_testdata\region.tif'
    SimulateRasterFile = r'C:\Users\ZhuLJ\Desktop\ME_RMSE_testdata\result.tif'
    TheoryRasterFile = r'C:\Users\ZhuLJ\Desktop\ME_RMSE_testdata\theoSCA_1m.asc'
    ME, RMSE = StatisticsByMask(SimulateRasterFile, TheoryRasterFile, MaskRasterFile)
    print "ME: %f, RMSE: %f" % (ME, RMSE)
    f = open(r'C:\Users\ZhuLJ\Desktop\test.txt', 'w+')
    f.write("ME: %f, RMSE: %f" % (ME, RMSE))
    f.close()
