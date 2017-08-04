#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""TauDEM Utility Class

   includes fill, d8flowdir, dinfflowdir, aread8, threshold, moveoutletstostrm,
            peukerdouglas(peukerdouglas), streamnet, dropanalysis, d8distdowntostream
   Thanks to the open-source software QSWAT by Chris George.

    author: Liangjun Zhu

    changlog: 12-04-12 jz - origin version.\n
              16-07-01 lj - reorganized for pygeoc.\n
              17-06-25 lj - check by pylint and reformat by Google style.\n
"""

import os

from postTauDEM import DinfUtil
from ..raster.raster import RasterUtilClass
from ..utils.utils import UtilClass, MathClass, FileClass, StringClass


class TauDEMFilesUtils(object):
    """predefined TauDEM resulted file names"""
    # intermediate data
    _FILLEDDEM = 'demFilledTau.tif'
    _D8FLOWDIR = 'flowDirTauD8.tif'
    _SLOPE = 'slopeTau.tif'
    _D8ACC = 'accTauD8.tif'
    _D8ACCWITHWEIGHT = 'accTauD8WithWeight.tif'
    _STREAMRASTER = 'streamRasterTau.tif'
    _FLOWDIRDINF = 'flowDirDinfTau.tif'
    _DIRCODEDINF = 'dirCodeDinfTau.tif'
    _WEIGHTDINF = 'weightDinfTau.tif'
    _SLOPEDINF = 'slopeDinfTau.tif'
    _DEFAULTOUTLET = 'outlet_pre.shp'
    _MODIFIEDOUTLET = 'outletM.shp'
    _STREAMSKELETON = 'streamSkeleton.tif'
    _DROPTXT = 'drp.txt'
    _STREAMORDER = 'streamOrderTau.tif'
    _CHNETWORK = 'chNetwork.txt'
    _CHCOORD = 'chCoord.txt'
    _STREAMNET = 'streamNet.shp'
    _DIST2STREAMD8 = 'dist2StreamD8Org.tif'
    _SUBBASIN = 'subbasinTau.tif'
    # masked file names
    _SUBBASINM = 'subbasinTauM.tif'
    _D8FLOWDIRM = 'flowDirTauM.tif'
    _STREAMRASTERM = 'streamRasterTauM.tif'

    def __init__(self, tau_dir):
        """assign taudem resulted file path"""
        self.workspace = tau_dir
        self.filldem = tau_dir + os.sep + self._FILLEDDEM
        self.d8flow = tau_dir + os.sep + self._D8FLOWDIR
        self.slp = tau_dir + os.sep + self._SLOPE
        self.d8acc = tau_dir + os.sep + self._D8ACC
        self.d8acc_weight = tau_dir + os.sep + self._D8ACCWITHWEIGHT
        self.stream_raster = tau_dir + os.sep + self._STREAMRASTER
        self.dinf = tau_dir + os.sep + self._FLOWDIRDINF
        self.dinf_d8dir = tau_dir + os.sep + self._DIRCODEDINF
        self.dinf_weight = tau_dir + os.sep + self._WEIGHTDINF
        self.dinf_slp = tau_dir + os.sep + self._SLOPEDINF
        self.outlet_pre = tau_dir + os.sep + self._DEFAULTOUTLET
        self.outlet_m = tau_dir + os.sep + self._MODIFIEDOUTLET
        self.stream_pd = tau_dir + os.sep + self._STREAMSKELETON
        self.stream_order = tau_dir + os.sep + self._STREAMORDER
        self.channel_net = tau_dir + os.sep + self._CHNETWORK
        self.channel_coord = tau_dir + os.sep + self._CHCOORD
        self.streamnet_shp = tau_dir + os.sep + self._STREAMNET
        self.dist2stream_d8 = tau_dir + os.sep + self._DIST2STREAMD8
        self.subbsn = tau_dir + os.sep + self._SUBBASIN
        self.subbsn_m = tau_dir + os.sep + self._SUBBASINM
        self.d8flow_m = tau_dir + os.sep + self._D8FLOWDIRM
        self.stream_m = tau_dir + os.sep + self._STREAMRASTERM
        self.drptxt = tau_dir + os.sep + self._DROPTXT


class TauDEM(object):
    """Methods for calling TauDEM executables."""

    def __init__(self):
        """Empty function"""
        pass

    @staticmethod
    def error(msg, log_file=None):
        """Print, output error message and raise RuntimeError."""
        UtilClass.print_msg(msg)
        if log_file is not None:
            UtilClass.writelog(log_file, msg, 'append')
        raise RuntimeError(msg)

    @staticmethod
    def log(lines, log_file=None):
        """Output log message."""
        err = False
        for line in lines:
            if log_file is not None:
                UtilClass.writelog(log_file, line, 'append')
            if 'BAD TERMINATION' in line.upper():
                err = True
                break
        if err:
            TauDEM.error("Error occurred when calling TauDEM function, please check!\n", log_file)

    @staticmethod
    def run(function_name, in_files, in_params=None, out_files=None, mpi_params=None,
            log_params=None):
        """
        Run TauDEM function.
        The command will not execute if:
            1) any input file does not exist;
            2) all output files exist and were last modified no earlier than any input file.
        An error will be detected after running the TauDEM command if:
            1) any output file does not exist ;
            2) any output file was last modified earlier than any input file.

        Args:
            function_name: TauDEM function name (with or without path) as string
            in_files: Required. Dict of pairs of parameter id (string) and file path
                        (string or list) for input files.
            in_params: Optional. Dict of pairs of parameter id (string) and value (or None for a
                        flag parameter without a value) for input parameters.
            out_files: Optional. Dict of pairs of parameter id (string) and file path (string) for
                        output files.
            mpi_params: Optional. Dict of pairs of parameter id (string) and value or path for
                         MPI setting.
                        e.g. {'mpipath':'/soft/bin','hostfile':'/soft/bin/cluster.node','n':4} or
                             {'mpipath':'/soft/bin', 'n':4} or {'n':4}
            log_params: Optional. Dict of pairs of parameter id (string) and value or path for
                         runtime and log output parameters. e.g. {'logfile': '/home/user/log.txt'}

        Returns:
            True if TauDEM run successfully, otherwise False.
        """
        # check the log parameter
        log_file = None
        if log_params is not None:
            if not isinstance(log_params, dict):
                TauDEM.error('The log parameter must be a dict!\n')
            if 'logfile' in log_params and log_params['logfile'] is not None:
                log_file = log_params['logfile']

        # check input files
        if in_files is None:
            TauDEM.error('Input files parameter is required!\n', log_file)
        if not isinstance(in_files, dict):
            TauDEM.error('The input files parameter must be a dict!\n')
        for (pid, infile) in in_files.items():
            if infile is None:
                continue
            if isinstance(infile, list) or isinstance(infile, tuple):
                for inf in infile:
                    if inf is not None and not os.path.exists(inf):
                        TauDEM.error('Input files parameter %s: %s is not '
                                     'existed!\n' % (pid, inf), log_file)
            elif len(StringClass.split_string(infile, ' ')) > 0:  # not mean to a file
                continue
            elif not os.path.exists(infile):
                TauDEM.error('Input files parameter %s: %s is not existed!\n' % (pid, infile),
                             log_file)

        # check output files to figure out if current run is necessary.
        # need_run = True
        # basedatetime = 0.
        # for (pid, path) in in_files.items():
        #     if path is not None and os.path.getmtime(path) > basedatetime:
        #         basedatetime = os.path.getmtime(path)
        # if out_files is not None:
        #     for (pid, out_file) in out_files.items():
        #         if out_file is not None and FileClass.is_up_to_date(out_file, basedatetime):
        #             need_run = False
        #             break
        # if not need_run:
        #     return True

        # remove out_files to avoid any file IO related error
        if out_files is not None:
            if not isinstance(out_files, dict):
                TauDEM.error('The output files parameter must be a dict!\n')
            for (pid, out_file) in out_files.items():
                if isinstance(out_file, list) or isinstance(out_file, tuple):
                    for outf in out_file:
                        if outf is not None:
                            FileClass.remove_files(outf)
                elif out_file is not None:
                    FileClass.remove_files(out_file)

        # concatenate command line
        commands = []
        # MPI header
        if mpi_params is not None:
            if not isinstance(mpi_params, dict):
                TauDEM.error('The MPI settings parameter must be a dict!\n')
            if 'mpipath' in mpi_params and mpi_params['mpipath'] is not None:
                commands.append(mpi_params['mpipath'] + os.sep + 'mpiexec')
            else:
                commands.append('mpiexec')
            if 'hostfile' in mpi_params and mpi_params['hostfile'] is not None \
                    and not StringClass.string_match(mpi_params['hostfile'], 'none') \
                    and os.path.isfile(mpi_params['hostfile']):
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
            if isinstance(infile, list) or isinstance(infile, tuple):
                commands.append(' '.join(tmpf for tmpf in infile))
            else:
                commands.append(infile)
        # append input parameters
        if in_params is not None:
            if not isinstance(in_params, dict):
                TauDEM.error('The input parameters must be a dict!\n')
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
                if isinstance(outfile, list) or isinstance(outfile, tuple):
                    commands.append(' '.join(tmpf for tmpf in outfile))
                else:
                    commands.append(outfile)
        # run command
        # print (commands)
        runmsg = UtilClass.run_command(commands)
        TauDEM.log(runmsg, log_file)
        return True

    @staticmethod
    def fullpath(name, dirname=None):
        """Return full path if available."""
        if name is None:
            return None
        if os.sep in name:  # name is full path already
            return name
        if dirname is not None:
            return dirname + os.sep + name
        else:
            return name

    @staticmethod
    def fill(np, workingdir, dem, filleddem, mpiexedir=None, exedir=None, log_file=None,
             hostfile=None):
        """Run pit remove using the flooding approach """
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('pitremove', exedir), {'-z': dem}, None,
                          {'-fel': filleddem},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def d8flowdir(np, workingdir, filleddem, flowdir, slope, mpiexedir=None, exedir=None,
                  log_file=None, hostfile=None):
        """Run D8 flow direction"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('d8flowdir', exedir), {'-fel': filleddem}, None,
                          {'-p': flowdir, '-sd8': slope},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def dinfflowdir(np, workingdir, filleddem, flowangle, slope, mpiexedir=None, exedir=None,
                    log_file=None, hostfile=None):
        """Run Dinf flow direction"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('dinfflowdir', exedir), {'-fel': filleddem}, None,
                          {'-ang': flowangle, '-slp': slope},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def aread8(np, workingdir, flowdir, acc, outlet=None, streamskeleton=None, mpiexedir=None,
               exedir=None, edgecontaimination=False, log_file=None, hostfile=None):
        """Run Accumulate area according to D8 flow direction"""
        # -nc means do not consider edge contaimination
        if not edgecontaimination:
            in_params = {'-nc': None}
        else:
            in_params = None
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('aread8', exedir),
                          {'-p': flowdir, '-o': outlet, '-wg': streamskeleton},
                          in_params, {'-ad8': acc},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def areadinf(np, workingdir, angfile, sca, outlet=None, wg=None, edgecontaimination=False,
                 mpiexedir=None, exedir=None, log_file=None, hostfile=None):
        """Run Accumulate area according to Dinf flow direction"""
        # -nc means do not consider edge contaimination
        if edgecontaimination:
            in_params = {'-nc': None}
        else:
            in_params = None
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('areadinf', exedir),
                          {'-ang': angfile, '-o': outlet, '-wg': wg},
                          in_params, {'-sca': sca},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def connectdown(np, workingdir, acc, outlet, mpiexedir=None,
                    exedir=None, log_file=None, hostfile=None):
        """Reads an ad8 contributing area file,
        identifies the location of the largest ad8 value as the outlet of the largest watershed"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('connectdown', exedir), {'-ad8': acc},
                          None, {'-o': outlet},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def gridnet(np, workingdir, pfile, plenfile, tlenfile, gordfile, outlet=None, mpiexedir=None,
                exedir=None, log_file=None, hostfile=None):
        """Run gridnet"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('gridnet', exedir), {'-p': pfile, '-o': outlet},
                          None, {'-plen': plenfile, '-tlen': tlenfile, '-gord': gordfile},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def threshold(np, workingdir, acc, stream_raster, threshold=100., mpiexedir=None, exedir=None,
                  log_file=None, hostfile=None):
        """Run threshold for stream raster"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('threshold', exedir),
                          {'-ssa': acc}, {'-thresh': threshold}, {'-src': stream_raster},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def streamnet(np, workingdir, filleddem, flowdir, acc, streamRaster, modifiedOutlet,
                  streamOrder, chNetwork, chCoord, streamNet, subbasin, mpiexedir=None,
                  exedir=None, log_file=None, hostfile=None):
        """Run streamnet"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('streamnet', exedir),
                          {'-fel': filleddem, '-p': flowdir, '-ad8': acc, '-src': streamRaster,
                           '-o': modifiedOutlet}, None,
                          {'-ord': streamOrder, '-tree': chNetwork, '-coord': chCoord,
                           '-net': streamNet, '-w': subbasin},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def moveoutletstostrm(np, workingdir, flowdir, streamRaster, outlet, modifiedOutlet,
                          mpiexedir=None,
                          exedir=None, log_file=None, hostfile=None):
        """Run move the given outlets to stream"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('moveoutletstostrm', exedir),
                          {'-p': flowdir, '-src': streamRaster, '-o': outlet}, None,
                          {'-om': modifiedOutlet},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def convertdistmethod(method_str):
        """Convert distance method to h, v, p, and s."""
        if StringClass.string_match(method_str, 'Horizontal'):
            return 'h'
        elif StringClass.string_match(method_str, 'Vertical'):
            return 'v'
        elif StringClass.string_match(method_str, 'Pythagoras'):
            return 'p'
        elif StringClass.string_match(method_str, 'Surface'):
            return 's'
        elif method_str.lower() in ['h', 'v', 'p', 's']:
            return method_str.lower()
        else:
            return 's'

    @staticmethod
    def convertstatsmethod(method_str):
        """Convert statistics method to ave, min, and max."""
        if StringClass.string_match(method_str, 'Average'):
            return 'ave'
        elif StringClass.string_match(method_str, 'Maximum'):
            return 'max'
        elif StringClass.string_match(method_str, 'Minimum'):
            return 'min'
        elif method_str.lower() in ['ave', 'max', 'min']:
            return method_str.lower()
        else:
            return 'ave'

    @staticmethod
    def d8distdowntostream(np, workingdir, p, fel, src, dist, distancemethod, thresh,
                           mpiexedir=None, exedir=None, log_file=None, hostfile=None):
        """Run D8 distance down to stream"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('d8distdowntostream', exedir),
                          {'-fel': fel, '-p': p, '-src': src},
                          {'-thresh': thresh, '-m': TauDEM.convertdistmethod(distancemethod)},
                          {'-dist': dist}, {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def dinfdistdown(np, workingdir, ang, fel, slp, src, statsm, distm, edgecontamination, wg, dist,
                     mpiexedir=None, exedir=None, log_file=None, hostfile=None):
        """Run D-inf distance down to stream"""
        os.chdir(workingdir)
        in_params = {'-m': '%s %s' % (TauDEM.convertstatsmethod(statsm),
                                      TauDEM.convertdistmethod(distm))}
        if StringClass.string_match(edgecontamination, 'false') or edgecontamination is False:
            in_params['-nc'] = None
        return TauDEM.run(TauDEM.fullpath('dinfdistdown', exedir),
                          {'-fel': fel, '-slp': slp, '-ang': ang, '-src': src, '-wg': wg},
                          in_params,
                          {'-dd': dist}, {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def peukerdouglas(np, workingdir, fel, streamSkeleton, mpiexedir=None, exedir=None,
                      log_file=None, hostfile=None):
        """Run peuker-douglas function"""
        os.chdir(workingdir)
        return TauDEM.run(TauDEM.fullpath('peukerdouglas', exedir), {'-fel': fel}, None,
                          {'-ss': streamSkeleton},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})

    @staticmethod
    def dropanalysis(np, workingdir, fel, p, ad8, ssa, outlet, minthresh, maxthresh, numthresh,
                     logspace, drp, mpiexedir=None, exedir=None, log_file=None, hostfile=None):
        """Drop analysis for optimal threshold for extracting stream."""
        os.chdir(workingdir)
        parstr = '%f %f %f' % (minthresh, maxthresh, numthresh)
        if logspace == 'false':
            parstr += ' 1'
        else:
            parstr += ' 0'
        return TauDEM.run(TauDEM.fullpath('dropanalysis', exedir),
                          {'-fel': fel, '-p': p, '-ad8': ad8, '-ssa': ssa, '-o': outlet},
                          {'-par': parstr},
                          {'-drp': drp}, {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': TauDEM.fullpath(log_file, workingdir)})


class TauDEMWorkflow(object):
    """Common used workflow based on TauDEM"""

    def __init__(self):
        """Empty function"""
        pass

    @staticmethod
    def watershed_delineation(bin_dir, mpi_bin, np, dem, outlet_file, thresh, d8_down_method,
                              namecfg, logfile=None, hostfile=None):
        """Watershed Delineation."""
        # 1. Check directories
        tau_dir = namecfg.workspace
        UtilClass.mkdir(tau_dir)
        # 2. Check log file
        if logfile is not None and FileClass.is_file_exists(logfile):
            os.remove(logfile)
        # 3. Get predefined intermediate file names
        filled_dem = namecfg.filldem
        flow_dir = namecfg.d8flow
        slope = namecfg.slp
        flow_dir_dinf = namecfg.dinf
        slope_dinf = namecfg.dinf_slp
        dir_code_dinf = namecfg.dinf_d8dir
        weight_dinf = namecfg.dinf_weight
        acc = namecfg.d8acc
        stream_raster = namecfg.stream_raster
        default_outlet = namecfg.outlet_pre
        modified_outlet = namecfg.outlet_m
        stream_skeleton = namecfg.stream_pd
        acc_with_weight = namecfg.d8acc_weight
        stream_order = namecfg.stream_order
        ch_network = namecfg.channel_net
        ch_coord = namecfg.channel_coord
        stream_net = namecfg.streamnet_shp
        subbasin = namecfg.subbsn
        dist2_stream_d8 = namecfg.dist2stream_d8

        # 4. perform calculation
        UtilClass.writelog(logfile, "[Output] %d..., %s" % (10, "fill DEM..."), 'a')
        TauDEM.fill(np, tau_dir, dem, filled_dem, mpi_bin, bin_dir,
                    log_file=logfile, hostfile=hostfile)
        UtilClass.writelog(logfile, "[Output] %d..., %s" %
                           (20, "Calculating D8 and Dinf flow direction..."), 'a')
        TauDEM.d8flowdir(np, tau_dir, filled_dem, flow_dir, slope,
                         mpi_bin, bin_dir, log_file=logfile, hostfile=hostfile)
        TauDEM.dinfflowdir(np, tau_dir, filled_dem, flow_dir_dinf, slope_dinf,
                           mpi_bin, bin_dir, log_file=logfile, hostfile=hostfile)
        DinfUtil.output_compressed_dinf(flow_dir_dinf, dir_code_dinf, weight_dinf)
        UtilClass.writelog(logfile, "[Output] %d..., %s" % (30, "D8 flow accumulation..."), 'a')
        TauDEM.aread8(np, tau_dir, flow_dir, acc, None, None, mpi_bin, bin_dir,
                      log_file=logfile, hostfile=hostfile)
        UtilClass.writelog(logfile, "[Output] %d..., %s" %
                           (40, "Generating stream raster initially..."), 'a')
        min_accum, max_accum, mean_accum, std_accum = RasterUtilClass.raster_statistics(acc)
        TauDEM.threshold(np, tau_dir, acc, stream_raster, mean_accum,
                         mpi_bin, bin_dir, log_file=logfile, hostfile=hostfile)
        UtilClass.writelog(logfile, "[Output] %d..., %s" % (50, "Moving outlet to stream..."), 'a')
        if outlet_file is None:
            outlet_file = default_outlet
            TauDEM.connectdown(np, tau_dir, acc, outlet_file, mpi_bin, bin_dir,
                               log_file=logfile, hostfile=hostfile)
        TauDEM.moveoutletstostrm(np, tau_dir, flow_dir, stream_raster, outlet_file,
                                 modified_outlet, mpi_bin, bin_dir,
                                 log_file=logfile, hostfile=hostfile)
        UtilClass.writelog(logfile, "[Output] %d..., %s" %
                           (60, "Generating stream skeleton..."), 'a')
        TauDEM.peukerdouglas(np, tau_dir, filled_dem, stream_skeleton,
                             mpi_bin, bin_dir, log_file=logfile, hostfile=hostfile)
        UtilClass.writelog(logfile, "[Output] %d..., %s" %
                           (70, "Flow accumulation with outlet..."), 'a')
        TauDEM.aread8(np, tau_dir, flow_dir, acc_with_weight, None, stream_skeleton,
                      mpi_bin, bin_dir, log_file=logfile, hostfile=hostfile)

        if thresh <= 0:  # find the optimal threshold using dropanalysis function
            UtilClass.writelog(logfile, "[Output] %d..., %s" %
                               (75, "Drop analysis to select optimal threshold..."), 'a')
            min_accum, max_accum, mean_accum, std_accum = \
                RasterUtilClass.raster_statistics(acc_with_weight)
            if mean_accum - std_accum < 0:
                minthresh = mean_accum
            else:
                minthresh = mean_accum - std_accum
            maxthresh = mean_accum + std_accum
            numthresh = 20
            logspace = 'true'
            drp_file = namecfg.drptxt
            TauDEM.dropanalysis(np, tau_dir, filled_dem, flow_dir, acc_with_weight,
                                acc_with_weight, modified_outlet, minthresh, maxthresh,
                                numthresh, logspace, drp_file, mpi_bin, bin_dir,
                                log_file=logfile, hostfile=hostfile)
            if not FileClass.is_file_exists(drp_file):
                raise RuntimeError("Dropanalysis failed and drp.txt was not created!")
            drpf = open(drp_file, "r")
            temp_contents = drpf.read()
            (beg, thresh) = temp_contents.rsplit(' ', 1)
            print (thresh)
            drpf.close()
        UtilClass.writelog(logfile, "[Output] %d..., %s" % (80, "Generating stream raster..."), 'a')
        TauDEM.threshold(np, tau_dir, acc_with_weight, stream_raster, float(thresh),
                         mpi_bin, bin_dir, log_file=logfile, hostfile=hostfile)
        UtilClass.writelog(logfile, "[Output] %d..., %s" % (90, "Generating stream net..."), 'a')
        TauDEM.streamnet(np, tau_dir, filled_dem, flow_dir, acc_with_weight, stream_raster,
                         modified_outlet, stream_order, ch_network,
                         ch_coord, stream_net, subbasin, mpi_bin, bin_dir,
                         log_file=logfile, hostfile=hostfile)
        UtilClass.writelog(logfile, "[Output] %d..., %s" %
                           (95, "Calculating distance to stream (D8)..."), 'a')
        TauDEM.d8distdowntostream(np, tau_dir, flow_dir, filled_dem, stream_raster,
                                  dist2_stream_d8, d8_down_method, 1,
                                  mpi_bin, bin_dir, log_file=logfile, hostfile=hostfile)
        UtilClass.writelog(logfile, "[Output] %d.., %s" %
                           (100, "Original subbasin delineation is finished!"), 'a')


def run_test():
    """run test function"""
    workingspace = r"D:\test"
    # execdir = r"C:\z_code\Hydro\SEIMS2017\seims\bin"
    # log = "taudem.log"
    dem = "dem_30m.tif"
    feldem = "dem_fel.tif"
    flowd8 = "flowd8.tif"
    sloped8 = "slpd8.tif"
    TauDEM.fill(1, workingspace, dem, feldem)
    TauDEM.d8flowdir(4, workingspace, feldem, flowd8, sloped8)


if __name__ == "__main__":
    run_test()
