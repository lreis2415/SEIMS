#! /usr/bin/env python
# coding=utf-8
# @TauDEM Utility Class
#
#  includes Fill, FlowDirD8, FlowDirDinf, FlowAccD8, StreamRaster, MoveOutlet,
#           StreamSkeleton(peukerdouglas), StreamNet, DropAnalysis, D8DistDownToStream
#
# @Author: Liang-Jun Zhu
# Thanks to the open-source software QSWAT by Chris George.
#

import os
from ..utils.utils import UtilClass, MathClass, FileClass


class TauDEM(object):
    """Methods for calling TauDEM executables."""

    def __init__(self):
        pass

    @staticmethod
    def error(msg, log_file = None):
        UtilClass.printmsg(msg)
        if log_file is not None:
            UtilClass.writelog(log_file, msg, 'append')
        raise RuntimeError(msg)

    @staticmethod
    def log(lines, log_file = None):
        err = False
        for line in lines:
            if log_file is not None:
                UtilClass.writelog(log_file, line, 'append')
            if 'BAD TERMINATION' in line.upper():
                err = True
        if err:
            TauDEM.error("Error occurred when calling TauDEM function, please check!\n", log_file)

    @staticmethod
    def run(function_name, in_files, in_params = None, out_files = None, mpi_params = None, log_params = None):
        """
        Run TauDEM function.
        The command will not execute if:
            1) any input file does not exist;
            2) all output files exist and were last modified no earlier than any input file.
        An error will be detected after running the TauDEM command if:
            1) any output file does not exist ;
            2) any output file was last modified earilier than any input file.

        :param function_name: TauDEM function name (with or without path) as string
        :param in_files: Required. Dict of pairs of parameter id (string) and file path (string) for input files.
        :param in_params: Optional. Dict of pairs of parameter id (string) and value (or None for a flag parameter
                          without a value) for input parameters.
        :param out_files: Optional. Dict of pairs of parameter id (string) and file path (string) for output files.
        :param mpi_params: Optional. Dict of pairs of parameter id (string) and value or path for MPI setting.
                           e.g. {'mpipath':'/soft/bin','hostfile':'/soft/bin/cluster.node','n':4} or
                                {'mpipath':'/soft/bin', 'n':4} or {'n':4}
        :param log_params: Optional. Dict of pairs of parameter id (string) and value or path for runtime and log
                           output parameters. e.g. {'logfile': '/home/user/log.txt'}
        :return True if TauDEM run successfully, otherwise False.
        """
        # check the log parameter
        log_file = None
        if log_params is not None:
            if type(log_params) != dict:
                TauDEM.error("The log parameter must be a dict!\n")
            if 'logfile' in log_params and log_params['logfile'] is not None:
                log_file = log_params['logfile']

        # check input files
        if in_files is None:
            TauDEM.error("Input files parameter is required!\n", log_file)
        if type(in_files) != dict:
            TauDEM.error("The input files parameter must be a dict!\n")
        for (pid, path) in in_files.items():
            if path is not None and not os.path.exists(path):
                TauDEM.error("Input files parameter %s: %s is not existed!\n" % (pid, path), log_file)

        # check output files to figure out if current run is necessary.
        # need_run = True
        # basedatetime = 0.
        # for (pid, path) in in_files.items():
        #     if path is not None and os.path.getmtime(path) > basedatetime:
        #         basedatetime = os.path.getmtime(path)
        # if out_files is not None:
        #     for (pid, out_file) in out_files.items():
        #         if out_file is not None and FileClass.isuptodate(out_file, basedatetime):
        #             need_run = False
        #             break
        # if not need_run:
        #     return True

        # remove out_files to avoid any file IO related error
        if out_files is not None:
            if type(out_files) != dict:
                TauDEM.error("The output files parameter must be a dict!\n")
            for (pid, out_file) in out_files.items():
                if out_file is not None:
                    FileClass.removefiles(out_file)

        # concatenate command line
        commands = []
        # MPI header
        if mpi_params is not None:
            if type(mpi_params) != dict:
                TauDEM.error("The MPI settings parameter must be a dict!\n")
            if 'mpipath' in mpi_params and mpi_params['mpipath'] is not None:
                commands.append(mpi_params['mpipath'] + os.sep + 'mpiexec')
            else:
                commands.append('mpiexec')
            if 'hostfile' in mpi_params and mpi_params['hostfile'] is not None:
                commands.append('-f')
                commands.append(mpi_params['hostfile'])
            if 'n' in mpi_params and mpi_params['n'] > 1:
                commands.append('-n')
                commands.append(str(mpi_params['n']))
            else:  # If number of processor is less equal than 1, then do not call mpiexec.
                commands = []
        # append TauDEM function name, which can be full path or just one name
        commands.append(function_name)
        # append input files
        for (pid, infile) in in_files.items():
            if infile is None:
                continue
            if pid[0] != '-':
                pid = '-' + pid
            commands.append(pid)
            commands.append(infile)
        # append input parameters
        if in_params is not None:
            if type(in_params) != dict:
                TauDEM.error("The input parameters must be a dict!\n")
            for (pid, v) in in_params.items():
                if pid[0] != '-':
                    pid = '-' + pid
                commands.append(pid)
                # allow for parameter which is an flag without value
                if v != '' and v is not None:
                    if MathClass.isnumerical(v):
                        commands.append(str(v))
                    else:
                        commands.append(v)
        # append output parameters
        if out_files is not None:
            for (pid, outfile) in out_files.items():
                if outfile is None:
                    continue
                if pid[0] != '-':
                    pid = '-' + pid
                commands.append(pid)
                commands.append(outfile)
        # run command
        # print (commands)
        runmsg = UtilClass.runcommand(commands)
        TauDEM.log(runmsg, log_file)
        return True

    @staticmethod
    def fullpath(name, dirname = None):
        if name is None:
            return None
        if os.sep in name:  # name is full path already
            return name
        if dirname is not None:
            return dirname + os.sep + name
        else:
            return name

    @staticmethod
    def Fill(np, workingdir, dem, filleddem, mpiexedir = None, exedir = None, log_file = None, hostfile = None):
        """Run pit remove using the flooding approach """
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('pitremove', exedir), {'-z': dem}, None, {'-fel': filleddem},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def FlowDirD8(np, workingdir, filleddem, flowdir, slope, mpiexedir = None, exedir = None, log_file = None,
                  hostfile = None):
        """Run D8 flow direction"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('d8flowdir', exedir), {'-fel': filleddem}, None,
                          {'-p': flowdir, '-sd8': slope},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def FlowDirDinf(np, workingdir, filleddem, flowangle, slope, mpiexedir = None, exedir = None, log_file = None,
                    hostfile = None):
        """Run Dinf flow direction"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('dinfflowdir', exedir), {'-fel': filleddem}, None,
                          {'-ang': flowangle, '-slp': slope},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def FlowAccD8(np, workingdir, flowdir, acc, outlet = None, streamskeleton = None, mpiexedir = None, exedir = None,
                  edgecontaimination = False, log_file = None, hostfile = None):
        """Run Accumulate area according to D8 flow direction"""
        # -nc means do not consider edge contaimination
        if not edgecontaimination:
            in_params = {'-nc': None}
        else:
            in_params = None
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('aread8', exedir), {'-p': flowdir, '-o': outlet, '-wg': streamskeleton},
                          in_params, {'-ad8': acc}, {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def FlowAccDinf(np, workingdir, angfile, sca, outlet = None, mpiexedir = None, exedir = None,
                    edgecontaimination = False, log_file = None, hostfile = None):
        """Run Accumulate area according to Dinf flow direction"""
        # -nc means do not consider edge contaimination
        if edgecontaimination:
            in_params = {'-nc': None}
        else:
            in_params = None
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('areadinf', exedir), {'-ang': angfile},
                          in_params, {'-sca': sca, '-o': outlet},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def GridNet(np, workingdir, pfile, plenfile, tlenfile, gordfile, outlet = None, mpiexedir = None, exedir = None,
                log_file = None, hostfile = None):
        """Run GridNet"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('gridnet', exedir), {'-p': pfile, '-o': outlet},
                          None, {'-plen': plenfile, '-tlen': tlenfile, '-gord': gordfile},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def StreamRaster(np, workingdir, acc, streamRaster, threshold = 1000, mpiexedir = None, exedir = None,
                     log_file = None, hostfile = None):
        """Run threshold for stream raster"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('threshold', exedir),
                          {'-ssa': acc}, {'-thresh': threshold}, {'-src': streamRaster},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def StreamNet(np, workingdir, filleddem, flowdir, acc, streamRaster, modifiedOutlet, streamOrder, chNetwork,
                  chCoord, streamNet, subbasin, mpiexedir = None, exedir = None, log_file = None, hostfile = None):
        """Run streamnet"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('streamnet', exedir),
                          {'-fel': filleddem, '-p': flowdir, '-ad8': acc, '-src': streamRaster, '-o': modifiedOutlet},
                          None, {'-ord': streamOrder, '-tree': chNetwork, '-coord': chCoord, '-net': streamNet,
                                 '-w'  : subbasin}, {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def MoveOutlet(np, workingdir, flowdir, streamRaster, outlet, modifiedOutlet, mpiexedir = None, exedir = None,
                   log_file = None, hostfile = None):
        """Run move the given outlets to stream"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('moveoutletstostrm', exedir),
                          {'-p': flowdir, '-src': streamRaster, '-o': outlet}, None,
                          {'-om': modifiedOutlet}, {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def D8DistDownToStream(np, workingdir, p, fel, src, dist, distancemethod, thresh, mpiexedir = None, exedir = None,
                           log_file = None, hostfile = None):
        """Run D8 distance to stream"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('d8distdowntostream', exedir),
                          {'-fel': fel, '-p': p, '-src': src}, {'-thresh': thresh, '-m': distancemethod},
                          {'-dist': dist}, {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def StreamSkeleton(np, workingdir, fel, streamSkeleton, mpiexedir = None, exedir = None, log_file = None,
                       hostfile = None):
        """Run peuker-douglas function"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('peukerdouglas', exedir), {'-fel': fel}, None,
                          {'-ss': streamSkeleton}, {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def DropAnalysis(np, workingdir, fel, p, ad8, ssa, outlet, minthresh, maxthresh, numthresh, logspace, drp,
                     mpiexedir = None, exedir = None, log_file = None, hostfile = None):
        os.chdir(workingdir)
        parstr = '%f %f %f' % (minthresh, maxthresh, numthresh)
        if logspace == 'false':
            parstr += ' 1'
        else:
            parstr += ' 0'
        return TauDEM.run(TauDEM.fullpath('dropanalysis', exedir),
                          {'-fel': fel, '-p': p, '-ad8': ad8, '-ssa': ssa, '-o': outlet}, {'-par': parstr},
                          {'-drp': drp}, {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})


if __name__ == "__main__":
    workingspace = r"D:\test"
    execdir = r"C:\z_code\Hydro\SEIMS2017\seims\bin"
    log = "taudem.log"
    dem = "dem_30m.tif"
    feldem = "dem_fel.tif"
    flowd8 = "flowd8.tif"
    sloped8 = "slpd8.tif"
    TauDEM.Fill(1, workingspace, dem, feldem)
    TauDEM.FlowDirD8(4, workingspace, feldem, flowd8, sloped8)
