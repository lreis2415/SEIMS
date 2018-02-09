#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
  Run SEIMS model.
  Author: Liangjun Zhu
  Date: 2017-12-7
"""
import os
from subprocess import CalledProcessError

from pygeoc.utils import UtilClass

from postprocess.load_mongodb import ReadModelData


class MainSEIMS(object):
    """
    Main entrance to SEIMS model.
    """

    def __init__(self, bin_dir, model_dir, nthread=4, lyrmtd=0,
                 ip='127.0.0.1', port=27017, sceid=-1, caliid=-1,
                 ver='OMP'):
        if ver == 'MPI':  # TODO, MPI version is currently unusable.
            self.seims_exec = bin_dir + os.sep + 'seims_mpi'
        else:
            self.seims_exec = bin_dir + os.sep + 'seims_omp'
        self.model_dir = os.path.abspath(model_dir)
        self.nthread = nthread
        self.lyrmtd = lyrmtd
        self.host = ip
        self.port = port
        self.scenario_id = sceid
        self.calibration_id = caliid
        self.version = ver
        self.run_success = False
        self.cmd = self.Command
        self.output_dir = self.OutputDirectory
        # read model data from MongoDB
        self.spatialdb_name = self.model_dir.split(os.sep)[-1]
        self.outlet_id = self.OutletID
        self.start_time, self.end_time = self.SimulatedPeriod

    @property
    def OutputDirectory(self):
        self.output_dir = self.model_dir + os.sep + 'OUTPUT'
        if self.scenario_id >= 0:
            self.output_dir += str(self.scenario_id)
        if self.calibration_id >= 0:
            self.output_dir += '-%d' % self.calibration_id
        return self.output_dir

    @property
    def Command(self):
        """Concatenate command to run SEIMS."""
        self.cmd = [self.seims_exec,
                    '-wp', self.model_dir, '-thread', str(self.nthread),
                    '-lyr', str(self.lyrmtd), '-host', self.host, '-port', self.port]
        if self.scenario_id >= 0:
            self.cmd.append('-sce')
            self.cmd.append(str(self.scenario_id))
        if self.calibration_id >= 0:
            self.cmd.append('-cali')
            self.cmd.append(str(self.calibration_id))
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

    bindir = r'D:\compile\bin\seims'
    modeldir = r'C:\z_data\ChangTing\seims_models_phd\youwuzhen10m_longterm_model'
    seimsobj = MainSEIMS(bindir, modeldir,
                         nthread=2, lyrmtd=1,
                         ip='127.0.0.1', port=27017,
                         sceid=0, caliid=-1)
    s = pickle.dumps(seimsobj)
    # print(s)
    new_s = pickle.loads(s)
    print(new_s.output_dir)
    print(new_s.OutletID)
    print(str(new_s.start_time), str(new_s.end_time))
