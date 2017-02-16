#! /usr/bin/env python
# coding=utf-8

# Function List:
# 1. currentPath() return the path of your script
# 2. mkdir(dir) Make directory if not exists.
# 3. rmmkdir(dir) If dir is not exists, then make it, else remove and remake it.
# 4. FloatEqual(a, b) If a = b?

import os
import sys
from shutil import rmtree
from const import *


def currentPath():
    path = sys.path[0]
    if os.path.isdir(path):
        return path
    elif os.path.isfile(path):
        return os.path.dirname(path)


def mkdir(dir):
    if not os.path.isdir(dir):
        os.mkdir(dir)


def rmmkdir(dir):
    if not os.path.isdir(dir):
        os.mkdir(dir)
    else:
        rmtree(dir, True)
        os.mkdir(dir)


def FloatEqual(a, b):
    return abs(a - b) < DELTA


def WriteLog(logfile,  contentlist):
    if os.path.exists(logfile):
        logStatus = open(logfile, 'a')
    else:
        logStatus = open(logfile, 'w')
    for content in contentlist:
        logStatus.write("%s\n" % content)
    logStatus.flush()
    logStatus.close()
