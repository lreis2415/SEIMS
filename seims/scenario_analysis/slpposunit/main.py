# -*- coding: utf-8 -*-

# Author: Huiran GAO
# Date: Sep 10, 2016

import os
from fieldPartion import *
from flowDirRlat import *

## Define data path
dataDir = r'D:\MasterWorks\SA\data\youfang_data\Data_Youfang\field_partion\FieldPartion'
## input data
slpPosfile = r'SlpPos.tif'
subBasinfile = r'subBasin.tif'
maskfile = r'mask.tif'

## output data
slpPosfile_ywzh = r'SlpPos_ywzh.tif'
prePartion = r'prePartion.tif'
fieldPartion = r'Partion_Integ.tif'

## read raster info
slpPos = ReadRaster(dataDir + os.sep + slpPosfile_ywzh).data
subBasin = ReadRaster(dataDir + os.sep + subBasinfile).data
# field = ReadRaster(dataDir + os.sep + prePartion).data

nCols = ReadRaster(dataDir + os.sep + maskfile).nCols
nRows = ReadRaster(dataDir + os.sep + maskfile).nRows
geotrans = ReadRaster(dataDir + os.sep + maskfile).geotrans
srs = ReadRaster(dataDir + os.sep + maskfile).srs

noDtVal_slpP = ReadRaster(dataDir + os.sep + slpPosfile).noDataValue
noDtVal_subB = ReadRaster(dataDir + os.sep + subBasinfile).noDataValue
noDataValue = -9999

if __name__ == "__main__":
    ## Start
    print "Start culcalation......"
    ## Field partion
    ClipRaster(dataDir + os.sep + slpPosfile, dataDir + os.sep + maskfile, dataDir + os.sep + slpPosfile_ywzh)
    PrefieldParti(slpPos, subBasin)
    extractSlpPos()
    ## Generate flow relation files
    SummitforSub(dataDir, subBasin)
    SummitforSlp(dataDir, prePartion)
    SlpforVly(dataDir)
    print "Finished!"
