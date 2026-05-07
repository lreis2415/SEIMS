# -*- coding: utf-8 -*-
"""TauDEM Utility Class for TauDEM original DTA tools and
     extended algorithms based on TauDEM parallelization framework.

   Thanks to the open-source software `TauDEM`_ by David Tarboton and `QSWAT`_ by Chris George.

   @author: Liangjun Zhu

   @changlog:

    - 12-04-12 jz - origin version.
    - 16-07-01 lj - reorganized for pygeoc.
    - 17-06-25 lj - check by pylint and reformat by Google style.
    - 21-04-07 lj - add MFD-md algorithm: flowmfdmd
    - 21-09-07 lj - remove unnecessary functions of watershed_delineation
    - 21-11-01 lj - separate TauDEM and TauDEM_Ext related classes
    - 23-10-30 lj - move taudem extension functions of AutoFuzSlpPos to here

   .. _TauDEM:
      https://github.com/dtarb/TauDEM
   .. _QSWAT:
      http://swat.tamu.edu/software/qswat/
"""
from __future__ import absolute_import, unicode_literals
from future.utils import iteritems

import os
from io import open
from typing import List, Dict, AnyStr, Optional

from osgeo.gdal import GDT_Int32
from pygeoc.postTauDEM import StreamnetUtil
from pygeoc.raster import RasterUtilClass
from pygeoc.vector import VectorUtilClass
from pygeoc.utils import UtilClass, MathClass, FileClass, StringClass, sysstr


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
    _SUBBASIN = 'subbasinTau.tif'
    # distance down to stream
    _DIST2STREAMDINF = 'dist2Stream_dinf.tif'
    # Masked by subbasins
    _D8FLOWDIRM = 'flowDirTauD8M.tif'
    # Serialized IDs of subbasins and streams
    _SUBBASINM = 'subbasinTauM.tif'
    _SUBBASINSHP = 'subbasin.shp'
    _SUBBASINID = 'SUBBASINID'
    _STREAMNETM = 'streamNetSerialized.shp'
    _STREAMRASTERM = 'streamRasterTauM.tif'

    def __init__(self, tau_dir):
        """assign taudem resulted file path"""
        tau_dir = os.path.abspath(tau_dir)
        self.workspace = tau_dir
        self.filldem = self.workspace + os.sep + self._FILLEDDEM
        self.d8flow = self.workspace + os.sep + self._D8FLOWDIR
        self.slp = self.workspace + os.sep + self._SLOPE
        self.d8acc = self.workspace + os.sep + self._D8ACC
        self.d8acc_weight = self.workspace + os.sep + self._D8ACCWITHWEIGHT
        self.stream_raster = self.workspace + os.sep + self._STREAMRASTER
        self.dinf = self.workspace + os.sep + self._FLOWDIRDINF
        self.dinf_d8dir = self.workspace + os.sep + self._DIRCODEDINF
        self.dinf_weight = self.workspace + os.sep + self._WEIGHTDINF
        self.dinf_slp = self.workspace + os.sep + self._SLOPEDINF
        self.outlet_pre = self.workspace + os.sep + self._DEFAULTOUTLET
        self.outlet_m = self.workspace + os.sep + self._MODIFIEDOUTLET
        self.stream_pd = self.workspace + os.sep + self._STREAMSKELETON
        self.drptxt = self.workspace + os.sep + self._DROPTXT
        self.stream_order = self.workspace + os.sep + self._STREAMORDER
        self.channel_net = self.workspace + os.sep + self._CHNETWORK
        self.channel_coord = self.workspace + os.sep + self._CHCOORD
        self.streamnet_shp = self.workspace + os.sep + self._STREAMNET
        self.subbsn = self.workspace + os.sep + self._SUBBASIN
        self.dist2stream_dinf = self.workspace + os.sep + self._DIST2STREAMDINF
        # Masked files by subbasins
        self.d8flow_m = self.workspace + os.sep + self._D8FLOWDIRM
        self.subbsn_m = self.workspace + os.sep + self._SUBBASINM
        self.subbsn_shp = self.workspace + os.sep + self._SUBBASINSHP
        self.streamnet_m = self.workspace + os.sep + self._STREAMNETM  # shp
        self.stream_m = self.workspace + os.sep + self._STREAMRASTERM  # tif


class TauDEMExtFiles(TauDEMFilesUtils):
    """predefined TauDEM_Ext resulted file names"""
    _DIRCODEMFDMD = 'dirCodeMFDmd.tif'
    _FLOWFRACTIONMFDMD = 'fractionsMFDmd.tif'
    _DIST2STREAMD8 = 'dist2Stream.tif'

    def __init__(self, tau_dir):
        """assign taudem resulted file path"""
        TauDEMFilesUtils.__init__(self, tau_dir)

        self.mfdmd_dir = self.workspace + os.sep + self._DIRCODEMFDMD
        self.mfdmd_frac = self.workspace + os.sep + self._FLOWFRACTIONMFDMD
        self.dist2stream_d8 = self.workspace + os.sep + self._DIST2STREAMD8


class TauDEM(object):
    """Methods for calling TauDEM executables."""

    def __init__(self):
        """Empty function"""
        pass

    @staticmethod
    def error(msg, log_file=None):
        """Print, output error message and raise RuntimeError."""
        UtilClass.print_msg(msg + os.linesep)
        if log_file is not None:
            UtilClass.writelog(log_file, msg, 'append')
        raise RuntimeError(msg)

    @staticmethod
    def log(lines, log_file=None):
        """Output log message."""
        err = False
        for line in lines:
            print(line)
            if log_file is not None:
                UtilClass.writelog(log_file, line, 'append')
            if 'BAD TERMINATION' in line.upper():
                err = True
                break
        if err:
            TauDEM.error('Error occurred when calling TauDEM function, please check!', log_file)

    @staticmethod
    def write_time_log(logfile, time):
        """Write time log."""
        if os.path.exists(logfile):
            log_status = open(logfile, 'a', encoding='utf-8')
        else:
            log_status = open(logfile, 'w', encoding='utf-8')
            log_status.write('Function Name\tRead Time\tCompute Time\tWrite Time\tTotal Time\t\n')
        log_status.write('%s\t%.5f\t%.5f\t%.5f\t%.5f\t\n' % (time['name'], time['readt'],
                                                             time['computet'],
                                                             time['writet'],
                                                             time['totalt']))
        log_status.flush()
        log_status.close()

    @staticmethod
    def output_runtime_to_log(title, lines, logfile):
        if logfile is None:
            return
        fname = FileClass.get_core_name_without_suffix(title)
        time_dict = {'name': fname, 'readt': 0, 'writet': 0, 'computet': 0, 'totalt': 0}
        for line in lines:
            # print(line)
            line = line.lower()
            time_value = line.split(os.linesep)[0].split(':')[-1]
            if not MathClass.isnumerical(time_value):
                continue
            time_value = float(time_value)
            if line.find('read') >= 0 and line.find('time') >= 0:
                time_dict['readt'] += time_value
            elif line.find('compute') >= 0 and line.find('time') >= 0:
                time_dict['computet'] += time_value
            elif line.find('write') >= 0 and line.find('time') >= 0:
                time_dict['writet'] += time_value
            elif line.find('total') >= 0 and line.find('time') >= 0:
                time_dict['totalt'] += time_value
        TauDEM.write_time_log(logfile, time_dict)

    @staticmethod
    def func_name(fname):
        if sysstr == 'Windows' and '.exe' not in fname:
            return fname + '.exe'
        else:
            return fname

    @staticmethod
    def check_infile_and_wp(curinf, curwp):
        """Check the existence of the given file and directory path.
           1. Raise Runtime exception if both not existed.
           2. If the ``curwp`` is None, the set the base folder of ``curinf`` to it.
        """
        if not os.path.exists(curinf):
            if curwp is None:
                TauDEM.error('You must specify one of the workspace and the '
                             'full path of input file!')
            curinf = curwp + os.sep + curinf
            curinf = os.path.abspath(curinf)
            if not os.path.exists(curinf):
                TauDEM.error('Input files parameter %s is not existed!' % curinf)
        else:
            curinf = os.path.abspath(curinf)
            if curwp is None:
                curwp = os.path.dirname(curinf)
        return curinf, curwp

    @staticmethod
    def run(function_name,  # type: AnyStr
            in_files,  # type: Dict[AnyStr, List[AnyStr]]
            wp=None,  # type: Optional[AnyStr]
            in_params=None,  # type: Optional[Dict[AnyStr, Optional[int, float, AnyStr, List[AnyStr]]]]
            out_files=None,  # type: Optional[Dict[AnyStr, List[AnyStr, List[AnyStr]]]]
            mpi_params=None,  # type: Optional[Dict[AnyStr, List[int, AnyStr]]]
            log_params=None,  # type: Optional[Dict[AnyStr, AnyStr]]
            ignore_err=False  # type: Optional[bool]
            ):
        # type: (...) -> bool
        """Run TauDEM function.

         - 1. The command will not execute if any input file does not exist.
         - 2. An error will be detected after running the TauDEM command if
              any output file does not exist;

        Args:
            function_name (str): Full path of TauDEM function.
            in_files (dict, required): Dict of pairs of parameter id (string) and file path
                (string or list) for input files, e.g.::

                    {'-z': '/full/path/to/dem.tif'}

            wp (str, optional): Workspace for outputs. If not specified, the directory of the
                first input file in ``in_files`` will be used.
            in_params (dict, optional): Dict of pairs of parameter id (string) and value
                (or list, or None for a flag parameter without a value) for input parameters, e.g.::

                    {'-nc': None}
                    {'-thresh': threshold}
                    {'-m': ['ave', 's'], '-nc': None}

            out_files (dict, optional): Dict of pairs of parameter id (string) and file
                path (string or list) for output files, e.g.::

                    {'-fel': 'filleddem.tif'}
                    {'-maxS': ['harden.tif', 'maxsimi.tif']}

            mpi_params (dict, optional): Dict of pairs of parameter id (string) and value or
                path for MPI setting, e.g.::

                    {'mpipath':'/soft/bin','hostfile':'/soft/bin/cluster.node','n':4}
                    {'mpipath':'/soft/bin', 'n':4}
                    {'n':4}

            log_params (dict, optional): Dict of pairs of parameter id (string) and value or
                path for runtime and log output parameters. e.g.::

                    {'logfile': '/home/user/log.txt',
                     'runtimefile': '/home/user/runtime.txt'}

            ignore_err (bool, optional): Ignore errors of verify the existence of output files

        Returns:
            True if TauDEM run successfully, otherwise False.
        """
        # Check input files
        if in_files is None:
            TauDEM.error('Input files parameter is required!')
        if not isinstance(in_files, dict):
            TauDEM.error('The input files parameter must be a dict!')
        for (pid, infile) in iteritems(in_files):
            if infile is None:
                continue
            if isinstance(infile, list) or isinstance(infile, tuple):
                for idx, inf in enumerate(infile):
                    if inf is None:
                        continue
                    inf, wp = TauDEM.check_infile_and_wp(inf, wp)
                    in_files[pid][idx] = FileClass.get_file_fullpath_string(inf)
                continue
            if os.path.exists(infile):
                infile, wp = TauDEM.check_infile_and_wp(infile, wp)
                in_files[pid] = FileClass.get_file_fullpath_string(infile)
            else:
                # For more flexible input files extension.
                # e.g., -inputtags 1 <path/to/tag1.tif> 2 <path/to/tag2.tif> ...
                # in such unpredictable circumstance, we cannot check the existence of
                # input files, so the developer will check it in other place.
                infile_list = StringClass.split_string(infile, ' ')
                if len(infile_list) > 1:
                    in_files[pid] = infile_list
                else:  # the infile still should be a existing file, so check in workspace
                    if wp is None:
                        TauDEM.error('Workspace should not be None!')
                    infile = wp + os.sep + infile
                    if not os.path.exists(infile):
                        TauDEM.error('Input files parameter %s: %s is not existed!' %
                                     (pid, infile))
                    in_files[pid] = FileClass.get_file_fullpath_string(infile)
        # Make workspace dir if not existed
        UtilClass.mkdir(wp)
        # Check the log parameter
        log_file = None
        runtime_file = None
        if log_params is not None:
            if not isinstance(log_params, dict):
                TauDEM.error('The log parameter must be a dict!')
            if 'logfile' in log_params and log_params['logfile'] is not None:
                log_file = log_params['logfile']
                # If log_file is just a file name, then save it in the default workspace.
                if os.sep not in log_file:
                    log_file = wp + os.sep + log_file
                    log_file = os.path.abspath(log_file)
            if 'runtimefile' in log_params and log_params['runtimefile'] is not None:
                runtime_file = log_params['runtimefile']
                # If log_file is just a file name, then save it in the default workspace.
                if os.sep not in runtime_file:
                    runtime_file = wp + os.sep + runtime_file
                    runtime_file = os.path.abspath(runtime_file)

        # remove out_files to avoid any file IO related error
        new_out_files = list()
        if out_files is not None:
            if not isinstance(out_files, dict):
                TauDEM.error('The output files parameter must be a dict!')
            for (pid, out_file) in iteritems(out_files):
                if out_file is None:
                    continue
                if isinstance(out_file, list) or isinstance(out_file, tuple):
                    for idx, outf in enumerate(out_file):
                        if outf is None:
                            continue
                        outf = FileClass.get_file_fullpath(outf, wp)
                        FileClass.remove_files(outf)
                        outf_str = FileClass.get_file_fullpath_string(outf)
                        out_files[pid][idx] = outf_str
                        new_out_files.append(outf_str)
                else:
                    out_file = FileClass.get_file_fullpath(out_file, wp)
                    FileClass.remove_files(out_file)
                    out_file_str = FileClass.get_file_fullpath_string(out_file)
                    out_files[pid] = out_file_str
                    new_out_files.append(out_file_str)

        # concatenate command line
        commands = list()
        # MPI header
        if mpi_params is not None:
            if not isinstance(mpi_params, dict):
                TauDEM.error('The MPI settings parameter must be a dict!')
            if 'mpipath' in mpi_params and mpi_params['mpipath'] is not None:
                if os.path.isfile(mpi_params['mpipath']):  # in case it is fullpath of mpiexec
                    commands.append(mpi_params['mpipath'])
                elif os.path.isdir(mpi_params['mpipath']):
                    commands.append(mpi_params['mpipath'] + os.sep + 'mpiexec')
                elif mpi_params['mpipath'][0] == mpi_params['mpipath'][-1] == '"' and \
                      os.path.isdir(mpi_params['mpipath'][1:-1]):
                    commands.append('\"%s\\mpiexec\"' %
                                    os.path.dirname(mpi_params['mpipath'][1:-1]))
                else:
                    commands.append('mpiexec')
            else:
                commands.append('mpiexec')
            if 'hostfile' in mpi_params and mpi_params['hostfile'] is not None \
                    and not StringClass.string_match(mpi_params['hostfile'], 'none') \
                    and os.path.isfile(mpi_params['hostfile']):
                commands.append('-f')
                commands.append(FileClass.get_file_fullpath_string(mpi_params['hostfile']))
            if 'n' in mpi_params and mpi_params['n'] > 1:
                commands.append('-n')
                commands.append(repr(mpi_params['n']))
            else:  # If number of processor is less equal than 1, then do not call mpiexec.
                commands = list()
        # append TauDEM function name, which can be full path or just one name
        commands.append(function_name)
        # append input files
        for (pid, infile) in iteritems(in_files):
            if infile is None:
                continue
            if pid[0] != '-':
                pid = '-' + pid
            commands.append(pid)
            if isinstance(infile, list) or isinstance(infile, tuple):
                if sysstr == 'Windows':
                    commands.append(' '.join(tmpf for tmpf in infile))
                else:
                    for tmpf in infile:
                        commands.append(tmpf)
            else:
                commands.append(infile)
        # append input parameters
        if in_params is not None:
            if not isinstance(in_params, dict):
                TauDEM.error('The input parameters must be a dict!')
            for (pid, v) in iteritems(in_params):
                if pid[0] != '-':
                    pid = '-' + pid
                commands.append(pid)
                # allow for parameter which is a flag without value
                if v != '' and v is not None:
                    if isinstance(v, list) or isinstance(v, tuple):
                        for tmppar in v:
                            if MathClass.isnumerical(tmppar):
                                commands.append(repr(tmppar))
                            else:
                                commands.append(str(tmppar))
                    else:
                        if MathClass.isnumerical(v):
                            commands.append(repr(v))
                        else:
                            commands.append(str(v))
        # append output parameters
        if out_files is not None:
            for (pid, outfile) in iteritems(out_files):
                if outfile is None:
                    continue
                if pid[0] != '-':
                    pid = '-' + pid
                commands.append(pid)
                if isinstance(outfile, list) or isinstance(outfile, tuple):
                    if sysstr == 'Windows':
                        commands.append(' '.join(tmpf for tmpf in outfile))
                    else:
                        for tmpf in outfile:
                            commands.append(tmpf)
                else:
                    commands.append(outfile)
        # run command
        runmsg = UtilClass.run_command(commands)
        TauDEM.log(runmsg, log_file)
        TauDEM.output_runtime_to_log(function_name, runmsg, runtime_file)
        # Check out_files, raise RuntimeError if not exist.
        for of in new_out_files:
            if not os.path.exists(of) and not ignore_err:
                TauDEM.error('%s failed, and the %s was not generated!' % (function_name, of))
                return False
        return True

    @staticmethod
    def pitremove(np, dem, filleddem, workingdir=None, mpiexedir=None, exedir=None, log_file=None,
                  runtime_file=None, hostfile=None):
        """Run pit remove using the flooding approach """
        fname = TauDEM.func_name('pitremove')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-z': dem}, workingdir,
                          None,
                          {'-fel': filleddem},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def d8flowdir(np, filleddem, flowdir, slope, workingdir=None, mpiexedir=None, exedir=None,
                  log_file=None, runtime_file=None, hostfile=None):
        """Run D8 flow direction"""
        fname = TauDEM.func_name('d8flowdir')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-fel': filleddem}, workingdir,
                          None,
                          {'-p': flowdir, '-sd8': slope},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def dinfflowdir(np, filleddem, flowangle, slope, workingdir=None, mpiexedir=None, exedir=None,
                    log_file=None, runtime_file=None, hostfile=None):
        """Run Dinf flow direction"""
        fname = TauDEM.func_name('dinfflowdir')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-fel': filleddem}, workingdir,
                          None,
                          {'-ang': flowangle, '-slp': slope},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def aread8(np, flowdir, acc, outlet=None, streamskeleton=None, edgecontaimination=False,
               workingdir=None, mpiexedir=None, exedir=None,
               log_file=None, runtime_file=None, hostfile=None):
        """Run Accumulate area according to D8 flow direction"""
        # -nc means do not consider edge contaimination
        if not edgecontaimination:
            in_params = {'-nc': None}
        else:
            in_params = None
        fname = TauDEM.func_name('aread8')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-p': flowdir, '-o': outlet, '-wg': streamskeleton}, workingdir,
                          in_params,
                          {'-ad8': acc},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def areadinf(np, angfile, sca, outlet=None, wg=None, edgecontaimination=False,
                 workingdir=None, mpiexedir=None, exedir=None,
                 log_file=None, runtime_file=None, hostfile=None):
        """Run Accumulate area according to Dinf flow direction"""
        # -nc means do not consider edge contaimination
        if edgecontaimination:
            in_params = {'-nc': None}
        else:
            in_params = None
        fname = TauDEM.func_name('areadinf')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-ang': angfile, '-o': outlet, '-wg': wg}, workingdir,
                          in_params,
                          {'-sca': sca},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def connectdown(np, p, acc, outlet, modifiedOutlet, wtsd=None, maxdist=50,
                    workingdir=None, mpiexedir=None,
                    exedir=None, log_file=None, runtime_file=None, hostfile=None):
        """Reads an ad8 contributing area file,
        identifies the location of the largest ad8 value as the outlet of the largest watershed"""
        # If watershed is not specified, use acc to generate a mask layer.
        if wtsd is None or not os.path.isfile(wtsd):
            p, workingdir = TauDEM.check_infile_and_wp(p, workingdir)
            wtsd = workingdir + os.sep + 'wtsd_default.tif'
            RasterUtilClass.get_mask_from_raster(p, wtsd, True)
        fname = TauDEM.func_name('connectdown')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-p': p, '-ad8': acc, '-w': wtsd},
                          workingdir,
                          {'-d': maxdist},
                          {'-o': outlet, '-od': modifiedOutlet},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def gridnet(np, pfile, plenfile, tlenfile, gordfile, outlet=None, workingdir=None,
                mpiexedir=None, exedir=None, log_file=None, runtime_file=None, hostfile=None):
        """Run gridnet"""
        fname = TauDEM.func_name('gridnet')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-p': pfile, '-o': outlet}, workingdir,
                          None,
                          {'-plen': plenfile, '-tlen': tlenfile, '-gord': gordfile},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def threshold(np, acc, stream_raster, threshold=100., workingdir=None,
                  mpiexedir=None, exedir=None, log_file=None, runtime_file=None, hostfile=None):
        """Run threshold for stream raster"""
        fname = TauDEM.func_name('threshold')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-ssa': acc}, workingdir,
                          {'-thresh': threshold},
                          {'-src': stream_raster},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def streamnet(np, filleddem, flowdir, acc, streamRaster, modifiedOutlet,
                  streamOrder, chNetwork, chCoord, streamNet, subbasin, workingdir=None,
                  mpiexedir=None, exedir=None, log_file=None, runtime_file=None, hostfile=None):
        """Run streamnet"""
        fname = TauDEM.func_name('streamnet')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-fel': filleddem, '-p': flowdir, '-ad8': acc, '-src': streamRaster,
                           '-o': modifiedOutlet}, workingdir,
                          None,
                          {'-ord': streamOrder, '-tree': chNetwork, '-coord': chCoord,
                           '-net': streamNet, '-w': subbasin},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def moveoutletstostrm(np, flowdir, streamRaster, outlet, modifiedOutlet, maxdist=50,
                          workingdir=None, mpiexedir=None,
                          exedir=None, log_file=None, runtime_file=None, hostfile=None):
        """Run move the given outlets to stream"""
        fname = TauDEM.func_name('moveoutletstostrm')
        legacy_names = [TauDEM.func_name('moveoutletstostreams')]  # name in old TauDEM version
        for lname in legacy_names:
            if FileClass.get_executable_fullpath(lname, exedir, raise_exception=False) is not None:
                fname = lname
                break
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-p': flowdir, '-src': streamRaster, '-o': outlet},
                          workingdir,
                          {'-md': maxdist},
                          {'-om': modifiedOutlet},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

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
    def d8hdisttostrm(np, p, src, dist, thresh, workingdir=None,
                      mpiexedir=None, exedir=None, log_file=None, runtime_file=None, hostfile=None):
        """Run D8 horizontal distance down to stream.
        """
        fname = TauDEM.func_name('d8hdisttostrm')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-p': p, '-src': src},
                          workingdir,
                          {'-thresh': thresh},
                          {'-dist': dist},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def dinfdistdown(np, ang, fel, slp, src, statsm, distm, edgecontamination, wg, dist,
                     workingdir=None, mpiexedir=None, exedir=None,
                     log_file=None, runtime_file=None, hostfile=None):
        """Run D-inf distance down to stream"""
        in_params = {'-m': [TauDEM.convertstatsmethod(statsm), TauDEM.convertdistmethod(distm)]}
        if StringClass.string_match(repr(edgecontamination), 'false') or edgecontamination is False:
            in_params['-nc'] = None
        fname = TauDEM.func_name('dinfdistdown')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-fel': fel, '-slp': slp, '-ang': ang, '-src': src, '-wg': wg},
                          workingdir,
                          in_params,
                          {'-dd': dist},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def peukerdouglas(np, fel, streamSkeleton, workingdir=None, mpiexedir=None, exedir=None,
                      log_file=None, runtime_file=None, hostfile=None):
        """Run peuker-douglas function"""
        fname = TauDEM.func_name('peukerdouglas')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-fel': fel}, workingdir,
                          None,
                          {'-ss': streamSkeleton},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def dropanalysis(np, fel, p, ad8, ssa, outlet, minthresh, maxthresh, numthresh,
                     logspace, drp, workingdir=None,
                     mpiexedir=None, exedir=None, log_file=None, runtime_file=None, hostfile=None):
        """Drop analysis for optimal threshold for extracting stream."""
        parlist = [minthresh, maxthresh, numthresh]
        if logspace == 'false':
            parlist.append(1)
        else:
            parlist.append(0)
        fname = TauDEM.func_name('dropanalysis')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-fel': fel, '-p': p, '-ad8': ad8, '-ssa': ssa, '-o': outlet},
                          workingdir,
                          {'-par': parlist},
                          {'-drp': drp},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})


class TauDEM_Ext(TauDEM):
    """TauDEM extended DTA tools."""
    def __init__(self):
        """Empty function"""
        TauDEM.__init__(self)

    @staticmethod
    def d8distdowntostream(np, p, fel, src, dist, distancemethod, thresh, workingdir=None,
                           mpiexedir=None, exedir=None,
                           log_file=None, runtime_file=None, hostfile=None):
        """Run D8 distance down to stream by different method for distance.
        This function is extended from d8hdisttostrm by Liangjun.

        Please clone `TauDEM_ext by lreis2415`_ and compile for this program.

        .. _TauDEM_ext by lreis2415:
           https://github.com/lreis2415/TauDEM_ext
        """
        fname = TauDEM.func_name('d8distdowntostream')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-fel': fel, '-p': p, '-src': src},
                          workingdir,
                          {'-thresh': thresh, '-m': TauDEM.convertdistmethod(distancemethod)},
                          {'-dist': dist},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def d8distuptoridge(np, p, fel, src, dist, distm,
                        workingdir=None, mpiexedir=None, exedir=None,
                        log_file=None, runtime_file=None, hostfile=None):
        """Run D8 distance to stream"""
        fname = TauDEM_Ext.func_name('d8distuptoridge')
        return TauDEM_Ext.run(FileClass.get_executable_fullpath(fname, exedir),
                              in_files={'-fel': fel, '-p': p, '-src': src},
                              wp=workingdir,
                              in_params={'-m': TauDEM_Ext.convertdistmethod(distm)},
                              out_files={'-du': dist},
                              mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                              log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def dinfdistuptoridge(np, ang, fel, slp, propthresh, dist, statsm, distm,
                          edgecontamination, rdg=None,
                          workingdir=None, mpiexedir=None, exedir=None,
                          log_file=None, runtime_file=None, hostfile=None):
        """Run Dinf distance to ridge."""
        fname = TauDEM_Ext.func_name('dinfdistuptoridge')
        in_params = {'-thresh': str(propthresh),
                     '-m': [TauDEM_Ext.convertstatsmethod(statsm), TauDEM_Ext.convertdistmethod(distm)]}
        if StringClass.string_match(edgecontamination, 'false') or edgecontamination is False:
            in_params['-nc'] = None
        return TauDEM_Ext.run(FileClass.get_executable_fullpath(fname, exedir),
                              in_files={'-ang': ang, '-fel': fel, '-slp': slp, '-rdg': rdg},
                              wp=workingdir,
                              in_params=in_params,
                              out_files={'-du': dist},
                              mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                              log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def extractridge(np, angfile, elevfile, rdgsrc,
                     workingdir=None, mpiexedir=None, exedir=None,
                     log_file=None, runtime_file=None, hostfile=None):
        """Extract ridge source."""
        fname = TauDEM_Ext.func_name('ridgeextraction')
        return TauDEM_Ext.run(FileClass.get_executable_fullpath(fname, exedir),
                              in_files={'-dir': angfile, '-fel': elevfile},
                              wp=workingdir,
                              in_params=None,
                              out_files={'-src': rdgsrc},
                              mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                              log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def rpiskidmore(np, vlysrc, rdgsrc, rpi, vlytag=1, rdgtag=1, dist2vly=None, dist2rdg=None,
                    workingdir=None, mpiexedir=None, exedir=None,
                    log_file=None, runtime_file=None, hostfile=None):
        """Calculate RPI according to Skidmore (1990)."""
        fname = TauDEM_Ext.func_name('rpiskidmore')
        in_params = dict()
        if vlytag > 0:
            in_params['-vlytag'] = vlytag
        if rdgtag > 0:
            in_params['-rdgtag'] = rdgtag
        return TauDEM_Ext.run(FileClass.get_executable_fullpath(fname, exedir),
                              in_files={'-vly': vlysrc, '-rdg': rdgsrc},
                              wp=workingdir,
                              in_params=in_params,
                              out_files={'-rpi': rpi, '-dist2vly': dist2vly, '-dist2rdg': dist2rdg},
                              mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                              log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def curvature(np, fel, profc,
                  horizc=None, planc=None, unspherc=None, avec=None, maxc=None, minc=None,
                  workingdir=None, mpiexedir=None, exedir=None,
                  log_file=None, runtime_file=None, hostfile=None):
        """Calculate various curvature."""
        fname = TauDEM_Ext.func_name('curvature')
        return TauDEM_Ext.run(FileClass.get_executable_fullpath(fname, exedir),
                              in_files={'-fel': fel},
                              wp=workingdir,
                              in_params=None,
                              out_files={'-prof': profc, '-plan': planc, '-horiz': horizc,
                                         '-unspher': unspherc, '-ave': avec, '-max': maxc,
                                         '-min': minc},
                              mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                              log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def simplecalculator(np, inputa, inputb, output, operator,
                         workingdir=None, mpiexedir=None, exedir=None,
                         log_file=None, runtime_file=None, hostfile=None):
        """Run simple calculator.

           operator = 0: add
                      1: minus
                      2: multiply
                      3: divide
                      4: a/(a+b)
                      5: mask
        """
        fname = TauDEM_Ext.func_name('simplecalculator')
        return TauDEM_Ext.run(FileClass.get_executable_fullpath(fname, exedir),
                              in_files={'-in': [inputa, inputb]},
                              wp=workingdir,
                              in_params={'-op': operator},
                              out_files={'-out': output},
                              mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                              log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def selecttyplocslppos(np, inputconf, outputconf=None, extlog=None,
                           workingdir=None, mpiexedir=None, exedir=None,
                           log_file=None, runtime_file=None, hostfile=None):
        """Select typical locations."""
        fname = TauDEM_Ext.func_name('selecttyplocslppos')
        return TauDEM_Ext.run(FileClass.get_executable_fullpath(fname, exedir),
                              in_files={'-in': inputconf},
                              wp=workingdir,
                              in_params=None,
                              out_files={'-out': outputconf, '-extlog': extlog},
                              mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                              log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def fuzzyslpposinference(np, config, workingdir=None, mpiexedir=None,
                             exedir=None, log_file=None, runtime_file=None, hostfile=None):
        """Run fuzzy inference."""
        fname = TauDEM_Ext.func_name('fuzzyslpposinference')
        return TauDEM_Ext.run(FileClass.get_executable_fullpath(fname, exedir),
                              in_files={'-in': config},
                              wp=workingdir,
                              in_params=None,
                              out_files=None,
                              mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                              log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def hardenslppos(np, simifiles, tags, hard, maxsimi,
                     sechard=None, secsimi=None, spsim=None, spsi=None,
                     workingdir=None, mpiexedir=None, exedir=None,
                     log_file=None, runtime_file=None, hostfile=None):
        """Select typical locations."""
        fname = TauDEM_Ext.func_name('hardenslppos')
        if len(simifiles) != len(tags):
            raise RuntimeError("hardenslppos: simifiles and tags must have the same size!")
        tag_path = ''
        tag_list = list()
        tag_list.append('%d' % len(simifiles))
        for i, tag in enumerate(tags):
            tag_path += ' %d %s' % (tag, simifiles[i])
            tag_list.append('%d' % tag)
            tag_list.append(simifiles[i])
        in_params = dict()
        if spsim is not None and spsi is not None:
            in_params['-m'] = '%d %s' % (spsim, spsi)
        return TauDEM_Ext.run(FileClass.get_executable_fullpath(fname, exedir),
                              in_files={'-inf': '%d%s' % (len(simifiles), tag_path)},
                              # in_files={'-inf': tag_list},
                              wp=workingdir,
                              in_params=in_params,
                              out_files={'-maxS': [hard, maxsimi], '-secS': [sechard, secsimi]},
                              mpi_params={'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                              log_params={'logfile': log_file, 'runtimefile': runtime_file})

    @staticmethod
    def mfdmdflowdir(np, filleddem, flowdir, portion, min_portion=0.01,
                     p0=1.1, rng=8.9, lb=0., ub=1.,
                     workingdir=None, mpiexedir=None, exedir=None,
                     log_file=None, runtime_file=None, hostfile=None):
        """Run MFD-md flow direction (Qin et al., 2007)"""
        fname = TauDEM.func_name('flowmfdmd')
        return TauDEM.run(FileClass.get_executable_fullpath(fname, exedir),
                          {'-dem': filleddem}, workingdir,
                          {'-min_portion': min_portion,
                           '-p0': p0, '-range': rng, '-tanb_lb': lb, '-tanb_ub': ub},
                          {'-mfd': flowdir, '-portion': portion},
                          {'mpipath': mpiexedir, 'hostfile': hostfile, 'n': np},
                          {'logfile': log_file, 'runtimefile': runtime_file},
                          ignore_err=True)
        # The output flow fraction files are not the same with the argument 'portion'

class TauDEMWorkflow(object):
    """Common used workflow based on TauDEM"""

    def __init__(self):
        """Empty function"""
        pass

    @staticmethod
    def watershed_delineation(np, dem, outlet_file=None, thresh=0, singlebasin=False,
                              workingdir=None, mpi_bin=None, bin_dir=None,
                              logfile=None, runtime_file=None, hostfile=None,
                              avoid_redo=False):
        """Watershed Delineation based on D8 flow direction.

        Args:
            np: process number for MPI
            dem: DEM path
            outlet_file: predefined outlet shapefile path
            thresh: predefined threshold for extracting stream from accumulated flow direction
            singlebasin: when set True, only extract subbasins that drains into predefined outlets
            workingdir: directory that store outputs
            mpi_bin: directory of MPI executable binary, e.g., mpiexec, mpirun
            bin_dir: directory of TauDEM and other executable binaries
            logfile: log file path
            runtime_file: runtime file path
            hostfile: host list file path for MPI
            avoid_redo: avoid executing some functions that do not depend on input arguments
                        when repeatedly invoke this function
        """
        # 1. Check directories
        if not os.path.exists(dem):
            TauDEM.error('DEM: %s is not existed!' % dem)
        dem = os.path.abspath(dem)
        if workingdir is None or workingdir == '':
            workingdir = os.path.dirname(dem)
        nc = TauDEMFilesUtils(workingdir)  # predefined names
        workingdir = nc.workspace
        UtilClass.mkdir(workingdir)
        # 2. Check log file
        if logfile is not None and FileClass.is_file_exists(logfile):
            os.remove(logfile)
        # 3. perform calculation
        # Filling DEM
        if not (avoid_redo and FileClass.is_file_exists(nc.filldem)):
            UtilClass.writelog(logfile, '[Output] %s' % 'remove pit...', 'a')
            TauDEM.pitremove(np, dem, nc.filldem, workingdir, mpi_bin, bin_dir,
                             log_file=logfile, runtime_file=runtime_file, hostfile=hostfile)
        # Flow direction based on D8 algorithm
        if not (avoid_redo and FileClass.is_file_exists(nc.d8flow)):
            UtilClass.writelog(logfile, '[Output] %s' % 'D8 flow direction...', 'a')
            TauDEM.d8flowdir(np, nc.filldem, nc.d8flow, nc.slp, workingdir,
                             mpi_bin, bin_dir, log_file=logfile,
                             runtime_file=runtime_file, hostfile=hostfile)
        # Flow accumulation without stream skeleton as weight
        if not (avoid_redo and FileClass.is_file_exists(nc.d8acc)):
            UtilClass.writelog(logfile, '[Output] %s' % 'D8 flow accumulation...', 'a')
            TauDEM.aread8(np, nc.d8flow, nc.d8acc, None, None, False, workingdir, mpi_bin, bin_dir,
                          log_file=logfile, runtime_file=runtime_file, hostfile=hostfile)
        # Initial stream network using mean accumulation as threshold
        UtilClass.writelog(logfile, '[Output] %s' % 'Generating stream raster initially...', 'a')
        min_accum, max_accum, mean_accum, std_accum = RasterUtilClass.raster_statistics(nc.d8acc)
        TauDEM.threshold(np, nc.d8acc, nc.stream_raster, mean_accum, workingdir,
                         mpi_bin, bin_dir, log_file=logfile,
                         runtime_file=runtime_file, hostfile=hostfile)
        # Outlets position initialization and adjustment
        UtilClass.writelog(logfile, '[Output] %s' % 'Moving outlet to stream...', 'a')
        if outlet_file is None:  # if not given, take cell with maximum accumulation as outlet
            outlet_file = nc.outlet_pre
            TauDEM.connectdown(np, nc.d8flow, nc.d8acc, outlet_file, nc.outlet_m, wtsd=None,
                               workingdir=workingdir, mpiexedir=mpi_bin, exedir=bin_dir,
                               log_file=logfile, runtime_file=runtime_file, hostfile=hostfile)
        TauDEM.moveoutletstostrm(np, nc.d8flow, nc.stream_raster, outlet_file,
                                 nc.outlet_m, workingdir=workingdir,
                                 mpiexedir=mpi_bin, exedir=bin_dir,
                                 log_file=logfile, runtime_file=runtime_file, hostfile=hostfile)
        # Stream skeleton by peuker-douglas algorithm
        UtilClass.writelog(logfile, '[Output] %s' % 'Generating stream skeleton ...', 'a')
        TauDEM.peukerdouglas(np, nc.filldem, nc.stream_pd, workingdir,
                             mpi_bin, bin_dir, log_file=logfile,
                             runtime_file=runtime_file, hostfile=hostfile)
        # Weighted flow acculation with outlet
        UtilClass.writelog(logfile, '[Output] %s' % 'Flow accumulation with outlet...', 'a')
        tmp_outlet = None
        if singlebasin:
            tmp_outlet = nc.outlet_m
        TauDEM.aread8(np, nc.d8flow, nc.d8acc_weight, tmp_outlet, nc.stream_pd, False,
                      workingdir, mpi_bin, bin_dir, log_file=logfile,
                      runtime_file=runtime_file, hostfile=hostfile)
        # Determine threshold by input argument or dropanalysis function
        if thresh <= 0:  # find the optimal threshold using dropanalysis function
            UtilClass.writelog(logfile, '[Output] %s' %
                               'Drop analysis to select optimal threshold...', 'a')
            min_accum, max_accum, mean_accum, std_accum = \
                RasterUtilClass.raster_statistics(nc.d8acc_weight)
            if mean_accum - std_accum < 0:
                minthresh = mean_accum
            else:
                minthresh = mean_accum - std_accum
            maxthresh = mean_accum + std_accum
            TauDEM.dropanalysis(np, nc.filldem, nc.d8flow, nc.d8acc_weight,
                                nc.d8acc_weight, nc.outlet_m, minthresh, maxthresh,
                                20, 'true', nc.drptxt, workingdir, mpi_bin, bin_dir,
                                log_file=logfile, runtime_file=runtime_file, hostfile=hostfile)
            if not FileClass.is_file_exists(nc.drptxt):
                # raise RuntimeError('Dropanalysis failed and drp.txt was not created!')
                UtilClass.writelog(logfile, '[Output] %s' %
                                   'dropanalysis failed!', 'a')
                thresh = 0.5 * (maxthresh - minthresh) + minthresh
            else:
                with open(nc.drptxt, 'r', encoding='utf-8') as drpf:
                    temp_contents = drpf.read()
                    (beg, thresh) = temp_contents.rsplit(' ', 1)
            thresh = float(thresh)
            UtilClass.writelog(logfile, '[Output] %s: %f' %
                               ('Selected optimal threshold: ', thresh), 'a')
        # Final stream network
        UtilClass.writelog(logfile, '[Output] %s' % 'Generating stream raster...', 'a')
        TauDEM.threshold(np, nc.d8acc_weight, nc.stream_raster, thresh,
                         workingdir, mpi_bin, bin_dir, log_file=logfile,
                         runtime_file=runtime_file, hostfile=hostfile)
        UtilClass.writelog(logfile, '[Output] %s' % 'Generating stream net...', 'a')
        TauDEM.streamnet(np, nc.filldem, nc.d8flow, nc.d8acc_weight, nc.stream_raster,
                         nc.outlet_m, nc.stream_order, nc.channel_net,
                         nc.channel_coord, nc.streamnet_shp, nc.subbsn,
                         workingdir, mpi_bin, bin_dir,
                         log_file=logfile, runtime_file=runtime_file, hostfile=hostfile)
        # Serialize IDs of subbasins and the corresponding streams
        UtilClass.writelog(logfile, '[Output] %s' % 'Serialize subbasin&stream IDs...', 'a')
        id_map = StreamnetUtil.serialize_streamnet(nc.streamnet_shp, nc.streamnet_m)
        RasterUtilClass.raster_reclassify(nc.subbsn, id_map, nc.subbsn_m, GDT_Int32)
        StreamnetUtil.assign_stream_id_raster(nc.stream_raster, nc.subbsn_m, nc.stream_m)
        # convert raster to shapefile (for subbasin and basin)
        UtilClass.writelog(logfile, '[Output] %s' % 'Generating subbasin vector...', 'a')
        VectorUtilClass.raster2shp(nc.subbsn_m, nc.subbsn_shp, 'subbasin', 'SUBBASINID')
        # Finish the workflow
        UtilClass.writelog(logfile, '[Output] %s' %
                           'Original subbasin delineation is finished!', 'a')


def run_test():
    workingspace = r'../tests/data/tmp_results'
    dem = '../tests/data/Jamaica_dem.tif'
    log = 'taudem.log'
    runtime = 'runtime.log'
    td_names = TauDEMExtFiles(workingspace)

    TauDEM.pitremove(1, dem, td_names.filldem, workingspace, log_file=log, runtime_file=runtime)
    TauDEM.d8flowdir(4, td_names.filldem, td_names.d8flow, td_names.slp, workingspace,
                     log_file=log, runtime_file=runtime)
    TauDEM_Ext.mfdmdflowdir(4, td_names.filldem, td_names.mfdmd_dir, td_names.mfdmd_frac,
                            min_portion=0.02, workingdir=workingspace,
                            log_file=log, runtime_file=runtime)
    TauDEMWorkflow.watershed_delineation(4, dem, workingdir=workingspace)


if __name__ == "__main__":
    run_test()
