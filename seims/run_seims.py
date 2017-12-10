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
                 ip='127.0.0.1', port=27017, sceid=0, ver='OMP'):
        if ver == 'MPI':
            self.seims_exec = bin_dir + os.sep + 'seims_mpi'
        else:
            self.seims_exec = bin_dir + os.sep + 'seims_omp'
        self.model_dir = model_dir
        self.nthread = nthread
        self.lyrmtd = lyrmtd
        self.host = ip
        self.port = port
        self.scenario = sceid
        self.version = ver
        self.run_success = False

    def run(self):
        cmd_str = '%s %s %d %d %s %d %d' % (self.seims_exec, self.model_dir, self.nthread,
                                            self.lyrmtd, self.host, self.port, self.scenario)
        try:
            UtilClass.run_command(cmd_str)
            self.run_success = True
        except CalledProcessError or Exception:
            print ('Run SEIMS model failed!')
            self.run_success = False
        return self.run_success
