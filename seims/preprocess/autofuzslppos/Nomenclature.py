"""Predefined file names.

    @author   : Liangjun Zhu

    @changelog:
    - 15-03-20  - lj - initial implementation.
    - 17-07-30  - lj - reorganize and incorporate with pygeoc.
"""
from __future__ import absolute_import, unicode_literals
import os

from pygeoc.TauDEM import TauDEMFilesUtils
from pygeoc.utils import UtilClass, StringClass


class CreateWorkspace(object):
    """Create workspace directories for outputs."""

    def __init__(self, root_dir):
        """Create workspace directories for outputs.
        Args:
            root_dir: Root directory
        """
        if not os.path.isdir(root_dir):
            try:
                os.makedirs(root_dir)
            except Exception:  # failed of any types
                root_dir = UtilClass.current_path(lambda: 0) + os.sep + "FuzzySlpPos"
                os.mkdir(root_dir)
        self.root_dir = root_dir

        self.pre_dir = self.root_dir + os.sep + "PreDir"
        self.param_dir = self.root_dir + os.sep + "Params"
        self.log_dir = self.root_dir + os.sep + "Log"
        self.output_dir = self.root_dir + os.sep + "FuzzySlpPos"
        self.typloc_dir = self.root_dir + os.sep + "TypLoc"
        self.conf_dir = self.root_dir + os.sep + "Config"

        UtilClass.mkdir(self.pre_dir)
        UtilClass.mkdir(self.param_dir)
        UtilClass.mkdir(self.output_dir)
        UtilClass.mkdir(self.log_dir)
        UtilClass.mkdir(self.typloc_dir)
        UtilClass.mkdir(self.conf_dir)


class PreProcessAttrNames(TauDEMFilesUtils):
    """File names derived in preprocessing based TauDEM."""
    _DINFACCWITHWEIGHT = 'accTauDinfWithWeight.tif'
    _DINFSTREAM = 'stream_dinf.tif'
    _DIST2STREAM_V = 'dist2strm_v'
    _DIST2STREAM = 'dist2strm'
    _DIST2STREAM_ED = 'dist2strm_ed'
    _DIST2RIDGE_ED = 'dist2rdg_ed'
    _DISTUP2RDG = 'distup2ridge'
    _RIDGESRC = 'rdgsrc'
    _RPIHYDRO = 'rpi_hydro'
    _RPISKIDMORE = 'rpi_skidmore'

    def __init__(self, pre_dir, flow_model):
        """Initialize."""
        TauDEMFilesUtils.__init__(self, pre_dir)
        self.dinfacc_weight = pre_dir + os.sep + self._DINFACCWITHWEIGHT
        self.stream_dinf = pre_dir + os.sep + self._DINFSTREAM
        # File names dependent on flow model
        suffix = '_dinf.tif'
        if flow_model == 0:
            suffix = '_d8.tif'
        self.dist2stream_v = pre_dir + os.sep + self._DIST2STREAM_V + suffix
        self.dist2stream = pre_dir + os.sep + self._DIST2STREAM + suffix
        self.distup2rdg = pre_dir + os.sep + self._DISTUP2RDG + suffix
        self.rdgsrc = pre_dir + os.sep + self._RIDGESRC + suffix
        self.rpi_hydro = pre_dir + os.sep + self._RPIHYDRO + suffix
        self.rpi_skidmore = pre_dir + os.sep + self._RPISKIDMORE + suffix
        self.dist2stream_ed = pre_dir + os.sep + self._DIST2STREAM_ED + suffix
        self.dist2rdg_ed = pre_dir + os.sep + self._DIST2RIDGE_ED + suffix


class TopoAttrNames(object):
    """Topographic attributes names."""

    def __init__(self, ws):
        """Initialization."""
        self.rpi = ws.param_dir + os.sep + 'rpi.tif'
        self.profc = ws.param_dir + os.sep + 'profc.tif'
        self.horizc = ws.param_dir + os.sep + 'horizc.tif'
        self.slope = ws.param_dir + os.sep + 'slp.tif'
        self.hand = ws.param_dir + os.sep + 'hand.tif'
        self.elev = ws.param_dir + os.sep + 'elev.tif'
        self.pre_derived_terrain_attrs = {'rpi': self.rpi, 'profc': self.profc,
                                          'horizc': self.horizc, 'slp': self.slope,
                                          'hand': self.hand, 'elev': self.elev}
        self.region_attrs = ['rpi']

    def add_user_defined_attribute(self, toponame, topoattr_file, is_regional=True):
        """Add regional attribute specified by user, and return the key value (i.e., filename)."""
        if is_regional:
            toponame = 'rpi'
        self.pre_derived_terrain_attrs[toponame] = topoattr_file

    def get_attr_file(self, attrname):
        """Get the file path of pre-prepared topographic attribute."""
        if StringClass.string_match(attrname, 'rpi'):
            return self.rpi
        elif StringClass.string_match(attrname, 'profc'):
            return self.profc
        elif StringClass.string_match(attrname, 'horizc'):
            return self.horizc
        elif StringClass.string_match(attrname, 'slp'):
            return self.slope
        elif StringClass.string_match(attrname, 'elev'):
            return self.elev
        elif StringClass.string_match(attrname, 'hand'):
            return self.hand
        else:
            return None
            # raise RuntimeError("%s is not prepared by default, please provided "
            #                    "with it's filepath!" % attrname)


class SingleSlpPosFiles(object):
    """Predefined file names during deriving fuzzy slope position."""
    _EXTINITIAL = 'ExtConfigInitial.dat'
    _EXTCONFIG = 'ExtConfig.dat'
    _EXTLOG = 'ExtLog.dat'
    _TYPLOC = 'Typ.tif'
    _INFRECOMMEND = 'InfRecommend.dat'
    _INFCONFIG = 'InfConfig.dat'
    _FUZSLPPOS = 'Inf.tif'

    def __init__(self, ws, slppos_type):
        """Initialize by slope position type"""
        self.extinitial = ws.conf_dir + os.sep + slppos_type + self._EXTINITIAL
        self.extconfig = ws.conf_dir + os.sep + slppos_type + self._EXTCONFIG
        self.extlog = ws.log_dir + os.sep + slppos_type + self._EXTLOG
        self.typloc = ws.typloc_dir + os.sep + slppos_type + self._TYPLOC
        self.infrecommend = ws.conf_dir + os.sep + slppos_type + self._INFRECOMMEND
        self.infconfig = ws.conf_dir + os.sep + slppos_type + self._INFCONFIG
        self.fuzslppos = ws.output_dir + os.sep + slppos_type + self._FUZSLPPOS


class FuzSlpPosFiles(object):
    """Fuzzy slope position files."""

    def __init__(self, ws):
        """Initialization."""
        self.extconfig = ws.conf_dir + os.sep + "ExtConfig.dat"
        self.infconfig = ws.conf_dir + os.sep + "InfConfig.dat"
        self.harden_slppos = ws.output_dir + os.sep + "HardenSlpPos.tif"
        self.max_similarity = ws.output_dir + os.sep + "MaxSimilarity.tif"
        self.secharden_slppos = ws.output_dir + os.sep + "SecHardenSlpPos.tif"
        self.secmax_similarity = ws.output_dir + os.sep + "SecMaxSimilarity.tif"
        self.spsi = ws.output_dir + os.sep + "SPSI.tif"
        self.profiles = ws.output_dir + os.sep + "ProfileFuzSlpPos.shp"


class LogNames(object):
    """Runtime log file names."""

    def __init__(self, log_dir):
        """Initialize."""
        self.preproc = log_dir + os.sep + 'log_preprocessing.txt'
        self.all = log_dir + os.sep + 'log_fuzzyslppos.txt'
        self.runtime = log_dir + os.sep + 'log_runtime.txt'
