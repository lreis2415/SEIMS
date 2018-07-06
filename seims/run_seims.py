#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Run SEIMS model.
    @author   : Liangjun Zhu
    @changelog: 2017-12-07 - lj - Initial implementation.
                2018-07-04 - lj - Support MPI version.
"""
import os
import sys
from subprocess import CalledProcessError

from pygeoc.utils import UtilClass, FileClass

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from postprocess.load_mongodb import ReadModelData


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

    def __init__(self, bin_dir, model_dir, nthread=4, lyrmtd=0,
                 ip='127.0.0.1', port=27017, sceid=-1, caliid=-1,
                 ver='OMP', nprocess=1, mpi_bin='', hosts_opt='-f', hostfile=''):
        if ver == 'MPI':
            self.seims_exec = bin_dir + os.path.sep + 'seims_mpi'
        else:
            self.seims_exec = bin_dir + os.path.sep + 'seims_omp'
            if not FileClass.is_file_exists(self.seims_exec):
                self.seims_exec = bin_dir + os.path.sep + 'seims'  # For compiler not support OpenMP
        self.seims_exec = os.path.abspath(self.seims_exec)
        self.model_dir = os.path.abspath(model_dir)
        self.nthread = nthread
        self.lyrmtd = lyrmtd
        self.host = ip
        self.port = port
        self.scenario_id = sceid
        self.calibration_id = caliid
        self.version = ver
        self.nprocess = nprocess
        self.mpi_bin = mpi_bin
        self.hosts_opt = hosts_opt
        self.hostfile = hostfile
        self.run_success = False
        # Concatenate executable command
        self.cmd = self.Command
        self.output_dir = self.OutputDirectory
        # read model data from MongoDB
        self.spatialdb_name = os.path.split(self.model_dir)[1]
        self.outlet_id = self.OutletID
        self.start_time, self.end_time = self.SimulatedPeriod

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
        return read_model.Observation(self.outlet_id, vars_list,
                                      self.start_time, self.end_time)

    def run(self):
        """Run SEIMS model"""
        UtilClass.rmmkdir(self.OutputDirectory)
        try:
            UtilClass.run_command(self.Command)
            self.run_success = True
        except CalledProcessError or Exception:
            print('Run SEIMS model failed!')
            self.run_success = False
        return self.run_success


if __name__ == '__main__':
    # test the picklable of MainSEIMS class.
    import pickle

    bindir = r'D:\compile\bin\seims_mpi_omp'
    modeldir = r'D:\test\model_dianbu2_30m_demo'
    seimsobj = MainSEIMS(bindir, modeldir,
                         nthread=2, lyrmtd=1,
                         ip='127.0.0.1', port=27017,
                         sceid=0, caliid=-1,
                         ver='MPI', mpi_bin='mpiexec', nprocess=2)
    s = pickle.dumps(seimsobj)
    # print(s)
    new_s = pickle.loads(s)
    print(new_s.cmd)
    print(new_s.output_dir)
    print(new_s.OutletID)
    print(str(new_s.start_time), str(new_s.end_time))
