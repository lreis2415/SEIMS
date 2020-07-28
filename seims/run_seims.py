"""@package run_seims
Configure and run SEIMS model.

    In order to avoid thread lock problems may caused by pymongo (MongoClient),
      several functions should be called by following format:

      model.SetMongoClient()  # the global client object (global_mongoclient.py) will be used
      model.ReadOutletObservations()
      model.UnsetMongoClient()

      These functions are: `run()`, `clean()`, `ResetSimulationPeriod()`, `ResetOutputsPeriod()`,
                           `ReadMongoDBData()`, `ReadTimeseriesSimulations()`,
                           `ReadOutletObservations()`.

    @author   : Liangjun Zhu

    @changelog:
    - 2017-12-07 - lj - Initial implementation.
    - 2018-07-04 - lj - Support MPI version.
    - 2018-07-07 - lj - Add the outputs of single model run.
    - 2018-07-10 - lj - Add ParseSEIMSConfig for all SEIMS tools.
    - 2018-08-28 - lj - Add GetTimespan function and timespan counted by time.time().
    - 2018-11-15 - lj - Add model clean function.
    - 2019-01-08 - lj - Add output time period setting.
    - 2020-07-20 - lj - Read data from MongoDB once for all currently used properties.
"""
from __future__ import absolute_import, unicode_literals

import bisect
from copy import deepcopy
from collections import OrderedDict
from configparser import ConfigParser
from datetime import datetime
from io import open
import math
import os
import sys
from shutil import rmtree
import time
from typing import Optional, Union, Dict, List, AnyStr
from subprocess import CalledProcessError

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import global_mongoclient as MongoDBObj

from pygeoc.utils import UtilClass, FileClass, StringClass, sysstr, is_string, get_config_parser

from preprocess.text import DBTableNames
from preprocess.db_mongodb import MongoClient, ConnectMongoDB
from preprocess.db_read_model import ReadModelData
from utility import read_simulation_from_txt, parse_datetime_from_ini
from utility import match_simulation_observation, calculate_statistics


class ParseSEIMSConfig(object):
    """Parse SEIMS model related configurations from `ConfigParser` object."""

    def __init__(self, cf=None):
        # type: (Optional[ConfigParser]) -> None
        self.host = '127.0.0.1'  # localhost by default
        self.port = 27017
        self.bin_dir = ''
        self.model_dir = ''
        self.db_name = ''
        self.version = 'OMP'
        self.mpi_bin = None
        self.hosts_opt = None
        self.hostfile = None
        self.nprocess = 1
        self.nthread = 1
        self.lyrmtd = 1
        self.scenario_id = 0
        self.calibration_id = -1
        self.subbasin_id = 0
        self.simu_stime = None
        self.simu_etime = None
        self.out_stime = None
        self.out_etime = None
        self.config_dict = dict()
        # Running time counted by time.time() of Python, in case of GetTimespan() function failed
        self.runtime = 0.

        if cf is None:
            return

        sec_name = 'SEIMS_Model'
        if sec_name not in cf.sections():
            raise ValueError('[%s] section MUST be existed in *.ini file.' % sec_name)
        if cf.has_option(sec_name, 'hostname'):
            self.host = cf.get(sec_name, 'hostname')
        if cf.has_option(sec_name, 'port'):
            self.port = cf.getint(sec_name, 'port')
        if not StringClass.is_valid_ip_addr(self.host):
            raise ValueError('HOSTNAME defined in [%s] is illegal!' % sec_name)

        self.bin_dir = cf.get(sec_name, 'bin_dir')
        self.model_dir = cf.get(sec_name, 'model_dir')
        if not (FileClass.is_dir_exists(self.model_dir)
                and FileClass.is_dir_exists(self.bin_dir)):
            raise IOError('Please Check Directories defined in [%s]. '
                          'BIN_DIR and MODEL_DIR are required!' % sec_name)
        self.db_name = os.path.split(self.model_dir)[1]  # Defaultly, spatial dbname equals dirname
        if cf.has_option(sec_name, 'spatial_dbname'):
            self.db_name = cf.get(sec_name, 'spatial_dbname')

        if cf.has_option(sec_name, 'version'):
            self.version = cf.get(sec_name, 'version')
        if cf.has_option(sec_name, 'mpi_bin'):  # full path of the executable MPI program
            self.mpi_bin = cf.get(sec_name, 'mpi_bin')
        if cf.has_option(sec_name, 'hostopt'):
            self.hosts_opt = cf.get(sec_name, 'hostopt')
        if cf.has_option(sec_name, 'hostfile'):
            self.hostfile = cf.get(sec_name, 'hostfile')
        if cf.has_option(sec_name, 'processnum'):
            self.nprocess = cf.getint(sec_name, 'processnum')
        if cf.has_option(sec_name, 'threadsnum'):
            self.nthread = cf.getint(sec_name, 'threadsnum')
        if cf.has_option(sec_name, 'layeringmethod'):
            self.lyrmtd = cf.getint(sec_name, 'layeringmethod')
        if cf.has_option(sec_name, 'scenarioid'):
            self.scenario_id = cf.getint(sec_name, 'scenarioid')
        if cf.has_option(sec_name, 'calibrationid'):
            self.calibration_id = cf.getint(sec_name, 'calibrationid')
        if cf.has_option(sec_name, 'subbasinid'):
            self.subbasin_id = cf.getint(sec_name, 'subbasinid')

        self.simu_stime = parse_datetime_from_ini(cf, sec_name, 'sim_time_start')
        self.simu_etime = parse_datetime_from_ini(cf, sec_name, 'sim_time_end')
        if self.simu_stime and self.simu_etime and self.simu_stime >= self.simu_etime:
            raise ValueError('Wrong simulation time settings in [%s]!' % sec_name)

        self.out_stime = parse_datetime_from_ini(cf, sec_name, 'output_time_start')
        self.out_etime = parse_datetime_from_ini(cf, sec_name, 'output_time_end')
        if self.out_stime and self.out_etime and self.out_stime >= self.out_etime:
            raise ValueError('Wrong output time settings in [%s]!' % sec_name)


    @property
    def ConfigDict(self):
        # type: () -> Dict[AnyStr, Union[AnyStr, datetime, int, None]]
        if self.config_dict:
            return self.config_dict
        model_cfg_dict = {'bin_dir': self.bin_dir, 'model_dir': self.model_dir,
                          'nthread': self.nthread, 'lyrmtd': self.lyrmtd,
                          'host': self.host, 'port': self.port,
                          'db_name': self.db_name,
                          'simu_stime': self.simu_stime, 'simu_etime': self.simu_etime,
                          'out_stime': self.out_stime, 'out_etime': self.out_etime,
                          'scenario_id': self.scenario_id, 'calibration_id': self.calibration_id,
                          'subbasin_id': self.subbasin_id,
                          'version': self.version,
                          'mpi_bin': self.mpi_bin, 'nprocess': self.nprocess,
                          'hosts_opt': self.hosts_opt, 'hostfile': self.hostfile}
        return model_cfg_dict


class MainSEIMS(object):
    """Main entrance to SEIMS model."""

    def __init__(self, bin_dir='',  # type: AnyStr # The directory of SEIMS binary
                 model_dir='',  # type: AnyStr # The directory of SEIMS model
                 nthread=4,  # type: int # Thread number for OpenMP
                 lyrmtd=0,  # type: int # Layering method, can be 0 (UP_DOWN) or 1 (DOWN_UP)
                 host='127.0.0.1',  # type: AnyStr # MongoDB host address, default is `localhost`
                 port=27017,  # type: int # MongoDB port, default is 27017
                 db_name='',  # type: AnyStr  # Main spatial dbname which can diff from dirname
                 scenario_id=-1,  # type: int # Scenario ID defined in `<model>_Scenario` database
                 calibration_id=-1,  # type: int # Calibration ID used for model auto-calibration
                 subbasin_id=0,  # type: int # Subbasin ID, 0 for whole watershed, 9999 for field version
                 version='OMP',  # type: AnyStr # SEIMS version, can be `MPI` or `OMP` (default)
                 nprocess=1,  # type: int # Process number for MPI
                 mpi_bin='',  # type: AnyStr # Full path of MPI executable file, e.g., './mpirun`
                 hosts_opt='-f',  # type: AnyStr # Option for assigning hosts,
                 # e.g., `-f`, `-hostfile`, `-machine`, `-machinefile`
                 hostfile='',  # type: AnyStr # File containing host names,
                 # or file mapping process numbers to machines
                 simu_stime=None,  # type: Optional[datetime, AnyStr] # Start time of simulation
                 simu_etime=None,  # type: Optional[datetime, AnyStr] # End time of simulation
                 out_stime=None,  # type: Optional[datetime, AnyStr] # Start time of outputs
                 out_etime=None,  # type: Optional[datetime, AnyStr] # End time of outputs
                 args_dict=None  # type: Dict[AnyStr, Optional[AnyStr, datetime, int]]
                 ):
        # type: (...) -> None
        #  Derived from input arguments
        if args_dict is None:  # Preferred to use 'args_dict' if existed.
            args_dict = dict()
        bin_dir = args_dict['bin_dir'] if 'bin_dir' in args_dict else bin_dir
        model_dir = args_dict['model_dir'] if 'model_dir' in args_dict else model_dir
        self.version = args_dict['version'] if 'version' in args_dict else version
        suffix = '.exe' if sysstr == 'Windows' else ''
        if self.version == 'MPI':
            self.seims_exec = '%s/seims_mpi%s' % (bin_dir, suffix)
        else:
            self.seims_exec = '%s/seims_omp%s' % (bin_dir, suffix)
            if not FileClass.is_file_exists(self.seims_exec):  # If not support OpenMP, use `seims`!
                self.seims_exec = '%s/seims%s' % (bin_dir, suffix)
        self.seims_exec = os.path.abspath(self.seims_exec)
        self.model_dir = os.path.abspath(model_dir)

        self.nthread = args_dict['nthread'] if 'nthread' in args_dict else nthread
        self.lyrmtd = args_dict['lyrmtd'] if 'lyrmtd' in args_dict else lyrmtd
        self.host = args_dict['host'] if 'host' in args_dict else host
        self.port = args_dict['port'] if 'port' in args_dict else port
        self.db_name = args_dict['db_name'] if 'db_name' in args_dict \
            else os.path.split(self.model_dir)[1]
        self.scenario_id = args_dict['scenario_id'] if 'scenario_id' in args_dict else scenario_id
        self.calibration_id = args_dict['calibration_id'] \
            if 'calibration_id' in args_dict else calibration_id
        self.subbasin_id = args_dict['subbasin_id'] if 'subbasin_id' in args_dict else subbasin_id
        self.nprocess = args_dict['nprocess'] if 'nprocess' in args_dict else nprocess
        self.mpi_bin = args_dict['mpi_bin'] if 'mpi_bin' in args_dict else mpi_bin
        self.hosts_opt = args_dict['hosts_opt'] if 'hosts_opt' in args_dict else hosts_opt
        self.hostfile = args_dict['hostfile'] if 'hostfile' in args_dict else hostfile
        self.simu_stime = args_dict['simu_stime'] if 'simu_stime' in args_dict else simu_stime
        self.simu_etime = args_dict['simu_etime'] if 'simu_etime' in args_dict else simu_etime
        self.out_stime = args_dict['out_stime'] if 'out_stime' in args_dict else out_stime
        self.out_etime = args_dict['out_etime'] if 'out_etime' in args_dict else out_etime
        if is_string(self.simu_stime) and not isinstance(self.simu_stime, datetime):
            self.simu_stime = StringClass.get_datetime(self.simu_stime)
        if is_string(self.simu_etime) and not isinstance(self.simu_etime, datetime):
            self.simu_etime = StringClass.get_datetime(self.simu_etime)
        if is_string(self.out_stime) and not isinstance(self.out_stime, datetime):
            self.out_stime = StringClass.get_datetime(self.out_stime)
        if is_string(self.out_etime) and not isinstance(self.out_etime, datetime):
            self.out_etime = StringClass.get_datetime(self.out_etime)

        # Concatenate executable command
        self.cmd = self.Command
        self.run_success = False
        self.output_dir = self.OutputDirectory

        # Model data read from MongoDB
        self.outlet_id = -1
        self.subbasin_count = -1
        self.scenario_dbname = ''
        self.start_time = None
        self.end_time = None
        self.output_ids = list()  # type: List[AnyStr]
        self.output_items = dict()  # type: Dict[AnyStr, Union[List[AnyStr]]]

        # Data maybe used after model run
        self.timespan = dict()  # type: Dict[AnyStr, Dict[AnyStr, Union[float, Dict[AnyStr, float]]]]
        self.obs_vars = list()  # type: List[AnyStr]  # Observation types at the outlet
        self.obs_value = dict()  # type: Dict[datetime, List[float]] # Observation value
        self.sim_vars = list()  # type: List[AnyStr]  # Simulation types, part of `obs_vars`
        self.sim_value = dict()  # type: Dict[datetime, List[float]] # Simulation value
        # The format of sim_obs_dict:
        #         {VarName: {'UTCDATETIME': [t1, t2, ..., tn],
        #                    'Obs': [o1, o2, ..., on],
        #                    'Sim': [s1, s2, ..., sn]},
        #         ...
        #         }
        self.sim_obs_dict = dict()  # type: Dict[AnyStr, Dict[AnyStr, Union[float, List[Union[datetime, float]]]]]
        self.runtime = 0.
        self.runlogs = list()  # type: List[AnyStr]

        self.mongoclient = None  # type: Union[MongoClient, None]  # Set to None after use

    @property
    def OutputDirectory(self):
        # type: (...) -> AnyStr
        """The format of OUTPUT directory is: OUTPUT<ScenarioID>-<CalibrationID>
        - OUTPUT-1 means no scenario, calibration ID is 1
        - OUTPUT100 means scenario ID is 100, no calibration
        - OUTPUT100-2 means scenario ID is 100, calibration ID is 2

        Returns:
            The fullpath of output directory
        """
        self.output_dir = os.path.join(self.model_dir, 'OUTPUT')
        if self.scenario_id >= 0:
            self.output_dir += str(self.scenario_id)
        if self.calibration_id >= 0:
            self.output_dir += '-%d' % self.calibration_id
        return self.output_dir

    @property
    def Command(self):
        # type: (...) -> List[AnyStr]
        """Concatenate command to run SEIMS-based model."""
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
        if self.subbasin_id >= 0:
            self.cmd += ['-id', str(self.subbasin_id)]
        return self.cmd

    def SetMongoClient(self):
        """Should be invoked outset of this script and followed by `UnsetMongoClient`
        """
        if self.mongoclient is None:
            self.mongoclient = MongoDBObj.client

    def ConnectMongoDB(self):
        """Connect to MongoDB if no connected `MongoClient` is available

        TODO: should we add a flag to force connect to MongoDB by host and port, rather than
        TODO:   import from db_mongodb module?
        """
        if self.mongoclient is not None:
            return
        # Currently, ConnectMongoDB will terminate the program if the connection is failed
        self.mongoclient = ConnectMongoDB(self.host, self.port).get_conn()

    def UnsetMongoClient(self):
        """Should be invoked together with `SetMongoClient`
        """
        self.mongoclient = None

    def ReadMongoDBData(self):
        """
        Examples:
            model.SetMongoClient()
            model.ReadMongoDBData()
            model.UnsetMongoClient()
        """
        if self.outlet_id >= 0:
            return

        self.ConnectMongoDB()
        read_model = ReadModelData(self.mongoclient, self.db_name)

        self.outlet_id = read_model.OutletID
        self.subbasin_count = read_model.SubbasinCount
        self.scenario_dbname = read_model.ScenarioDBName
        self.start_time, self.end_time = read_model.SimulationPeriod
        self.output_ids, self.output_items = read_model.OutputItems()

    @property
    def OutletID(self):  # type: (...) -> int
        self.ReadMongoDBData()
        return self.outlet_id

    @property
    def SubbasinCount(self):  # type: (...) -> int
        self.ReadMongoDBData()
        return self.subbasin_count

    @property
    def ScenarioDBName(self):  # type: (...) -> AnyStr
        self.ReadMongoDBData()
        return self.scenario_dbname

    @property
    def SimulatedPeriod(self):  # type: (...) -> (datetime, datetime)
        self.ReadMongoDBData()
        return self.start_time, self.end_time

    @property
    def OutputIDs(self):  # type: (...) -> List[AnyStr]
        """Read output items from database."""
        self.ReadMongoDBData()
        return self.output_ids

    @property
    def OutputItems(self):  # type: (...) -> Dict[AnyStr, Optional[List[AnyStr]]]
        """Read output items from database."""
        self.ReadMongoDBData()
        return self.output_items

    def ReadOutletObservations(self, vars_list):
        # type: (List[AnyStr]) -> (List[AnyStr], Dict[datetime, List[float]])
        """

        Examples:
            model.SetMongoClient()
            model.ReadOutletObservations()
            model.UnsetMongoClient()
        """
        self.ConnectMongoDB()
        self.ReadMongoDBData()
        read_model = ReadModelData(self.mongoclient, self.db_name)
        self.obs_vars, self.obs_value = read_model.Observation(self.outlet_id, vars_list,
                                                               self.start_time, self.end_time)
        return self.obs_vars, self.obs_value

    def SetOutletObservations(self, vars_list, vars_value):
        # type: (List[AnyStr], Dict[datetime, List[float]]) -> None
        """Set observation data from the inputs."""
        self.obs_vars = vars_list[:]
        self.obs_value = deepcopy(vars_value)

    def ReadTimeseriesSimulations(self, stime=None, etime=None):
        # type: (Optional[datetime], Optional[datetime]) -> bool
        """Read time series simulation results from OUTPUT directory.
        If no start time or end time are specified, the time ranges from `FILE_IN` will be used.
        """
        if not self.obs_vars:
            return False
        startt, endt = self.SimulatedPeriod
        if stime is None:
            stime = startt
        if etime is None:
            etime = endt
        self.sim_vars, self.sim_value = read_simulation_from_txt(self.OutputDirectory,
                                                                 self.obs_vars,
                                                                 self.OutletID,
                                                                 stime, etime)
        if len(self.sim_vars) < 1:  # No match simulation results
            return False
        self.sim_obs_dict = match_simulation_observation(self.sim_vars, self.sim_value,
                                                         self.obs_vars, self.obs_value)
        return True

    def ExtractSimData(self, stime=None, etime=None):
        # type: (Optional[datetime], Optional[datetime]) -> (List[AnyStr], Dict[datetime, List[float]])
        if stime is None and etime is None:
            return self.sim_vars, self.sim_value

        sidx = bisect.bisect_left(list(self.sim_value.keys()), stime)
        eidx = bisect.bisect_right(list(self.sim_value.keys()), etime)

        def slice_odict(odict, start=None, end=None):
            return OrderedDict([(k, v) for (k, v) in odict.items()
                                if k in list(odict.keys())[start:end]])

        return self.sim_vars, slice_odict(self.sim_value, sidx, eidx)

    def ExtractSimObsData(self, stime=None, etime=None):
        # type: (Optional[datetime], Optional[datetime]) -> Dict[AnyStr, Dict[AnyStr, List[Union[datetime, float]]]]
        if stime is None and etime is None:
            return deepcopy(self.sim_obs_dict)
        ext_dict = dict()
        for param, values in self.sim_obs_dict.items():
            ext_dict[param] = dict()
            sidx = bisect.bisect_left(values['UTCDATETIME'], stime)
            eidx = bisect.bisect_right(values['UTCDATETIME'], etime)
            ext_dict[param]['UTCDATETIME'] = values['UTCDATETIME'][sidx:eidx]
            ext_dict[param]['Obs'] = values['Obs'][sidx:eidx]
            ext_dict[param]['Sim'] = values['Sim'][sidx:eidx]
        return ext_dict

    @staticmethod
    def CalcTimeseriesStatistics(sim_obs_dict,
                                 # type: Dict[AnyStr, Dict[AnyStr, Union[float, List[Union[datetime, float]]]]]
                                 stime=None,  # type: Optional[datetime]
                                 etime=None  # type: Optional[datetime]
                                 ):
        # type: (...) -> (List[AnyStr], List[float])
        objnames = calculate_statistics(sim_obs_dict, stime, etime)
        if objnames is None:
            return None, None
        comb_vars = list()
        obj_values = list()
        for var in sim_obs_dict.keys():
            for objn in objnames:
                comb_vars.append('%s-%s' % (var, objn))
                objv = sim_obs_dict[var][objn]
                if objn.upper() == 'PBIAS':
                    objv = math.fabs(objv)
                obj_values.append(objv)
        return comb_vars, obj_values

    def GetTimespan(self):
        # type: (...) -> List[float]
        """Get summarized timespan, format is [IO, COMP, SIMU, RUNTIME]."""
        time_list = [0., 0., 0., self.runtime]
        # Do not use self.run_success to check if the timespan data is available or not! By lj.
        # if not self.run_success:
        #     time_list[2] = self.runtime
        #     return time_list
        if not self.timespan:
            time_list[2] = self.runtime
            return time_list
        tmp_timespan = self.timespan
        if 'MAX' in self.timespan:
            tmp_timespan = self.timespan['MAX']
        if 'IO' in tmp_timespan:
            if 'ALL' in tmp_timespan['IO']:
                time_list[0] = tmp_timespan['IO']['ALL']
            else:
                for k, v in tmp_timespan['IO'].items():
                    time_list[0] += v
        if 'COMP' in tmp_timespan:
            if 'ALL' in tmp_timespan['COMP']:
                time_list[1] = tmp_timespan['COMP']['ALL']
            else:
                for k, v in tmp_timespan['COMP'].items():
                    time_list[1] += v
        if 'SIMU' in tmp_timespan:
            if 'ALL' in tmp_timespan['SIMU']:
                time_list[2] = tmp_timespan['SIMU']['ALL']
        if time_list[2] == 0.:
            time_list[2] = self.runtime
        return time_list

    def ParseTimespan(self, items):
        # type: (List[AnyStr]) -> Dict[AnyStr, Dict[AnyStr, Union[float, Dict[AnyStr, float]]]]
        """The format of self.timespan is different for OpenMP version and MPI&OpenMP version.
        For OpenMP version:
           {'IO': {'Input': 0.2,
                   'Output': 0.04
                  }
            'COMP': {'TSD_RD_P': 0.0001,  # All modules
                     'ALL': 12.3
                    }
            'SIMU': {'ALL': 14.1}
           }
        For MPI&OpenMP version:
           {'MAX': {'IO': {'Input': 0.1,
                           'Output': 0.02,
                           'ALL': 0.12
                          }
                    'COMP': {'Slope': 5,
                             'Channel': 0.5,
                             'Barrier': 0.1,
                             'ALL': 5.6
                            }
                    'SIMU': {'ALL': 10.1}
                   }
            'MIN': {...}
            'AVG': {...}
           }
        """
        for item in items:
            if 'TIMESPAN' not in item:
                continue
            item = item.split('\n')[0]
            values = StringClass.extract_numeric_values_from_string(item)
            if values is None or len(values) != 1:
                continue
            time = values[0]
            titles = item.replace('[', '').split(']')[:-1]  # e.g., 'TIMESPAN', 'COMP', 'ALL'
            if len(titles) < 3:  # e.g., 'TIMESPAN', 'MAX', 'COMP', 'ALL'
                continue
            titles = [title.strip() for title in titles]
            self.timespan.setdefault(titles[1], dict())
            if len(titles) > 3:
                self.timespan[titles[1]].setdefault(titles[2], dict())
                self.timespan[titles[1]][titles[2]].setdefault(titles[3], time)
            else:
                self.timespan[titles[1]].setdefault(titles[2], time)
            return self.timespan

    def ResetSimulationPeriod(self):
        """Update simulation time range in MongoDB [FILE_IN].

        Examples:
            model.SetMongoClient()
            model.ResetSimulationPeriod()
            model.UnsetMongoClient()
        """
        self.ConnectMongoDB()
        read_model = ReadModelData(self.mongoclient, self.db_name)
        if self.simu_stime and self.simu_etime:
            stime_str = self.simu_stime.strftime('%Y-%m-%d %H:%M:%S')
            etime_str = self.simu_etime.strftime('%Y-%m-%d %H:%M:%S')
            db = read_model.maindb
            db[DBTableNames.main_filein].find_one_and_update({'TAG': 'STARTTIME'},
                                                             {'$set': {'VALUE': stime_str}})
            db[DBTableNames.main_filein].find_one_and_update({'TAG': 'ENDTIME'},
                                                             {'$set': {'VALUE': etime_str}})
        self.start_time, self.end_time = read_model.SimulationPeriod

    def ResetOutputsPeriod(self, output_ids,  # type: Union[AnyStr, List[AnyStr]]
                           stime,  # type: Union[datetime, List[datetime]]
                           etime  # type: Union[datetime, List[datetime]]
                           ):
        # type: (...) -> None
        """Reset the STARTTIME and ENDTIME of OUTPUTID(s).

        Examples:
            model.SetMongoClient()
            model.clean()
            model.UnsetMongoClient()
        """
        if not isinstance(output_ids, list):
            output_ids = [output_ids]
        if not isinstance(stime, list):
            stime = [stime]
        if not isinstance(etime, list):
            etime = [etime]
        self.ConnectMongoDB()
        read_model = ReadModelData(self.mongoclient, self.db_name)
        for idx, outputid in enumerate(output_ids):
            cur_stime = stime[0]
            if idx < len(stime):
                cur_stime = stime[idx]
            cur_etime = etime[0]
            if idx < len(etime):
                cur_etime = etime[idx]
            cur_stime_str = cur_stime.strftime('%Y-%m-%d %H:%M:%S')
            cur_etime_str = cur_etime.strftime('%Y-%m-%d %H:%M:%S')
            db = read_model.maindb
            db[DBTableNames.main_fileout].find_one_and_update({'OUTPUTID': outputid},
                                                              {'$set': {'STARTTIME': cur_stime_str,
                                                                        'ENDTIME': cur_etime_str}})

    def run(self):
        """Run SEIMS model

        Examples:
            model.SetMongoClient()
            model.run()
            model.UnsetMongoClient()
        """
        stime = time.time()
        if not os.path.isdir(self.OutputDirectory) or not os.path.exists(self.OutputDirectory):
            os.makedirs(self.OutputDirectory)
        # If the input time period is not consistent with the predefined time period in FILE_IN.
        if self.simu_stime and self.simu_etime and self.simu_stime != self.start_time \
            and self.simu_etime != self.end_time:
            self.ResetSimulationPeriod()
        # If the output time period is specified, reset the time period of all output IDs
        if self.out_stime and self.out_etime:
            self.ResetOutputsPeriod(self.OutputIDs, self.out_stime, self.out_etime)
        try:
            self.runlogs = UtilClass.run_command(self.Command)
            with open(self.OutputDirectory + os.sep + 'runlogs.txt', 'w', encoding='utf-8') as f:
                f.write('\n'.join(self.runlogs))
            self.ParseTimespan(self.runlogs)
            self.run_success = True
        except CalledProcessError or IOError or Exception as err:
            # 1. SEIMS-based model running failed
            # 2. The OUTPUT directory was not been created successfully by SEIMS-based model
            # 3. Other unpredictable errors
            print('Run SEIMS model failed! %s' % str(err))
            self.run_success = False
        self.runtime = time.time() - stime
        return self.run_success

    def clean(self, scenario_id=None, calibration_id=None, delete_scenario=False,
              delete_spatial_gfs=False):
        """Clean model outputs in OUTPUT<ScenarioID>-<CalibrationID> directory and/or
        GridFS files in OUTPUT collection.

        Examples:
            model.SetMongoClient()
            model.clean()
            model.UnsetMongoClient()
        """
        rmtree(self.OutputDirectory, ignore_errors=True)
        self.ConnectMongoDB()
        read_model = ReadModelData(self.mongoclient, self.db_name)
        if scenario_id is None:
            scenario_id = self.scenario_id
        if calibration_id is None:
            calibration_id = self.calibration_id
        read_model.CleanOutputGridFs(scenario_id, calibration_id)
        if delete_scenario:
            read_model.CleanScenariosConfiguration(scenario_id)
            if delete_spatial_gfs:
                read_model.CleanSpatialGridFs(scenario_id)


def create_run_model(modelcfg_dict, scenario_id=0, calibration_id=-1, subbasin_id=0):
    """Create, Run, and return SEIMS-based watershed model object.

    Args:
        modelcfg_dict: Dict of arguments for SEIMS-based watershed model
        scenario_id: Scenario ID which can override the scenario_id in modelcfg_dict
        calibration_id: Calibration ID which can override the calibration_id in modelcfg_dict
        subbasin_id: Subbasin ID (0 for the whole watershed, 9999 for the field version) which
                     can override the subbasin_id in modelcfg_dict
    Returns:
        The instance of SEIMS-based watershed model.
    """
    if 'scenario_id' not in modelcfg_dict:
        modelcfg_dict['scenario_id'] = scenario_id
    if 'calibration_id' not in modelcfg_dict:
        modelcfg_dict['calibration_id'] = calibration_id
    if 'subbasin_id' not in modelcfg_dict:
        modelcfg_dict['subbasin_id'] = subbasin_id
    model_obj = MainSEIMS(args_dict=modelcfg_dict)

    model_obj.SetMongoClient()
    model_obj.run()
    model_obj.UnsetMongoClient()

    return model_obj


def main():
    """Run SEIMS-based watershed model with configuration file."""
    cf = get_config_parser()
    runmodel_cfg = ParseSEIMSConfig(cf)
    seims_obj = MainSEIMS(args_dict=runmodel_cfg.ConfigDict)
    seims_obj.run()

    for log in seims_obj.runlogs:
        print(log)


if __name__ == '__main__':
    main()
