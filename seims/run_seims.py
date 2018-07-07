#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Run SEIMS model.
    @author   : Liangjun Zhu
    @changelog: 2017-12-07 - lj - Initial implementation.
                2018-07-04 - lj - Support MPI version.
                2018-07-07 - lj - Add the outputs of single model run.
"""
import os
import sys
from subprocess import CalledProcessError
from copy import deepcopy

from pygeoc.utils import UtilClass, FileClass, StringClass, sysstr

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from postprocess.load_mongodb import ReadModelData
from postprocess.utility import read_simulation_from_txt, match_simulation_observation, \
    calculate_statistics


class MainSEIMS(object):
    """Main entrance to SEIMS model.
    Args:
        bin_dir: The directory of SEIMS binary.
        model_dir: The directory of SEIMS model, the directory name MUST existed as one database name.
        nthread: Thread number for OpenMP.
        lyrmth: Layering method, can be 0 (UP_DOWN) or 1 (DOWN_UP).
        ip: MongoDB host address, default is `localhost`
        port: MongoDB port, default is 27017
        sceid: Scenario ID, which corresponding items in `<model>_Scenario` database
        caliid: Calibration ID used for model auto-calibration
        ver: SEIMS version, can be `MPI` or `OMP` (default)
        nprocess: Process number for MPI
        mpi_bin: Full path of MPI executable file, e.g., `./mpiexec` or `./mpirun`
        hosts_opt: Option for assigning hosts, e.g., `-f`, `-hostfile`, `-machine`, `-machinefile`
        hostfile: File containing host names, or file mapping process numbers to machines
    """

    def __init__(self, bin_dir='', model_dir='', nthread=4, lyrmtd=0,
                 ip='127.0.0.1', port=27017, sceid=-1, caliid=-1,
                 version='OMP', nprocess=1, mpi_bin='', hosts_opt='-f', hostfile='',
                 **kwargs):  # Allow any other keyword arguments
        #  Derived from input arguments
        args_dict = dict()
        if 'args_dict' in kwargs:  # Preferred to use 'args_dict' if existed.
            args_dict = kwargs['args_dict']
        bin_dir = args_dict['bin_dir'] if 'bin_dir' in args_dict else bin_dir
        model_dir = args_dict['model_dir'] if 'model_dir' in args_dict else model_dir
        self.version = args_dict['version'] if 'version' in args_dict else version
        suffix = '.exe' if sysstr == 'Windows' else ''
        if self.version == 'MPI':
            self.seims_exec = bin_dir + os.path.sep + 'seims_mpi' + suffix
        else:
            self.seims_exec = bin_dir + os.path.sep + 'seims_omp' + suffix
            if not FileClass.is_file_exists(self.seims_exec):  # If not support OpenMP, use `seims`!
                self.seims_exec = bin_dir + os.path.sep + 'seims' + suffix
        self.seims_exec = os.path.abspath(self.seims_exec)
        self.model_dir = os.path.abspath(model_dir)

        self.nthread = args_dict['nthread'] if 'nthread' in args_dict else nthread
        self.lyrmtd = args_dict['lyrmtd'] if 'lyrmtd' in args_dict else lyrmtd
        self.host = args_dict['ip'] if 'ip' in args_dict else ip
        self.port = args_dict['port'] if 'port' in args_dict else port
        self.scenario_id = args_dict['sceid'] if 'sceid' in args_dict else sceid
        self.calibration_id = args_dict['caliid'] if 'caliid' in args_dict else caliid
        self.nprocess = args_dict['nprocess'] if 'nprocess' in args_dict else nprocess
        self.mpi_bin = args_dict['mpi_bin'] if 'mpi_bin' in args_dict else mpi_bin
        self.hosts_opt = args_dict['hosts_opt'] if 'hosts_opt' in args_dict else hosts_opt
        self.hostfile = args_dict['hostfile'] if 'hostfile' in args_dict else hostfile

        # Concatenate executable command
        self.cmd = self.Command
        self.run_success = False
        self.output_dir = self.OutputDirectory
        # Read model data from MongoDB
        self.spatialdb_name = os.path.split(self.model_dir)[1]
        self.outlet_id = self.OutletID
        self.start_time, self.end_time = self.SimulatedPeriod
        # Data maybe used after model run
        self.timespan = dict()
        self.obs_vars = list()  # Observation types at the outlet
        self.obs_value = dict()  # Observation value, key: DATETIME, value: value list of obs_vars
        self.sim_vars = list()  # Simulation types at the outlet, which is part of obs_vars
        self.sim_value = dict()  # Simulation value, same as obs_value
        # The format of sim_obs_dict:
        #         {VarName: {'UTCDATETIME': [t1, t2, ..., tn],
        #                    'Obs': [o1, o2, ..., on],
        #                    'Sim': [s1, s2, ..., sn]},
        #         ...
        #         }
        self.sim_obs_dict = dict()

    @property
    def OutputDirectory(self):
        self.output_dir = os.path.join(self.model_dir, 'OUTPUT')
        if self.scenario_id >= 0:
            self.output_dir += str(self.scenario_id)
        if self.calibration_id >= 0:
            self.output_dir += '-%d' % self.calibration_id
        return self.output_dir

    @property
    def Command(self):
        """Concatenate command to run SEIMS."""
        self.cmd = list()
        if self.version == 'MPI':
            self.cmd += [self.mpi_bin]
            if self.hostfile != '' and self.hostfile is not None and os.path.exists(self.hostfile):
                self.cmd += [self.hosts_opt, self.hostfile]
            self.cmd += ['-n', str(self.nprocess)]
        self.cmd += [self.seims_exec,
                     '-wp', self.model_dir, '-thread', str(self.nthread),
                     '-lyr', str(self.lyrmtd), '-host', self.host, '-port', self.port]
        if self.scenario_id >= 0:
            self.cmd += ['-sce', str(self.scenario_id)]
        if self.calibration_id >= 0:
            self.cmd += ['-cali', str(self.calibration_id)]
        return self.cmd

    @property
    def OutletID(self):
        read_model = ReadModelData(self.host, self.port, self.spatialdb_name)
        return read_model.OutletID

    @property
    def SimulatedPeriod(self):
        read_model = ReadModelData(self.host, self.port, self.spatialdb_name)
        return read_model.SimulationPeriod

    def ReadOutletObservations(self, vars_list):
        read_model = ReadModelData(self.host, self.port, self.spatialdb_name)
        self.obs_vars, self.obs_value = read_model.Observation(self.outlet_id, vars_list,
                                                               self.start_time, self.end_time)
        return self.obs_vars, self.obs_value

    def SetOutletObservations(self, vars_list, vars_value):
        """Set observation data from the inputs."""
        self.obs_vars = vars_list[:]
        self.obs_value = deepcopy(vars_value)

    def ReadTimeseriesSimulations(self, stime, etime):
        if not self.obs_vars:
            return False
        self.sim_vars, self.sim_value = read_simulation_from_txt(self.output_dir,
                                                                 self.obs_vars,
                                                                 self.outlet_id,
                                                                 stime, etime)
        if len(self.sim_vars) < 1:  # No match simulation results
            return False
        return True

    def CalcTimeseriesStatistics(self):
        self.sim_obs_dict = match_simulation_observation(self.sim_vars, self.sim_value,
                                                         self.obs_vars, self.obs_value)
        objnames = calculate_statistics(self.sim_obs_dict)
        if objnames is None:
            return None, None
        comb_vars = list()
        obj_values = list()
        for var in self.sim_vars:
            for objn in objnames:
                comb_vars.append('%s-%s' % (var, objn))
                obj_values.append(self.sim_obs_dict[var][objn])
        return comb_vars, obj_values


    def ParseTimespan(self, items):
        for item in items:
            if 'TIMESPAN' not in item:
                continue
            item = item.split('\n')[0]
            values = StringClass.extract_numeric_values_from_string(item)
            if values is None or len(values) != 1:
                continue
            time = values[0]
            titles = item.replace('[', '').split(']')[:-1]
            if len(titles) < 3:
                continue
            titles = [title.strip() for title in titles]
            if titles[1] not in self.timespan:
                self.timespan[titles[1]] = dict()
            if len(titles) > 3:
                if titles[2] not in self.timespan[titles[1]]:
                    self.timespan[titles[1]][titles[2]] = dict()
                if titles[3] not in self.timespan[titles[1]][titles[2]]:
                    self.timespan[titles[1]][titles[2]][titles[3]] = list()
                self.timespan[titles[1]][titles[2]][titles[3]].append(time)
            else:
                if titles[2] not in self.timespan[titles[1]]:
                    self.timespan[titles[1]][titles[2]] = list()
                self.timespan[titles[1]][titles[2]].append(time)

    def run(self):
        """Run SEIMS model"""
        if not os.path.isdir(self.OutputDirectory) or not os.path.exists(self.OutputDirectory):
            os.makedirs(self.OutputDirectory)
        try:
            run_logs = UtilClass.run_command(self.Command)
            self.ParseTimespan(run_logs)
            self.run_success = True
        except CalledProcessError or Exception:
            print('Run SEIMS model failed!')
            self.run_success = False
        return self.run_success


def create_run_model(modelcfg_dict, sce_id=0, cali_idx=-1):
    """Create, Run, and return SEIMS model object.
    See Also:
        get_evaluate_output_name_unit
    Args:
        modelcfg_dict: Dict of arguments for SEIMS model
    Returns:
        The instance of SEIMS model.
    """
    if 'sceid' not in modelcfg_dict:
        modelcfg_dict['sceid'] = sce_id
    if 'caliid' not in modelcfg_dict:
        modelcfg_dict['caliid'] = cali_idx
    model_obj = MainSEIMS(args_dict=modelcfg_dict)
    model_obj.run()
    return model_obj


if __name__ == '__main__':
    bindir = r'D:\compile\bin\seims_mpi_omp'
    modeldir = r'D:\test\model_dianbu2_30m_demo'
    # Method 1
    # seimsobj = MainSEIMS(bindir, modeldir,
    #                      nthread=2, lyrmtd=1,
    #                      ip='127.0.0.1', port=27017,
    #                      sceid=0, caliid=-1,
    #                      version='MPI', mpi_bin='mpiexec', nprocess=2)
    # Method 2
    args = {'bin_dir': bindir, 'model_dir': modeldir,
            'nthread': 2, 'lyrmtd': 1,
            'ip': '127.0.0.1', 'port': 27017,
            'sceid': 0, 'caliid': -1,
            'version': 'MPI', 'mpi_bin': 'mpiexec', 'nprocess': 2}
    seimsobj = MainSEIMS(args_dict=args)
    seimsobj.run()

    # test the picklable of MainSEIMS class.
    import pickle

    s = pickle.dumps(seimsobj)
    new_s = pickle.loads(s)
    print(new_s.cmd)
    print(new_s.output_dir)
    print(new_s.OutletID)
    print(str(new_s.start_time), str(new_s.end_time))
