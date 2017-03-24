#! /usr/bin/env python
# coding=utf-8
# @Utility functions
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#
import math
import sys

from pygeoc.utils.parseConfig import getconfigfile
from pygeoc.utils.utils import UtilClass

# Global variables
UTIL_ZERO = 1.e-6
MINI_SLOPE = 0.0001
DEFAULT_NODATA = -9999.
SQ2 = math.sqrt(2.)
LFs = ['\r\n', '\n\r', '\r', '\n']


def LoadConfiguration(inifile):
    command = '%s %s/config.py -ini %s' % (sys.executable, UtilClass.currentpath(), inifile)
    UtilClass.runcommand(command)


def status_output(status_msg, percent, file_object):
    """Print status and flush to file."""
    print ("[Output] %d..., %s" % (percent, status_msg))
    file_object.write("%d, %s\n" % (percent, status_msg))
    file_object.flush()


def ReadDataItemsFromTxt(txtFile):
    """
    Read data items include title from text file
    :param txtFile: data file
    :return: 2D data array
    """
    f = open(txtFile)
    dataItems = []
    for line in f:
        strLine = line
        for LF in LFs:
            if LF in line:
                strLine = line.split(LF)[0]
                break

        # strLine = line.split(LF)[0]
        if strLine != '' and strLine.find('#') < 0:
            lineList = strLine.split('\t')
            dataItems.append(lineList)
    f.close()
    return dataItems


#  TEST CODE
if __name__ == "__main__":
    LoadConfiguration(getconfigfile())
    # p = r'E:\data\model_data\model_dianbu_10m_longterm\data_prepare\spatial'
    # print GetFileNameWithSuffixes(p,['.tif','.txt'])
    # print GetFullPathWithSuffixes(p,['.tif','.txt'])
    # dist2Stream = r'E:\data_m\SEIMS\dianbu_30m_output\dist2Stream.tif'
    # R = ReadRaster(dist2Stream)
    # print "XMin: %f" % R.xMin
    # print "XMax: %f" % R.xMax
    # print "YMin: %f" % R.yMin
    # print "YMax: %f" % R.yMax
    # print "Mean: %f" % R.GetAverage()
    # print "Max : %f" % R.GetMax()
    # print "Min : %f" % R.GetMin()
    # print "Sum : %f" % R.GetSum()
    # print "STD : %f" % R.GetSTD()
    # print "Value at (x, y): %f" % R.GetValueByXY(39542419.65, 3543174.289)
    datafile = r'G:\code_zhulj\SEIMS\model_data\dianbu\data_prepare\climate\Sites_P.txt'
    data = ReadDataItemsFromTxt(datafile)
    print (data)
