"""Configuration of pyAutoFuzSlpPos project.

    @author: Liangjun Zhu

    @changelog:
    - 15-07-31  lj - initial implementation.
    - 17-07-31  lj - reorganize as basic class, and incorporated with pygeoc.
"""
from __future__ import absolute_import, unicode_literals, division

import argparse
import os
import sys
from multiprocessing import cpu_count
from configparser import ConfigParser

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import FileClass, StringClass

from autofuzslppos.Nomenclature import CreateWorkspace, PreProcessAttrNames, TopoAttrNames, \
    LogNames, FuzSlpPosFiles, SingleSlpPosFiles


class AutoFuzSlpPosConfig(object):
    """Get input arguments for pyAutoFuzSlpPos main program and
       parse configuration file (\*.ini file).

    Attributes:
        bin_dir: Required. Executable binary file path.
        ws: Derived from inputs. Workspace directories, see also CreateWorkspace.
        dem: Required. Input dem of study area. Be caution! DEM file should have one cell buffer
             of the desired extent, e.g., watershed boundary.
             If flag_PreProcess is set to False, dem can be None.
        mpi_dir: Optional. MPI binary path. If it has been exported to the environmental path,
                 set as None.
        hostfile: Optional. The hostfile is a text file that contains the names of hosts,
                  the number of available slots on each host, set to None if not stated.
        outlet: Optional. Watershed outlet as ESRI shapefile. Be caution! The outlet point should
                locate at least one cell inner the DEM boundary. If outlet is None, the maximum
                of Contributing Area will be identified as outlet.
        valley: Optional. Vally source as raster file. If not provided, set None.
        ridge: Optional. Ridge source as raster file. If not provided, set None.
        regional_attr: Regional topographic attributes, decrease from ridge to valley, range from
                       1 to 0, e.g., RPI (Relative Position Index, Skidmore, 1990)
        flag_preprocess: Preprocess for terrain attributes? True is default, if false, topographic
                         attributes used for fuzzy inference must be existed in 'Params' dir.
        flag_selecttyploc: Select typical locations automatically? True is default, if false,
                           typical locations of each slope position must be existed.
        flag_auto_typlocparams: Automatically determine the parameters for typical locations?
                                True is default, if false, the script will find these parameters
                                from the \*.ini configuration file, and/or the XXXExtConfig.dat file
                                in 'Config' directory.
                                Exception will be raised if all tries failed.
        flag_fuzzyinference: Calculate fuzzy membership of each slope position? True is default.
        flag_auto_inferenceparams: Automatically determine the parameters for fuzzy inference?
                                   True is default, if false, the script will find these parameters
                                   from the \*.ini configuration file, and/or the XXXInfConfig.dat
                                   in 'Config' directory.
                                   Exception will be raised if all tries failed.
        flag_log: Write runtime log information to files. True is default.
        selectedtopo: Topographic attributes used for AutoFuzSlpPos. The key is attribute name,
                      and value is full file path. See also topoparam.
        extractrange: Extract value ranges for each topographic attributes of each slope positions.
                      {SlpPosType: {regionalAttr: [minv, maxv], ...}}
        inferparam: A Directory to store fuzzy inference parameters, the basic format is:
                    {SlpPosType: {regionalAttr: [FMFShape, w1, r1, k1, w2, r2, k2], ...}}
    See Also:
        ``Nomenclature.CreateWorkspace``

    """

    # B(1)      6	2	0.5	6	2	0.5
    # S(2)      6	2	0.5	1	0	1
    # Z(3)      1	0	1	6	2	0.5
    _FMFTYPE = {1: 'B', 2: 'S', 3: 'Z'}
    _FMFPARAM = {'B': [6, 2, 0.5, 6, 2, 0.5],
                 'S': [6, 2, 0.5, 1, 0, 1],
                 'Z': [1, 0, 1, 6, 2, 0.5]}

    def __init__(self, cfg_parser, bin_dir=None, proc_num=-1, rawdem=None, root_dir=None):
        """
        Initialize an AutoFuzSlpPosConfig object
        Args:
            cfg_parser: ConfigParser object
            bin_dir: Executable binaries path
            proc_num: thread (or process) number used for MPI
            rawdem: DEM of study area
            root_dir: workspace path
        """
        # Part I Initialize attributes
        # 1.1. input parameters
        self.cf = cfg_parser
        self.bin_dir = bin_dir
        self.proc = proc_num
        self.root_dir = root_dir

        # 1.2. Required inputs
        self.dem = rawdem

        # 1.3. Executable Flags (Set default flags first)
        self.flag_preprocess = True
        self.flag_selecttyploc = True
        self.flag_auto_typlocparams = True
        self.flag_fuzzyinference = True
        self.flag_auto_inferenceparams = True
        self.flag_log = True
        # 1.4. Optional inputs
        self.mpi_dir = None
        self.hostfile = None
        self.outlet = None
        self.valley = None
        self.ridge = None
        self.regional_attr = None

        # 1.5. Optional DTA related parameters
        self.flow_model = 1
        self.rpi_method = 1
        self.dist_exp = 8
        self.max_move_dist = 50
        self.numthresh = 20
        self.d8_stream_thresh = 0
        self.d8_down_method = 'Surface'
        self.d8_stream_tag = 1
        self.d8_up_method = 'Surface'
        self.dinf_stream_thresh = 0
        self.dinf_down_stat = 'Average'
        self.dinf_down_method = 'Surface'
        self.dinf_dist_down_wg = None
        self.propthresh = 0.0
        self.dinf_up_stat = 'Average'
        self.dinf_up_method = 'Surface'

        # 1.6. Slope position types, tags, typical location extract value ranges, and inference
        #      parameters.
        self.slppostype = list()  # From top to bottom on hillslope.
        self.slppostag = list()  # The same sequence with self.slppostype
        self.selectedtopo = dict()  # Topographic attributes used for AutoFuzSlpPos
        self.extractrange = dict()
        self.param4typloc = dict()
        self.infshape = dict()
        self.inferparam = dict()

        # 1.7. derived attributes
        self.ws = None
        self.log = None
        self.topoparam = None
        self.slpposresult = None
        self.pretaudem = None
        self.singleslpposconf = dict()

        # Part II Set default settings, if cfg_parser is None, the program still can be executed.

        # default slope position settings
        self.slppostype = ['rdg', 'shd', 'bks', 'fts', 'vly']
        self.slppostag = [1, 2, 4, 8, 16]
        self.selectedtopolist = ['rpi', 'profc', 'slp', 'elev']
        self.selectedtopo = dict()
        self.extractrange = {'rdg': {'rpi': [0.99, 1.0]},
                             'shd': {'rpi': [0.9, 0.95]},
                             'bks': {'rpi': [0.5, 0.6]},
                             'fts': {'rpi': [0.15, 0.2]},
                             'vly': {'rpi': [0., 0.1]}}

        self._DEFAULT_PARAM_TYPLOC = [10, 0.1, 0.3, 1, 0.1, 1, 50, 4.0]
        for slppos in self.slppostype:
            self.param4typloc[slppos] = self._DEFAULT_PARAM_TYPLOC[:]

        self.infshape = {'rdg': {'rpi': 'S', 'profc': 'S', 'slp': 'Z', 'elev': 'SN'},
                         'shd': {'rpi': 'B', 'profc': 'S', 'slp': 'B', 'elev': 'N'},
                         'bks': {'rpi': 'B', 'profc': 'B', 'slp': 'S', 'elev': 'N'},
                         'fts': {'rpi': 'B', 'profc': 'ZB', 'slp': 'ZB', 'elev': 'N'},
                         'vly': {'rpi': 'Z', 'profc': 'Z', 'slp': 'Z', 'elev': 'N'}}
        if self.cf is None:
            if self.dem is not None and self.bin_dir is not None:
                if self.root_dir is None:
                    self.root_dir = os.path.dirname(self.dem)
                if self.mpi_dir is None:
                    mpipath = FileClass.get_executable_fullpath('mpiexec')
                    self.mpi_dir = os.path.dirname(mpipath)
                if self.mpi_dir is None:
                    raise RuntimeError('Can not find mpiexec, make sure you have it installed!')
            else:
                raise RuntimeError("You MUST select one of ini file or dem and bin!")
        if self.root_dir is not None:
            self.ws = CreateWorkspace(self.root_dir)
            self.log = LogNames(self.ws.log_dir)
            self.topoparam = TopoAttrNames(self.ws)
            self.slpposresult = FuzSlpPosFiles(self.ws)
            self.pretaudem = PreProcessAttrNames(self.ws.pre_dir, self.flow_model)
            for attr in self.selectedtopolist:
                self.selectedtopo[attr] = self.topoparam.get_attr_file(attr)
            for slppos in self.slppostype:
                self.singleslpposconf[slppos] = SingleSlpPosFiles(self.ws, slppos)

        # Part III Parse the *.ini configuration file if existed
        #          Be careful, bin_dir, root_dir, proc_num, and rawdem related attributes
        #          that have already set MUST not be changed in the following procedures.
        if self.cf is not None:
            # Parse and check validation of all available inputs
            # define the section names in the *.ini configuration file
            _require = 'REQUIRED'
            _flag = 'EXECUTABLE_FLAGS'
            _optdta = 'OPTIONAL_DTA'
            _opt = 'OPTIONAL'
            _opttyploc = 'OPTIONAL_TYPLOC'
            _optfuzinf = 'OPTIONAL_FUZINF'
            self.read_required_section(_require)
            self.read_flag_section(_flag)
            self.read_optionaldta_section(_optdta)
            self.read_optional_section(_opt)
            self.read_optiontyploc_section(_opttyploc)
            self.read_optionfuzinf_section(_optfuzinf)

    @staticmethod
    def check_file_available(in_f):
        """Check the input file is existed or not, and return None, if not."""
        if StringClass.string_match(in_f, 'none') or in_f == '' or in_f is None:
            return None
        if not FileClass.is_file_exists(in_f):
            raise ValueError("The %s is not existed or have no access permission!" % in_f)
        else:
            return in_f

    def read_required_section(self, _require):
        """read and check required section"""
        if _require in self.cf.sections():
            if self.bin_dir is None:
                self.bin_dir = self.cf.get(_require, 'exedir')
            if self.root_dir is None and self.cf.has_option(_require, 'rootdir'):
                self.root_dir = self.cf.get(_require, 'rootdir')
            if self.root_dir is None:
                raise IOError("Workspace must be defined!")
            self.ws = CreateWorkspace(self.root_dir)
            self.log = LogNames(self.ws.log_dir)
            self.topoparam = TopoAttrNames(self.ws)
            self.slpposresult = FuzSlpPosFiles(self.ws)
            if self.dem is None:
                self.dem = self.cf.get(_require, 'rawdem')
                self.dem = AutoFuzSlpPosConfig.check_file_available(self.dem)
            if self.dem is None:
                raise ValueError("DEM can not be None!")
        else:
            raise ValueError("[REQUIRED] section MUST be existed in *.ini file.")

        if not os.path.isdir(self.bin_dir):
            self.bin_dir = None
            if FileClass.get_executable_fullpath('fuzzyslpposinference') is None:
                raise RuntimeError("exeDir is set to None, however the executable path are not "
                                   "set in environment path!")

    def read_flag_section(self, _flag):
        """read executable flags"""
        if _flag in self.cf.sections():
            self.flag_preprocess = self.cf.getboolean(_flag, 'preprocess')
            self.flag_selecttyploc = self.cf.getboolean(_flag, 'typlocselection')
            self.flag_auto_typlocparams = self.cf.getboolean(_flag, 'autotyplocparams')
            self.flag_fuzzyinference = self.cf.getboolean(_flag, 'fuzzyinference')
            self.flag_auto_inferenceparams = self.cf.getboolean(_flag, 'autoinfparams')
            self.flag_log = self.cf.getboolean(_flag, 'extlog')

    def read_optionaldta_section(self, _optdta):
        """Optional parameters settings of digital terrain analysis for topographic attributes"""
        if _optdta not in self.cf.sections():
            return
        self.flow_model = self.cf.getint(_optdta, 'flowmodel')
        self.rpi_method = self.cf.getint(_optdta, 'rpimethod')
        self.dist_exp = self.cf.getint(_optdta, 'distanceexponentforidw')
        self.max_move_dist = self.cf.getfloat(_optdta, 'maxmovedist')
        self.numthresh = self.cf.getint(_optdta, 'numthresh')
        self.d8_stream_thresh = self.cf.getint(_optdta, 'd8streamthreshold')
        self.d8_down_method = self.cf.get(_optdta, 'd8downmethod')
        self.d8_stream_tag = self.cf.getint(_optdta, 'd8streamtag')
        self.d8_up_method = self.cf.get(_optdta, 'd8upmethod')
        self.dinf_stream_thresh = self.cf.getint(_optdta, 'dinfstreamthreshold')
        self.dinf_down_stat = self.cf.get(_optdta, 'dinfdownstat')
        self.dinf_down_method = self.cf.get(_optdta, 'dinfdownmethod')
        self.dinf_dist_down_wg = self.cf.get(_optdta, 'dinfdistdownwg')
        self.propthresh = self.cf.getfloat(_optdta, 'propthresh')
        self.dinf_up_stat = self.cf.get(_optdta, 'dinfupstat')
        self.dinf_up_method = self.cf.get(_optdta, 'dinfupmethod')
        if self.flow_model != 0:
            self.flow_model = 1
        if self.rpi_method != 0:
            self.rpi_method = 1
        if self.dist_exp < 0:
            self.dist_exp = 8
        if self.max_move_dist < 0:
            self.max_move_dist = 50
        if self.numthresh < 0:
            self.numthresh = 20
        if self.d8_stream_thresh < 0:
            self.d8_stream_thresh = 0
        distance_method = ['Horizontal', 'Vertical', 'Pythagoras', 'Surface']
        stat_method = ['Average', 'Maximum', 'Minimum']
        if not StringClass.string_in_list(self.d8_down_method, distance_method):
            self.d8_down_method = 'Surface'
        if self.d8_stream_tag < 0:
            self.d8_stream_tag = 1
        if not StringClass.string_in_list(self.d8_up_method, distance_method):
            self.d8_up_method = 'Surface'
        if self.dinf_stream_thresh < 0:
            self.dinf_stream_thresh = 0
        if StringClass.string_in_list(self.dinf_down_stat, stat_method):
            self.dinf_down_stat = 'Average'
        if StringClass.string_in_list(self.dinf_down_method, distance_method):
            self.dinf_down_method = 'Surface'
        self.dinf_dist_down_wg = AutoFuzSlpPosConfig.check_file_available(self.dinf_dist_down_wg)
        if self.propthresh < 0:
            self.propthresh = 0.0
        if not StringClass.string_in_list(self.dinf_up_stat, stat_method):
            self.dinf_up_stat = 'Average'
        if not StringClass.string_in_list(self.dinf_up_method, distance_method):
            self.dinf_up_method = 'Surface'
        self.pretaudem = PreProcessAttrNames(self.ws.pre_dir, self.flow_model)

    def read_optional_section(self, _opt):
        """read and check OPTIONAL inputs."""
        if _opt not in self.cf.sections():
            return
        self.mpi_dir = self.cf.get(_opt, 'mpiexedir')
        self.hostfile = self.cf.get(_opt, 'hostfile')
        self.outlet = self.cf.get(_opt, 'outlet')
        self.valley = self.cf.get(_opt, 'vlysrc')
        self.ridge = self.cf.get(_opt, 'rdgsrc')
        self.regional_attr = self.cf.get(_opt, 'regionalattr')
        if self.proc <= 0 or self.proc is None:
            if self.cf.has_option(_opt, 'inputproc'):
                self.proc = self.cf.getint(_opt, 'inputproc')
            else:
                self.proc = cpu_count() / 2
        # if mpi directory is not set
        if self.mpi_dir is None or StringClass.string_match(self.mpi_dir, 'none') \
            or not os.path.isdir(self.mpi_dir):
            mpipath = FileClass.get_executable_fullpath('mpiexec')
            self.mpi_dir = os.path.dirname(mpipath)
        if self.mpi_dir is None:
            raise RuntimeError('Can not find mpiexec!')
        self.hostfile = AutoFuzSlpPosConfig.check_file_available(self.hostfile)
        self.outlet = AutoFuzSlpPosConfig.check_file_available(self.outlet)
        self.valley = AutoFuzSlpPosConfig.check_file_available(self.valley)
        self.ridge = AutoFuzSlpPosConfig.check_file_available(self.ridge)
        self.regional_attr = AutoFuzSlpPosConfig.check_file_available(self.regional_attr)
        if self.topoparam is None:
            self.topoparam = TopoAttrNames(self.ws)
        if self.regional_attr is not None:
            self.topoparam.add_user_defined_attribute('rpi', self.regional_attr, True)

    def read_optiontyploc_section(self, _opttyploc):
        """Optional parameter-settings for Typical Locations selection"""
        if _opttyploc not in self.cf.sections():
            return
        # handling slope position types and tags
        if self.cf.has_option(_opttyploc, 'slopepositiontypes'):
            self.slppostype = list()
            typstrs = self.cf.get(_opttyploc, 'slopepositiontypes')
            self.slppostype = StringClass.split_string(typstrs.lower(), ',')
        else:
            # five slope position system will be adapted.
            pass
        if self.cf.has_option(_opttyploc, 'slopepositiontags'):
            self.slppostag = list()
            tagstrs = self.cf.get(_opttyploc, 'slopepositiontags')
            self.slppostag = StringClass.extract_numeric_values_from_string(tagstrs)
            if len(self.slppostag) != len(self.slppostype):
                raise RuntimeError("The input number of slope position types and "
                                   "tags are not the same!")
        else:
            self.slppostag = list()
            for i in range(len(self.slppostype)):
                self.slppostag.append(pow(2, i))
        for typ in self.slppostype:
            self.singleslpposconf[typ] = SingleSlpPosFiles(self.ws, typ)
        # handling selected topographic attributes
        if self.cf.has_option(_opttyploc, 'terrainattrdict'):
            self.selectedtopolist = list()
            self.selectedtopo = dict()
            terrain_attr_dict_str = self.cf.get(_opttyploc, 'terrainattrdict')
            attrpath_strs = StringClass.split_string(terrain_attr_dict_str, ';')
            for i, singattr in enumerate(attrpath_strs):
                ap = StringClass.split_string(singattr, ',')
                attrname = ap[0].lower()
                if i == 0 and not StringClass.string_match(attrname, 'rpi'):
                    attrname = 'rpi'
                self.selectedtopolist.append(attrname)
                attrpath = self.topoparam.get_attr_file(attrname)
                if attrpath is not None:
                    self.selectedtopo[attrname] = attrpath
                else:  # this should be user-defined attribute, and should has a valid file path
                    if len(ap) != 2:
                        raise RuntimeError("User defined topographic attribute (%s) MUST have "
                                           "an existed file path!" % singattr)
                    attrp = AutoFuzSlpPosConfig.check_file_available(ap[1])
                    if attrp is None:
                        raise RuntimeError("User defined topographic attribute (%s) MUST have "
                                           "an existed file path!" % singattr)
                    self.selectedtopo[attrname] = attrp
                    is_regional = False
                    if i == 0:  # the first one is regional attribute
                        is_regional = True
                    self.topoparam.add_user_defined_attribute(attrname, attrp, is_regional)
        # handling several parameters used in extracting typical location
        if self.cf.has_option(_opttyploc, 'typlocextractparam'):
            self.param4typloc = dict()
            base_param_str = self.cf.get(_opttyploc, 'typlocextractparam')
            base_param_floats = StringClass.extract_numeric_values_from_string(base_param_str)
            defnum = len(self._DEFAULT_PARAM_TYPLOC)
            if len(base_param_floats) == defnum:
                for slppos in self.slppostype:
                    self.param4typloc[slppos] = base_param_floats[:]
            elif len(base_param_floats) == len(self.slppostype) * defnum:
                for i, slppos in enumerate(self.slppostype):
                    self.param4typloc[slppos] = base_param_floats[i * defnum:(i + 1) * defnum]
            else:
                raise RuntimeError("TyplocExtractParam MUST has the number of "
                                   "%d or %d!" % (defnum, len(self.slppostype) * defnum))
        else:
            for slppos in self.slppostype:
                self.param4typloc[slppos] = self._DEFAULT_PARAM_TYPLOC[:]
        # handling Pre-defined fuzzy membership function shapes of each terrain attribute
        #    for each slope position
        if self.cf.has_option(_opttyploc, 'fuzinfdefault'):
            self.infshape = dict()
            fuz_inf_shp_strs = self.cf.get(_opttyploc, 'fuzinfdefault')
            # inference shapes are separated by SIMICOLON bewteen slope positions
            fuz_inf_shp_types = StringClass.split_string(fuz_inf_shp_strs, ';')
            if len(fuz_inf_shp_types) != len(self.slppostype):
                raise RuntimeError("FuzInfDefault (%s) MUST be consistent with slope position types"
                                   " and separated by ';'!" % fuz_inf_shp_strs)
            for i, slppos in enumerate(self.slppostype):
                self.infshape[slppos] = dict()
                # inference shapes are separated by COMMA bewteen topographic attributes
                infshps = StringClass.split_string(fuz_inf_shp_types[i], ',')
                if len(infshps) != len(self.selectedtopolist):
                    raise RuntimeError("FuzInfDefault (%s) for each slope position MUST have "
                                       "the same size with TerrainAttrDict" % fuz_inf_shp_types[i])
                for j, attrn in enumerate(self.selectedtopolist):
                    self.infshape[slppos][attrn] = infshps[j]
        else:
            if len(self.slppostype) != 5:
                raise RuntimeError("Only the fuzzy membership function shapes of "
                                   "5 slope position system are built-in. For other "
                                   "classification system, please set as input!")
        # handling value ranges of terrain attributes for extracting prototypes
        if self.cf.has_option(_opttyploc, 'valueranges'):
            self.extractrange = dict()
            value_rng_strs = self.cf.get(_opttyploc, 'valueranges')
            value_rng_types = StringClass.split_string(value_rng_strs, ';')
            if len(value_rng_types) != len(self.slppostype):
                raise RuntimeError("ValueRanges (%s) MUST be consistent with slope position types"
                                   " and separated by ';'!" % value_rng_strs)
            for i, slppos in enumerate(self.slppostype):
                self.extractrange[slppos] = dict()
                value_rngs = StringClass.extract_numeric_values_from_string(value_rng_types[i])
                if len(value_rngs) == 0 or len(value_rngs) % 3 != 0:
                    raise RuntimeError("Each item of ValueRanges MUST contains three elements,"
                                       "i.e., Attributes No., Min, Max! Please check item: "
                                       "%s for %s." % (value_rng_types[i], slppos))
                for j in range(int(len(value_rngs) / 3)):
                    attridx = int(value_rngs[j * 3]) - 1
                    attrname = self.selectedtopolist[attridx]
                    min_v = value_rngs[j * 3 + 1]
                    max_v = value_rngs[j * 3 + 2]
                    self.extractrange[slppos][attrname] = [min_v, max_v]
        else:
            if len(self.slppostype) != 5:
                raise RuntimeError("Only the extract value ranges of "
                                   "5 slope position system are built-in. For other "
                                   "classification system, please set as input!")

    def read_optionfuzinf_section(self, _optfuzinf):
        """Optional parameter-settings for Fuzzy slope position inference."""
        if _optfuzinf not in self.cf.sections():
            return
        if self.cf.has_option(_optfuzinf, 'inferparams'):
            fuzinf_strs = self.cf.get(_optfuzinf, 'inferparams')
            if StringClass.string_match(fuzinf_strs, 'none'):
                return
            self.inferparam = dict()
            fuzinf_types = StringClass.split_string(fuzinf_strs, ';')
            if len(fuzinf_types) != len(self.slppostype):
                raise RuntimeError("InferParams (%s) MUST be consistent with slope position types"
                                   " and separated by ';'!" % fuzinf_strs)
            for i, slppos in enumerate(self.slppostype):
                self.inferparam[slppos] = dict()
                infparams = StringClass.extract_numeric_values_from_string(fuzinf_types[i])
                if len(infparams) % 4 != 0:
                    raise RuntimeError("Each item of InferParams MUST contains four elements,"
                                       "i.e., Attribute No., FMF No., w1, w2! Please check item: "
                                       "%s for %s." % (fuzinf_types[i], slppos))
                for j in range(int(len(infparams) / 4)):
                    attridx = int(infparams[j * 4]) - 1
                    attrname = self.selectedtopolist[attridx]
                    fmf = self._FMFTYPE[int(infparams[j * 4 + 1])]
                    curinfparam = self._FMFPARAM[fmf][:]
                    curinfparam[0] = infparams[j * 4 + 2]  # w1
                    curinfparam[3] = infparams[j * 4 + 3]  # w2
                    self.inferparam[slppos][attrname] = [fmf] + curinfparam


def get_input_cfgs():
    """Get model configuration arguments.

    Returns:
            InputArgs object.
    """
    parser = argparse.ArgumentParser(description="Read AutoFuzSlpPos configurations.")
    parser.add_argument('-ini', help="Full path of configuration file.")
    parser.add_argument('-bin', help="Path of executable programs, which will override"
                                     "exeDir in *.ini file.")
    parser.add_argument('-proc', help="Number of processor for parallel computing, "
                                      "which will override inputProc in *.ini file.")
    parser.add_argument('-dem', help="DEM of study area.")
    parser.add_argument('-root', help="Workspace to store results, which will override "
                                      "rootDir in *.ini file.")
    args = parser.parse_args()

    ini_file = args.ini
    bin_dir = args.bin
    input_proc = args.proc
    rawdem = args.dem
    root_dir = args.root
    if input_proc is not None:
        xx = StringClass.extract_numeric_values_from_string(input_proc)
        if xx is None or len(xx) != 1:
            raise RuntimeError("-proc MUST be one integer number!")
        input_proc = int(xx[0])
    else:
        input_proc = -1
    if not FileClass.is_file_exists(ini_file):
        if FileClass.is_file_exists(rawdem) and os.path.isdir(bin_dir):
            # In this scenario, the script can be executed by default setting, i.e., the *.ini
            # file is not required.
            cf = None
            if input_proc < 0:
                input_proc = cpu_count() / 2
        else:
            raise RuntimeError("*.ini file MUST be provided when '-dem', '-bin', "
                               "and '-root' are not provided!")
    else:
        cf = ConfigParser()
        cf.read(ini_file)

    return AutoFuzSlpPosConfig(cf, bin_dir, input_proc, rawdem, root_dir)


if __name__ == '__main__':
    fuzslppos_cfg = get_input_cfgs()
