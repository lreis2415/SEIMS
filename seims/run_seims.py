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
        self.model_dir = model_dir
        self.nthread = nthread
        self.lyrmtd = lyrmtd
        self.host = ip
        self.port = port
        self.scenario_id = sceid
        self.calibration_id = caliid
        self.version = ver
        self.run_success = False
        # Concatenate command
        self.cmd = [self.seims_exec,
                    '-wp', self.model_dir, '-thread', str(self.nthread),
                    '-lyr', str(self.lyrmtd), '-host', self.host, '-port', self.port]
        self.output_dir = self.model_dir + os.sep + 'OUTPUT'
        if self.scenario_id >= 0:
            self.cmd.append('-sce')
            self.cmd.append(str(self.scenario_id))
            self.output_dir += str(self.scenario_id)
        if self.calibration_id >= 0:
            self.cmd.append('-cali')
            self.cmd.append(str(self.calibration_id))
            self.output_dir += '-%d' % self.calibration_id

    def run(self):
        """Run SEIMS model"""
        UtilClass.rmmkdir(self.output_dir)
        try:
            UtilClass.run_command(self.cmd)
            self.run_success = True
        except CalledProcessError or Exception:
            print ('Run SEIMS model failed!')
            self.run_success = False
        return self.run_success
