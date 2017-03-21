from osgeo import gdal, ogr
import os, sys


def ReadDataItemsFromTxt(txtFile):
    '''
    Read data items include title from text file
    :param txtFile: data file
    :return: 2D data array
    '''
    f = open(txtFile)
    dataItems = []
    for line in f:
        strLine = line.split('\n')[0]
        if strLine != '' and strLine.find('#') < 0:
            lineList = strLine.split('\t')
            dataItems.append(lineList)
    f.close()
    return dataItems


def WriteLineShp(lineList, outShp, lineFieldsList = None):
    print "Write line shapefile: %s" % outShp
    gdal.SetConfigOption("GDAL_FILENAME_IS_UTF8", "NO")  ## support for path in Chinese
    gdal.SetConfigOption("SHAPE_ENCODING", "")  ## suppoert for field in Chinese
    ogr.RegisterAll()
    driver = ogr.GetDriverByName("ESRI Shapefile")
    if driver is None:
        print "ESRI Shapefile driver not available."
        sys.exit(1)
    if os.path.exists(outShp):
        driver.DeleteDataSource(outShp)

    fieldName = []
    fieldNameIdx = 0
    if lineFieldsList is not None:
        if len(lineList) != len(lineFieldsList):
            if len(lineList) + 1 == len(lineFieldsList):
                fieldName = lineFieldsList[0]
                fieldNameIdx = 1
            else:
                sys.exit(1)
        else:
            fieldLength = len(lineFieldsList[0])
            for i in range(fieldLength):
                name = 'lineName' + str(i)
                fieldName.append(name)
            fieldNameIdx = 0
    else:
        fieldName = ['LineName']
        fieldNameIdx = 0
    ds = driver.CreateDataSource(outShp.rpartition(os.sep)[0])
    if ds is None:
        print "ERROR Output: Creation of output file failed."
        sys.exit(1)
    lyr = ds.CreateLayer(outShp.rpartition(os.sep)[2].split('.')[0], None, ogr.wkbLineString)
    ## create fields
    for fld in fieldName:
        nameField = ogr.FieldDefn(fld, ogr.OFTString)
        lyr.CreateField(nameField)
    for l in lineList:
        idx = lineList.index(l)
        if len(l) > 1:
            line = ogr.Geometry(ogr.wkbLineString)
            for i in l:
                line.AddPoint(i[0], i[1])
            templine = ogr.CreateGeometryFromJson(line.ExportToJson())
            feature = ogr.Feature(lyr.GetLayerDefn())
            feature.SetGeometry(templine)
            for fld in fieldName:
                idx2 = fieldName.index(fld)
                if lineFieldsList is not None:
                    if fieldNameIdx == 1:
                        fieldValue = lineFieldsList[idx + 1][idx2]
                    else:
                        fieldValue = lineFieldsList[idx][idx2]
                else:
                    fieldValue = ' '
                # print fieldValue
                feature.SetField(fld, fieldValue)
            lyr.CreateFeature(feature)
            feature.Destroy()
    ds.Destroy()


def Points2Lines(txtFile, category, xName, yName):
    baseName = os.path.basename(txtFile).split('.')[0]
    lineShpPath = os.path.dirname(txtFile) + os.sep + baseName + '.shp'
    pointsData = ReadDataItemsFromTxt(txtFile)
    fieldArray = pointsData[0]
    dataArray = pointsData[1:]
    categoryIdx = fieldArray.index(category)
    xIdx = fieldArray.index(xName)
    yIdx = fieldArray.index(yName)
    linePointsList = []
    categoryList = []
    for item in dataArray:
        if (item[categoryIdx] not in categoryList):
            categoryList.append(item[categoryIdx])
            linePointsList.append([[float(item[xIdx]), float(item[yIdx])]])
        else:
            idx = categoryList.index(item[categoryIdx])
            linePointsList[idx].append([float(item[xIdx]), float(item[yIdx])])
    print linePointsList
    WriteLineShp(linePointsList, lineShpPath)


if __name__ == "__main__":
    pointsTxt = r'E:\test\points2lines\saddle_1.txt'
    Category = 'no'
    XName = 'globalx'
    YName = 'globaly'
    Points2Lines(pointsTxt, Category, XName, YName)
