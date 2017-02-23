#! /usr/bin/env python
# coding=utf-8

# Function List:
# 1. currentPath() return the path of your script
# 2. mkdir(dir) Make directory if not exists.
# 3. rmmkdir(dir) If dir is not exists, then make it, else remove and remake it.
# 4. FloatEqual(a, b) If a = b?

import os
import sys
import time
import glob
import subprocess
from shutil import copy, rmtree
import socket

from const import *


class MathClass(object):
    def __init__(self):
        pass

    @staticmethod
    def isnumerical(x):
        try:
            xx = float(x)
        except TypeError:
            return False
        except ValueError:
            return False
        except 'Exception':
            return False
        else:
            return True

    @staticmethod
    def floatequal(a, b):
        return abs(a - b) < DELTA

    @staticmethod
    def nashcoef(obsvalues, simvalues):
        """
        Calculate Nash coefficient
        :param obsvalues:
        :param simvalues:
        :return: NSE, numeric value
        """
        if len(obsvalues) != len(simvalues):
            raise ValueError("The size of observed and simulated values must be the same for NSE calculation!")
        n = len(obsvalues)
        ave = sum(obsvalues) / n
        a1 = 0.
        a2 = 0.
        for i in range(n):
            a1 += pow(float(obsvalues[i]) - float(simvalues[i]), 2.)
            a2 += pow(float(obsvalues[i]) - ave, 2.)
        if a2 == 0.:
            return 1.
        return 1. - a1 / a2

    @staticmethod
    def rsquare(obsvalues, simvalues):
        """
        Calculate R-square
        :param obsvalues:
        :param simvalues:
        :return: R-square numeric value
        """
        if len(obsvalues) != len(simvalues):
            raise ValueError("The size of observed and simulated values must be the same for R-square calculation!")
        n = len(obsvalues)
        obsAvg = sum(obsvalues) / n
        predAvg = sum(simvalues) / n
        obsMinusAvgSq = 0.
        predMinusAvgSq = 0.
        obsPredMinusAvgs = 0.
        for i in range(n):
            obsMinusAvgSq += pow((obsvalues[i] - obsAvg), 2.)
            predMinusAvgSq += pow((simvalues[i] - predAvg), 2.)
            obsPredMinusAvgs += (obsvalues[i] - obsAvg) * (simvalues[i] - predAvg)
        # Calculate R-square
        yy = (pow(obsMinusAvgSq, 0.5) * pow(predMinusAvgSq, 0.5))
        if yy == 0.:
            return 1.
        return pow((obsPredMinusAvgs / yy), 2.)

    @staticmethod
    def rmse(list1, list2):
        n = len(list1)
        s = 0.
        for i in range(n):
            s += pow(list1[i] - list2[i], 2.)
        return math.sqrt(s / n)


class StringClass(object):
    def __init__(self):
        pass

    @staticmethod
    def stringmatch(str1, str2):
        if str1.lower() == str2.lower():
            return True
        else:
            return False

    @staticmethod
    def stripstring(str):
        """Remove space(' ') and indent('\t') at the begin and end of the string."""
        # @Function:
        oldStr = ''
        newStr = str
        while oldStr != newStr:
            oldStr = newStr
            newStr = oldStr.strip('\t')
            newStr = newStr.strip(' ')
        return newStr

    @staticmethod
    def splitstring(str, spliters = None):
        """
        Split string by split character space(' ') and indent('\t') as default
        :param str:
        :param spliters: e.g. [' ', '\t'], []
        :return:
        """
        if spliters is None or not spliters:
            spliters = [' ', '\t']
        destStrs = []
        srcStrs = [str]
        while True:
            oldDestStrs = srcStrs[:]
            for s in spliters:
                for srcS in srcStrs:
                    tempStrs = srcS.split(s)
                    for tempS in tempStrs:
                        tempS = StringClass.stripstring(tempS)
                        if tempS != '':
                            destStrs.append(tempS)
                srcStrs = destStrs[:]
                destStrs = []
            if oldDestStrs == srcStrs:
                destStrs = srcStrs[:]
                break
        return destStrs

    @staticmethod
    def issubstring(substr, Str):
        """Is substr part of str, case insensitive."""
        if substr.lower() in Str.lower():
            return True
        else:
            return False

    @staticmethod
    def stringinlist(str, strlist):
        """Is str in strlist, case insensitive."""
        new_str_list = strlist[:]
        for i in range(len(new_str_list)):
            new_str_list[i] = new_str_list[i].lower()
        if str.lower() in new_str_list:
            return True
        else:
            return False

    @staticmethod
    def isvalidipaddr(address):
        try:
            socket.inet_aton(address)
            return True
        except "Exception":
            return False


class FileClass(object):
    def __init__(self):
        pass

    @staticmethod
    def isfileexists(filename):
        if filename is None or not os.path.exists(filename):
            return False
        else:
            return True

    @staticmethod
    def checkfileexists(filename):
        if not FileClass.isfileexists(filename):
            UtilClass.error("Input files path %s is None or not existed!\n" % filename)

    @staticmethod
    def copyfiles(filename, dstfilename):
        FileClass.removefiles(dstfilename)
        dstPrefix = os.path.splitext(dstfilename)[0]
        pattern = os.path.splitext(filename)[0] + '.*'
        for f in glob.iglob(pattern):
            ext = os.path.splitext(f)[1]
            dst = dstPrefix + ext
            copy(f, dst)

    @staticmethod
    def removefiles(filename):
        """
        Delete all files with same root as fileName,
        i.e. regardless of suffix, such as shapefile of ESRI
        """
        pattern = os.path.splitext(filename)[0] + '.*'
        for f in glob.iglob(pattern):
            os.remove(f)

    @staticmethod
    def isuptodate(outfile, basedatetime):
        """Return true if outfile exists and is no older than base datetime."""
        if os.path.exists(outfile):
            if os.path.getmtime(outfile) >= basedatetime:
                return True
        return False

    @staticmethod
    def getexecutablefullpath(name):
        """
        Not for Windows!!!
        get the full path of a given executable name
        :return:
        """
        findout = UtilClass.runcommand('which %s' % name)
        if findout == [] or len(findout) == 0:
            print ("%s is not included in the env path" % name)
            exit(-1)
        return findout[0].split('\n')[0]

    @staticmethod
    def getfilenamebysuffixes(dir, suffixes):
        list_files = os.listdir(dir)
        re_files = []
        for f in list_files:
            name, ext = os.path.splitext(f)
            if StringClass.stringinlist(ext, suffixes):
                re_files.append(f)
        return re_files

    @staticmethod
    def getfullfilenamebysuffixes(dir, suffixes):
        full_paths = []
        for name in FileClass.getfilenamebysuffixes(dir, suffixes):
            full_paths.append(dir + os.sep + name)
        return full_paths


class DateClass(object):
    """Utility function to handle datetime."""

    def __init__(self):
        pass

    @staticmethod
    def isleapyear(year):
        if (year % 4 == 0 and year % 100 != 0) or (year % 400 == 0):
            return True
        else:
            return False

    @staticmethod
    def dayofmonth(year, month):
        if month in [1, 3, 5, 7, 8, 10, 12]:
            return 31
        elif month in [4, 6, 9, 11]:
            return 30
        elif DateClass.isleapyear(year):
            return 29
        else:
            return 28

    @staticmethod
    def dayofyear(dt):
        sec = time.mktime(dt.timetuple())
        t = time.localtime(sec)
        return t.tm_yday


class UtilClass(object):
    """Other common used utility functions"""

    def __init__(self):
        pass

    @staticmethod
    def runcommand(commands):
        """
        Execute external command, and return the output lines list
        :param commands: string or list
        :return: output lines
        """
        use_shell = True
        if isinstance(commands, list) or isinstance(commands, tuple):
            use_shell = False
        process = subprocess.Popen(commands, shell = use_shell, stdout = subprocess.PIPE, stdin = open(os.devnull),
                                   stderr = subprocess.STDOUT, universal_newlines = True)
        return process.stdout.readlines()

    @staticmethod
    def currentpath():
        path = sys.path[0]
        if os.path.isdir(path):
            return path
        elif os.path.isfile(path):
            return os.path.dirname(path)

    @staticmethod
    def mkdir(dir):
        if not os.path.isdir(dir) or not os.path.exists(dir):
            os.mkdir(dir)

    @staticmethod
    def rmmkdir(dir):
        if not os.path.isdir(dir):
            os.mkdir(dir)
        else:
            rmtree(dir, True)
            os.mkdir(dir)

    @staticmethod
    def printmsg(contentlist):
        if isinstance(contentlist, list) or isinstance(contentlist, tuple):
            contentstr = ''
            for content in contentlist:
                contentstr += "%s\n" % content
            return contentstr
        else:
            return contentlist

    @staticmethod
    def error(msg):
        raise RuntimeError(msg)

    @staticmethod
    def writelog(logfile, contentlist, MODE = 'replace'):
        if os.path.exists(logfile):
            if MODE == 'replace':
                os.remove(logfile)
                logStatus = open(logfile, 'w')
            else:
                logStatus = open(logfile, 'a')
        else:
            logStatus = open(logfile, 'w')
        logStatus.write(UtilClass.printmsg(contentlist))
        logStatus.flush()
        logStatus.close()
