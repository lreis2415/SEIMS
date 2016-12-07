#! /usr/bin/env python
# coding=utf-8
# @TauDEM Functions
#
#  includes Fill, FlowDirD8, FlowDirDinf, FlowAccD8, StreamRaster, MoveOutlet,
#           StreamSkeleton(peukerdouglas), StreamNet, DropAnalysis, D8DistDownToStream
#
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#

import os
import platform
import subprocess


def MPIHeader(mpiexeDir, inputProc, hostfile=None):
    if mpiexeDir is not None:
        cmd = '"' + mpiexeDir + os.sep + 'mpiexec"'
    else:
        cmd = '"mpiexec"'
    if inputProc > 4 and hostfile is not None:
        cmd = cmd + ' -f ' + hostfile + ' -n '
    else:
        cmd += ' -n '
    return cmd

def abortRun(lines):
    for line in lines:
        if "ERROR" in line.upper() or 'BAD TERMINATION' in line.upper():
            return False
    return True

def Fill(np, workingDir, dem, filledDem, mpiexeDir=None, exeDir=None):
    os.chdir(workingDir)
    strCmd = MPIHeader(mpiexeDir, np)
    if exeDir is not None:
        exe = exeDir + os.sep + "pitremove"
    else:
        exe = "pitremove"
    strCmd = strCmd + " %d %s -z %s -fel %s" % (np, exe, dem, filledDem)
    print (strCmd)
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print (runMsg)
        raise RuntimeError("Error occurs when running external program, please check!")


def FlowDirD8(np, workingDir, filledDem, flowDir, slope, mpiexeDir=None, exeDir=None):
    os.chdir(workingDir)
    strCmd = MPIHeader(mpiexeDir, np)
    if exeDir is not None:
        exe = exeDir + os.sep + "d8flowdir"
    else:
        exe = "d8flowdir"
    strCmd = strCmd + \
        " %d %s -fel %s -p %s  -sd8 %s" % (np, exe, filledDem, flowDir, slope)
    print strCmd
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print runMsg
        raise RuntimeError("Error occurs when running external program, please check!")


def D8DistDownToStream(np, workingDir, p, fel, src, dist, distancemethod, thresh, mpiexeDir=None, exeDir=None,
                       hostfile=None):
    os.chdir(workingDir)
    cmd = MPIHeader(mpiexeDir, np)
    if distancemethod == 'Horizontal':
        distmeth = 'h'
    elif distancemethod == 'Vertical':
        distmeth = 'v'
    elif distancemethod == 'Pythagoras':
        distmeth = 'p'
    elif distancemethod == 'Surface':
        distmeth = 's'
    else:
        distmeth = 's'
    if exeDir is None:
        cmd = cmd + str(
            np) + ' d8distdowntostream -p ' + '"' + p + '"' + ' -fel ' + '"' + fel + '"' + ' -src ' + '"' + src + '"' + ' -dist ' + '"' + dist + '"' + ' -m ' + distmeth + ' -thresh ' + str(
            thresh)
    else:
        cmd = cmd + str(
            np) + ' ' + exeDir + os.sep + 'd8distdowntostream -p ' + '"' + p + '"' + ' -fel ' + '"' + fel + '"' + ' -src ' + '"' + src + '"' + ' -dist ' + '"' + dist + '"' + ' -m ' + distmeth + ' -thresh ' + str(
            thresh)
    print cmd
    process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print runMsg
        raise RuntimeError("Error occurs when running external program, please check!")


def FlowDirDinf(np, workingDir, filledDem, flowAngle, slope, mpiexeDir=None, exeDir=None):
    os.chdir(workingDir)
    strCmd = MPIHeader(mpiexeDir, np)
    if exeDir is not None:
        exe = exeDir + os.sep + "dinfflowdir"
    else:
        exe = "dinfflowdir"
    strCmd = strCmd + \
        " %d %s -fel %s -ang %s -slp %s" % (np,
                                            exe, filledDem, flowAngle, slope)
    print strCmd
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print runMsg
        raise RuntimeError("Error occurs when running external program, please check!")


def FlowAccD8(np, workingDir, flowDir, acc, outlet=None, streamSkeleton=None, mpiexeDir=None, exeDir=None):
    os.chdir(workingDir)
    strCmd = MPIHeader(mpiexeDir, np)
    if exeDir is not None:
        exe = exeDir + os.sep + "aread8"
    else:
        exe = "aread8"
    if outlet is not None:
        if streamSkeleton is not None:
            strCmd = strCmd + " %d %s -p %s -o %s -wg %s -ad8 %s -nc" % (
                np, exe, flowDir, outlet, streamSkeleton, acc)
        else:
            strCmd = strCmd + \
                " %d %s -p %s -o %s -ad8 %s -nc" % (np,
                                                    exe, flowDir, outlet, acc)
    else:
        if streamSkeleton is not None:
            strCmd = strCmd + \
                " %d %s -p %s -wg %s -ad8 %s -nc" % (
                    np, exe, flowDir, streamSkeleton, acc)
        else:
            strCmd = strCmd + \
                " %d %s -p %s -ad8 %s -nc" % (np, exe, flowDir, acc)
    # -nc means donot consider edge contaimination
    print strCmd
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print runMsg
        raise RuntimeError("Error occurs when running external program, please check!")


def StreamRaster(np, workingDir, acc, streamRaster, threshold=1000, mpiexeDir=None, exeDir=None):
    os.chdir(workingDir)
    strCmd = MPIHeader(mpiexeDir, np)
    if exeDir is not None:
        exe = exeDir + os.sep + "threshold"
    else:
        exe = "threshold"
    strCmd = strCmd + \
        " %d %s -ssa %s -thresh %s  -src %s" % (np,
                                                exe, acc, str(threshold), streamRaster)
    print strCmd
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print runMsg
        raise RuntimeError("Error occurs when running external program, please check!")


def MoveOutlet(np, workingDir, flowDir, streamRaster, outlet, modifiedOutlet, mpiexeDir=None, exeDir=None):
    os.chdir(workingDir)
    strCmd = MPIHeader(mpiexeDir, np)
    if exeDir is not None:
        exe = exeDir + os.sep + "moveoutletstostreams"
    else:
        exe = "moveoutletstostreams"
    strCmd = strCmd + " %d %s -p %s -src %s -o %s -om %s" % (
        np, exe, flowDir, streamRaster, outlet, modifiedOutlet)
    print strCmd
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print runMsg
        raise RuntimeError("Error occurs when running external program, please check!")


def StreamSkeleton(np, workingDir, filledDem, streamSkeleton, mpiexeDir=None, exeDir=None):
    os.chdir(workingDir)
    strCmd = MPIHeader(mpiexeDir, np)
    if exeDir is not None:
        exe = exeDir + os.sep + "peukerdouglas"
    else:
        exe = "peukerdouglas"
    strCmd = strCmd + \
        " %d %s -fel %s -ss %s" % (np, exe, filledDem, streamSkeleton)
    print strCmd
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print runMsg
        raise RuntimeError("Error occurs when running external program, please check!")


def StreamNet(np, workingDir, filledDem, flowDir, acc, streamRaster, modifiedOutlet, streamOrder, chNetwork, chCoord,
              streamNet, subbasin, mpiexeDir=None, exeDir=None):
    os.chdir(workingDir)
    strCmd = MPIHeader(mpiexeDir, np)
    if exeDir is not None:
        exe = exeDir + os.sep + "streamnet"
    else:
        exe = "streamnet"
    strCmd = strCmd + " %d %s -fel %s -p %s -ad8 %s -src %s -o %s  -ord %s -tree %s -coord %s -net %s -w %s" % (
        np, exe, filledDem, flowDir, acc, streamRaster, modifiedOutlet, streamOrder, chNetwork, chCoord, streamNet,
        subbasin)
    print strCmd
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print runMsg
        raise RuntimeError("Error occurs when running external program, please check!")


def DropAnalysis(np, workingDir, fel, p, ad8, ssa, shapefile, minthresh, maxthresh, numthresh, logspace, drp,
                 mpiexeDir=None, exeDir=None):
    os.chdir(workingDir)
    strCmd = MPIHeader(mpiexeDir, np)
    if exeDir is not None:
        exe = exeDir + os.sep + "dropanalysis"
    else:
        exe = "dropanalysis"
    strCmd = strCmd + " %d %s -fel %s -p %s -ad8 %s -ssa %s -o %s -drp %s -par %f %f %f" % (
        np, exe, fel, p, ad8, ssa, shapefile, drp, minthresh, maxthresh, numthresh)
    if logspace == 'false':
        strCmd = strCmd + ' 1'
    else:
        strCmd = strCmd + ' 0'
    print strCmd
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    runMsg = process.stdout.readlines()
    if abortRun(runMsg):
        return runMsg
    else:
        print runMsg
        raise RuntimeError("Error occurs when running external program, please check!")
