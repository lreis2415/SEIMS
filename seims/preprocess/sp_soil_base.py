from __future__ import absolute_import, unicode_literals

import os
import sys

import numpy

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from utility import DEFAULT_NODATA, UTIL_ZERO


class SoilPropertyBase(object):
    SEQN = "SEQN"
    SNAM = "SNAM"
    NLYRS = "SOILLAYERS"

    def __init__(self, seq_num, seq_name):
        """
        Initialize a soil property object.
        Args:
            seq_num (int): Soil sequence number, Unique identifier
            seq_name (str): The corresponding soil name
            SOILLAYERS (int, None): (nly in SWAT model, the same below) number of soil layers
            SOL_Z (float, mm): (sol_z) depth from the surface to bottom of soil layer
            SOILTHICK (float, mm): soil thickness for calculation convenient, derived from `SOILDEPTH`
            SOL_OM (float, %): organic matter content (weight percent)
            SOL_CBN (float, %): (sol_cbn) percent organic carbon in soil layer, calculated by `OM`
            SOL_N (float, %): (sol_n) used when using CSWAT = 1, i.e, C-FARM one carbon pool model, derived from `SOL_CBN`
            SOL_CLAY (float, %): (sol_clay) percent clay content in soil material, diameter < 0.002 mm
            SOL_SILT (float, %): (sol_silt) percent silt content in soil material,diameter between 0.002 mm and 0.05 mm
            SOL_SAND (float, %): (sol_sand) percent sand content in soil material,diameter between 0.05 mm and 2 mm
            SOL_ROCK (float, %): (sol_rock) percent of rock fragments content in soil material,diameter > 2 mm
            SOIL_TEXTURE (int, None): soil texture code used in WetSpa and SWAT model based on soil texture triangle by USDA
            HYDRO_GROUP (int, None): Hydrological soil group, 1, 2, 3, and 4 to represent A, B, C, and D
            SOL_ZMX (float, mm): (sol_zmx) maximum rooting depth of soil profile
            ANION_EXCL (float, None): (anion_excl) fraction of porosity from which anions are excluded, default is 0.5
            SOL_CRK (float, None): (sol_crk) crack volume potential of soil expressed as a fraction of the total soil volume
            SOL_BD (float, Mg/m3 or g/cm3): (sol_bd) bulk density of each soil layer
            SOL_AVBD (float, Mg/m3 or g/cm3): (sol_avbd) average bulk density for soil profile
            SOL_K (float, mm/hr): (sol_k) saturated hydraulic conductivity
            SOL_HK (float, None): (sol_hk) beta coefficent to calculate hydraulic conductivity
            SOL_WP (float, mm H2O / mm soil): (sol_wp) water content of soil at -1.5 MPa (wilting point)
            SOL_WPMM (float, mm H2O): (sol_wpmm) water content of soil at -1.5 MPa (wilting point)
            SOL_SUMWP (float, mm H2O): (sol_sumwp) amount of water held in the soil profile at wilting point
            SOL_FC (float,mm H2O / mm soil): (sol_up) water content of soil at -0.033 MPa (field capacity)
            AWC (float,mm H2O / mm soil): (sol_awc) available water capacity of soil layer
            SOL_AWC  (float,mm H2O): (sol_fc) amount of water available to plants in soil layer at field capacity (fc - wp)
            SOL_SUMAWC (float,mm H2O): (sol_sumfc) amount of water held in soil profile at field capacity
            SOL_POROSITY (float,None): (sol_por) total porosity of soil layer expressed as a fraction of the total volume
            POREINDEX (float,None): pore size distribution index
            SOL_AVPOR (float,None): (sol_avpor) average porosity for entire soil profile
            SOL_UL (float,mm H2O): (sol_ul) amount of water held in the soil layer at saturation (sat - wp water)
            SOL_SUMUL (float,mm H2O): (sol_sumul) amount of water held in soil profile at saturation
            USLE_K (float,None):  USLE K factor
            SOL_ALB (float,None): albedo of top soil surface
            WFSH (float,mm): wetting front matric potential (usde in Green-Ampt method)
            ESCO (float,None): soil evaporation compensation factor
            VWT (float,None): (vwt) variable water table factor, used in percolation modules
            DET_SAND (float,None): (det_san) detached sediment size distribution, sand fraction
            DET_SILT (float,None): (det_sil) detached sediment size distribution, silt fraction
            DET_CLAY (float,None): (det_cla) detached sediment size distribution, clay fraction
            DET_SMAGG (float,None): (det_sag) detached sediment size distribution, small aggregation fraction
            DET_LGAGG (float,None): (det_lag) detached sediment size distribution, large aggregation fraction
            CRDEP (float,mm): (crdep) maximum or potential crack volume
            VOLCR (float,mm): (volcr) crack volume for soil layer, should be calculated in SEIMS, using moist_ini
            SOL_NO3 (float,kg/ha): (sol_no3) concentration of nitrate in soil layers
            SOL_NH4 (float,kg/ha): (sol_nh4) concentration of ammonium-N in soil layers
            SOL_ORGN (float,kg/ha): (sol_orgn) organic N concentration in soil layers
            SOL_ORGP (float,kg/ha): (sol_orgp) organic P concentration in soil layers
            SOL_SOLP (float,kg/ha): (sol_solp) soluble P concentration in soil layers
        """
        self.SEQN = seq_num
        self.SNAM = seq_name
        self.SOILLAYERS = DEFAULT_NODATA
        self.SOILDEPTH = list()
        self.SOL_Z = list()
        self.SOILTHICK = list()

        self.SOL_OM = list()  # OM
        self.SOL_CBN = list()
        self.SOL_N = list()
        self.SOL_CLAY = list()  # CLAY
        self.SOL_SILT = list()  # SILT
        self.SOL_SAND = list()  # SAND
        self.SOL_ROCK = list()  # ROCK
        self.SOIL_TEXTURE = DEFAULT_NODATA
        self.HYDRO_GROUP = DEFAULT_NODATA
        self.SOL_ZMX = DEFAULT_NODATA
        self.ANION_EXCL = DEFAULT_NODATA
        self.SOL_CRK = DEFAULT_NODATA
        self.SOL_BD = list()  # DENSITY
        self.SOL_AVBD = list()
        self.SOL_K = list()  # CONDUCTIVITY
        self.SOL_HK = list()
        self.SOL_WP = list()
        self.SOL_WPMM = list()
        self.SOL_SUMWP = 0.
        self.SOL_FC = list()  # FIELDCAP, Field capacity
        self.SOL_AWC = list()
        self.SOL_AWC_AMOUNT = list()
        self.SOL_SUMAWC = 0.
        self.SOL_POROSITY = list()  # POROSITY
        self.POREINDEX = list()
        self.SOL_AVPOR = DEFAULT_NODATA
        self.SOL_UL = list()
        self.SOL_SUMUL = 0.
        self.USLE_K = list()
        self.SOL_ALB = DEFAULT_NODATA
        self.WFSH = DEFAULT_NODATA
        self.VWT = list()
        self.DET_SAND = DEFAULT_NODATA
        self.DET_SILT = DEFAULT_NODATA
        self.DET_CLAY = DEFAULT_NODATA
        self.DET_SMAGG = DEFAULT_NODATA
        self.DET_LGAGG = DEFAULT_NODATA
        self.CRDEP = list()
        self.ESCO = DEFAULT_NODATA
        # Here after are general soil chemical properties
        self.SOL_NO3 = list()
        self.SOL_NH4 = list()
        self.SOL_ORGN = list()
        self.SOL_ORGP = list()
        self.SOL_SOLP = list()

        # Conceptual model parameters. Can be used in physical model as well.
        self.GR4J_X2 = list()
        self.GR4J_X3 = list()
        self.MAX_IF_RATE = list()

    @staticmethod
    def soil_param_type():
        """
        Return the soil parameter type str.
        e.g., "conceptual" or "physical"
        """
        raise NotImplementedError("Please implement this method in subclass!")

    def soil_dict(self):
        """Convert to dict and remove the empty elements"""
        sol_dict = self.__dict__.copy()
        sol_dict.pop(SoilPropertyBase.SNAM)
        # remove the empty element
        remove_indices = list()
        for ele in sol_dict:
            if (isinstance(sol_dict[ele], list) and not sol_dict[ele]) or (str(sol_dict[ele]) == str(DEFAULT_NODATA)):
                # del sol_dict[ele]
                remove_indices.append(ele)
        for ele in remove_indices:
            del sol_dict[ele]
        return sol_dict

    def _pre_construct(self, *args, **kwargs):
        pass

    def _construct(self):
        pass

    def construct(self, *args, **kwargs):
        if self.SOILLAYERS == DEFAULT_NODATA:
            raise ValueError("Soil layer numbers is REQUIRED, please check the input file!")

        self._pre_construct(*args, **kwargs)

        if len(self.SOL_Z) == 0 or len(self.SOL_Z) != self.SOILLAYERS or \
            DEFAULT_NODATA in self.SOL_Z:
            raise IndexError("Soil depth must have a size equal to NLAYERS and "
                             "should not include NODATA (-9999)!")

        # Calculate soil thickness of each layer
        for lyr in range(self.SOILLAYERS):
            if lyr == 0:
                self.SOILTHICK.append(self.SOL_Z[lyr])
            else:
                self.SOILTHICK.append(self.SOL_Z[lyr] - self.SOL_Z[lyr - 1])

        self._construct()

    def find_and_set_attr(self, attr, value):
        if not hasattr(self, attr):
            return
        # if attr = [...], value = [...], replace it
        if isinstance(getattr(self, attr), list) and isinstance(value, list):
            setattr(self, attr, value)
        # elseif attr = [...], value = ..., append it
        elif isinstance(getattr(self, attr), list) and not isinstance(value, list):
            getattr(self, attr).append(value)
        # else if attr is single value, replace it
        else:
            setattr(self, attr, value)

    def check_layers(self, var: list, is_required=True, nodata_allowed=True):
        if len(var) != self.SOILLAYERS and is_required:
            raise ValueError(f"The length of {var} is not equal to SOILLAYERS ({self.SOILLAYERS})!")
        if not nodata_allowed and DEFAULT_NODATA in var:
            raise ValueError(f"{var} should not contain NODATA (-9999)!")

    def auto_fill_SOM(self):
        if not self.SOL_OM:
            return False
        if DEFAULT_NODATA in self.SOL_OM and self.SOL_OM.index(DEFAULT_NODATA) >= 2 and self.SOILLAYERS >= 3:
            for i in range(2, self.SOILLAYERS):
                if self.SOL_OM[i] == DEFAULT_NODATA:
                    self.SOL_OM[i] = self.SOL_OM[i - 1] * numpy.exp(-self.SOILTHICK[i])  # mm
        return True

    def auto_fill_CBN(self):
        """Calculate sol_cbn = sol_om * 0.58"""
        if not self.SOL_CBN or len(self.SOL_CBN) != self.SOILLAYERS:
            self.SOL_CBN = list()
            for i in range(self.SOILLAYERS):
                if self.SOL_OM[i] * 0.58 < UTIL_ZERO:
                    self.SOL_CBN.append(0.1)
                else:
                    self.SOL_CBN.append(self.SOL_OM[i] * 0.58)

    def auto_fill_N(self):
        if not self.SOL_N or len(self.SOL_N) != self.SOILLAYERS:
            self.SOL_N = list()
            for i in range(self.SOILLAYERS):
                self.SOL_N.append(self.SOL_CBN[i] / 11.)
