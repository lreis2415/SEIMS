"""Extract spatial soil parameters

    @author   : Liangjun Zhu, Junzhi Liu, Huiran Gao, Fang Shen

    @changelog:
    - 13-01-10  jz - initial implementation
    - 16-07-22  lj - Names and units of soil physical parameter are referred to
                     readsol.f, soil_par.f, and soil_phys.f in SWAT.
                     Data validation checking is also conducted.
    - 16-12-07  lj - rewrite for version 2.0
    - 17-06-23  lj - reorganize as basic class
    - 18-02-08  lj - compatible with Python3.
    - 22-06-08  lj - use mask_rasterio to reclassify soil and landuse parameters
"""
from __future__ import absolute_import, unicode_literals

import math
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import numpy
from osgeo.gdal import GDT_Float32
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import StringClass

from utility import DEFAULT_NODATA, UTIL_ZERO, MINI_SLOPE
from utility import status_output, read_data_items_from_txt
from utility import mask_rasterio


class SoilProperty(object):
    """
    base class of Soil physical and general chemical properties
    Attributes:
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
    _SEQN = "SEQN"
    _NAME = "SNAM"

    def __init__(self, seq_num, seq_name):
        """
        Initialize a soil property object.
        Args:
            seq_num (int): Soil sequence number, Unique identifier
            seq_name (str): The corresponding soil name
        """
        self.SEQN = seq_num
        self.SNAM = seq_name
        self.SOILLAYERS = DEFAULT_NODATA
        self.SOL_Z = list()  # SOILDEPTH
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
        self.AWC = list()  # 为什么有两个AWC
        self.SOL_AWC = list()
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
        self.GR4J_X2 = list()
        self.GR4J_X3 = list()

    def soil_dict(self):
        """Convert to dict"""
        sol_dict = self.__dict__
        sol_dict.pop(SoilProperty._NAME)
        # remove the empty element
        remove_indices = list()
        for ele in sol_dict:
            if isinstance(sol_dict[ele], list) and not sol_dict[ele]:
                # del sol_dict[ele]
                remove_indices.append(ele)
        for ele in remove_indices:
            del sol_dict[ele]
        return sol_dict

    def check_data_validation(self):
        """Check the required input, and calculate all physical and general chemical properties"""
        # set a soil layer at dep_new and adjust all lower layers
        # a septic layer:0-10mm, refers to layersplit.f in SWAT
        if self.SOILLAYERS == DEFAULT_NODATA:
            raise ValueError("Soil layers number must be provided, please check the input file!")
        dep_new = 10.
        if self.SOL_Z[0] - dep_new >= 10.:
            self.SOILLAYERS += 1
            # Required attributes
            self.SOL_Z.insert(0, dep_new)
            if self.SOL_OM:
                self.SOL_OM.insert(0, self.SOL_OM[0])
            else:
                raise ValueError("Organic matter must be provided!")
            if self.SOL_CLAY:
                self.SOL_CLAY.insert(0, self.SOL_CLAY[0])
            else:
                raise ValueError("Clay content must be provided!")
            if self.SOL_SILT:
                self.SOL_SILT.insert(0, self.SOL_SILT[0])
            else:
                raise ValueError("Silt content must be provided!")
            if self.SOL_SAND:
                self.SOL_SAND.insert(0, self.SOL_SAND[0])
            else:
                raise ValueError("Sand content must be provided!")
            if self.SOL_ROCK:
                self.SOL_ROCK.insert(0, self.SOL_ROCK[0])
            else:
                raise ValueError("Rock content must be provided!")
            # Optional attributes
            if self.SOL_FC:
                self.SOL_FC.insert(0, self.SOL_FC[0])
            if self.SOL_BD:
                self.SOL_BD.insert(0, self.SOL_BD[0])
            if self.SOL_K:
                self.SOL_K.insert(0, self.SOL_K[0])
            if self.SOL_WP:
                self.SOL_WP.insert(0, self.SOL_WP[0])
            if self.AWC:
                self.AWC.insert(0, self.AWC[0])
                for i in range(self.SOILLAYERS):
                    if self.AWC[i] <= 0.:
                        self.AWC[i] = 0.005
                    elif self.AWC[i] <= 0.01:
                        self.AWC[i] = 0.01
                    elif self.AWC[i] >= 0.8:
                        self.AWC[i] = 0.8
            if self.SOL_POROSITY:
                self.SOL_POROSITY.insert(0, self.SOL_POROSITY[0])
            if self.USLE_K:
                self.USLE_K.insert(0, self.USLE_K[0])
            if self.SOL_NO3:
                self.SOL_NO3.insert(0, self.SOL_NO3[0])
            else:
                self.SOL_NO3 = list(numpy.zeros(self.SOILLAYERS))
            if self.SOL_NH4:
                self.SOL_NH4.insert(0, self.SOL_NH4[0])
            else:
                self.SOL_NH4 = list(numpy.zeros(self.SOILLAYERS))
            if self.SOL_ORGN:
                self.SOL_ORGN.insert(0, self.SOL_ORGN[0])
            else:
                self.SOL_ORGN = list(numpy.zeros(self.SOILLAYERS))
            if self.SOL_SOLP:
                self.SOL_SOLP.insert(0, self.SOL_SOLP[0])
            else:
                self.SOL_SOLP = list(numpy.zeros(self.SOILLAYERS))
            if self.SOL_ORGP:
                self.SOL_ORGP.insert(0, self.SOL_ORGP[0])
            else:
                self.SOL_ORGP = list(numpy.zeros(self.SOILLAYERS))
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
        if self.SOL_ZMX == DEFAULT_NODATA or self.SOL_ZMX > self.SOL_Z[-1] \
            or self.SOL_ZMX < self.SOL_Z[-1]:
            self.SOL_ZMX = self.SOL_Z[-1]
        if self.ANION_EXCL == DEFAULT_NODATA:
            self.ANION_EXCL = 0.5
        if not self.SOL_OM or len(self.SOL_OM) != self.SOILLAYERS:
            raise IndexError("Soil organic matter must have a size equal to NLAYERS!")
        elif DEFAULT_NODATA in self.SOL_OM and self.SOL_OM.index(
            DEFAULT_NODATA) >= 2 and self.SOILLAYERS >= 3:
            for i in range(2, self.SOILLAYERS):
                if self.SOL_OM[i] == DEFAULT_NODATA:
                    self.SOL_OM[i] = self.SOL_OM[i - 1] * numpy.exp(-self.SOILTHICK[i])  # mm
        # Calculate sol_cbn = sol_om * 0.58
        if not self.SOL_CBN or len(self.SOL_CBN) != self.SOILLAYERS:
            self.SOL_CBN = list()
            for i in range(self.SOILLAYERS):
                if self.SOL_OM[i] * 0.58 < UTIL_ZERO:
                    self.SOL_CBN.append(0.1)
                else:
                    self.SOL_CBN.append(self.SOL_OM[i] * 0.58)
        # Calculate sol_n = sol_cbn/11.
        if not self.SOL_N or len(self.SOL_N) != self.SOILLAYERS:
            self.SOL_N = list()
            for i in range(self.SOILLAYERS):
                self.SOL_N.append(self.SOL_CBN[i] / 11.)
        if not self.SOL_CLAY or len(self.SOL_CLAY) != self.SOILLAYERS or DEFAULT_NODATA in self.SOL_CLAY:
            raise IndexError("Clay content must have a size equal to NLAYERS and "
                             "should not include NODATA (-9999)!")
        if not self.SOL_SILT or len(self.SOL_SILT) != self.SOILLAYERS or DEFAULT_NODATA in self.SOL_SILT:
            raise IndexError("Silt content must have a size equal to NLAYERS and "
                             "should not include NODATA (-9999)!")
        if not self.SOL_SAND or len(self.SOL_SAND) != self.SOILLAYERS or DEFAULT_NODATA in self.SOL_SAND:
            raise IndexError("Sand content must have a size equal to NLAYERS and "
                             "should not include NODATA (-9999)!")
        if not self.SOL_ROCK or len(self.SOL_ROCK) != self.SOILLAYERS or DEFAULT_NODATA in self.SOL_ROCK:
            raise IndexError("Rock content must have a size equal to NLAYERS and "
                             "should not include NODATA (-9999)!")

        # temporary variables
        tmp_fc = list()
        tmp_sat = list()
        tmp_wp = list()
        for i in range(self.SOILLAYERS):
            s = self.SOL_SAND[i] * 0.01  # % -> decimal
            c = self.SOL_CLAY[i] * 0.01
            om = self.SOL_OM[i]
            wpt = -0.024 * s + 0.487 * c + 0.006 * om + 0.005 * s * om \
                  - 0.013 * c * om + 0.068 * s * c + 0.031
            tmp_wp.append(1.14 * wpt - 0.02)
            fct = -0.251 * s + 0.195 * c + 0.011 * om + 0.006 * s * om \
                  - 0.027 * c * om + 0.452 * s * c + 0.299
            fc = fct + 1.283 * fct * fct - 0.374 * fct - 0.015
            s33t = 0.278 * s + 0.034 * c + 0.022 * om - 0.018 * s * om \
                   - 0.027 * c * om - 0.584 * s * c + 0.078
            s33 = 1.636 * s33t - 0.107
            sat = fc + s33 - 0.097 * s + 0.043
            tmp_fc.append(fc)
            tmp_sat.append(sat)

        if self.SOL_WP and len(self.SOL_WP) != self.SOILLAYERS:
            raise IndexError("Wilting point must have a size equal to soil layers number!")
        elif not self.SOL_WP:
            self.SOL_WP = tmp_wp[:]
        elif DEFAULT_NODATA in self.SOL_WP:
            for i in range(self.SOILLAYERS):
                if self.SOL_WP[i] == DEFAULT_NODATA:
                    self.SOL_WP[i] = tmp_wp[i]
        if self.SOL_BD and len(self.SOL_BD) != self.SOILLAYERS:
            raise IndexError("Bulk density must have a size equal to soil layers number!")
        elif not self.SOL_BD or DEFAULT_NODATA in self.SOL_BD:
            tmp_bd = list()
            for i in range(self.SOILLAYERS):
                sat = tmp_sat[i]
                fc = tmp_fc[i]
                if self.SOL_FC and len(self.SOL_FC) == self.SOILLAYERS:
                    sat = sat - fc + self.SOL_FC[i]
                tmp_bd.append(2.65 * (1.0 - sat))
            if DEFAULT_NODATA in self.SOL_BD:
                for i in range(self.SOILLAYERS):
                    if self.SOL_BD[i] == DEFAULT_NODATA:
                        self.SOL_BD[i] = tmp_bd[i]
            elif not self.SOL_BD:
                self.SOL_BD = tmp_bd[:]
        if self.SOL_FC and len(self.SOL_FC) != self.SOILLAYERS:
            raise IndexError("Field capacity must have a size equal to soil layers number!")
        elif not self.SOL_FC or DEFAULT_NODATA in self.SOL_FC:
            tmp_fc_bdeffect = list()
            for i in range(self.SOILLAYERS):
                fc = tmp_fc[i]
                sat = tmp_sat[i]
                if len(self.SOL_BD) != 0 and len(self.SOL_BD) == self.SOILLAYERS:
                    p_df = self.SOL_BD[i]
                else:
                    p_df = 2.65 * (1. - sat)
                sat_df = 1. - p_df / 2.65
                tmp_fc_bdeffect.append(fc - 0.2 * (sat - sat_df))
            if DEFAULT_NODATA in self.SOL_FC:
                for i in range(self.SOILLAYERS):
                    if self.SOL_FC[i] == DEFAULT_NODATA:
                        self.SOL_FC[i] = tmp_fc_bdeffect[i]
            elif not self.SOL_FC:
                self.SOL_FC = tmp_fc_bdeffect[:]
        if self.AWC and len(self.AWC) != self.SOILLAYERS:
            raise IndexError("Available water capacity must have the size equal to"
                             " soil layers number!")
        elif not self.AWC:
            for i in range(self.SOILLAYERS):
                self.AWC.append(self.SOL_FC[i] - self.SOL_WP[i])
        elif DEFAULT_NODATA in self.AWC:
            for i in range(self.SOILLAYERS):
                if self.AWC[i] == DEFAULT_NODATA:
                    self.AWC[i] = self.SOL_FC[i] - self.SOL_WP[i]

        if self.POREINDEX and len(self.POREINDEX) != self.SOILLAYERS:
            raise IndexError("Pore disconnectedness index must have a size "
                             "equal to soil layers number!")
        elif not self.POREINDEX:
            for i in range(self.SOILLAYERS):
                # An fitted equation proposed by Cosby et al. (1984) is adopted. By LJ, 2016-9-22
                b = 0.159 * self.SOL_CLAY[i] + 2.91
                self.POREINDEX.append(b)
                # previous version, currently deprecated by LJ
                # fc = self.FIELDCAP[i]
                # wp = self.WILTINGPOINT[i]
                # b = (math.log(1500.) - math.log(33.)) / (math.log(fc) - math.log(wp))
                # self.POREINDEX.append(1.0 / b)
        if self.SOL_POROSITY and len(self.SOL_POROSITY) != self.SOILLAYERS:
            raise IndexError("Soil Porosity must have a size equal to soil layers number!")
        elif not self.SOL_POROSITY:
            for i in range(self.SOILLAYERS):
                self.SOL_POROSITY.append(1. - self.SOL_BD[i] / 2.65)  # from the theory of swat
        elif DEFAULT_NODATA in self.SOL_POROSITY:
            for i in range(self.SOILLAYERS):
                if self.SOL_POROSITY[i] == DEFAULT_NODATA:
                    self.SOL_POROSITY[i] = 1. - self.SOL_BD[i] / 2.65
        if self.SOL_K and len(self.SOL_K) != self.SOILLAYERS:
            raise IndexError("Saturated conductivity must have a size equal to soil layers number!")
        elif not self.SOL_K or DEFAULT_NODATA in self.SOL_K:
            tmp_k = list()
            for i in range(self.SOILLAYERS):
                lamda = self.POREINDEX[i]
                fc = tmp_fc[i]
                sat = tmp_sat[i]
                tmp_k.append(1930. * pow(sat - fc, 3. - lamda))
            if not self.SOL_K:
                self.SOL_K = tmp_k[:]
            elif DEFAULT_NODATA in self.SOL_K:
                for i in range(self.SOILLAYERS):
                    if self.SOL_K[i] == DEFAULT_NODATA:
                        self.SOL_K[i] = tmp_k[i]
        # calculate water content of soil at -1.5 MPa and -0.033 MPa, refers to soil_phys.f in SWAT
        if not self.SOL_WP:
            for i in range(self.SOILLAYERS):
                tmpwp = 0.4 * self.SOL_CLAY[i] * self.SOL_BD[i] / 100.
                if tmpwp <= 0.:
                    tmpwp = 0.005
                self.SOL_WP.append(tmpwp)
        # calculate field capcity (sol_up)
        if not self.SOL_FC:
            for i in range(self.SOILLAYERS):
                self.SOL_FC.append(self.SOL_WP[i] + self.AWC[i])
        # calculate porosity
        if not self.SOL_POROSITY:
            for i in range(self.SOILLAYERS):
                self.SOL_POROSITY.append(1. - self.SOL_BD[i] / 2.65)
        if self.SOL_CRK == DEFAULT_NODATA:
            self.SOL_CRK = numpy.mean(self.SOL_POROSITY)
        for i in range(self.SOILLAYERS):
            if self.SOL_FC[i] >= self.SOL_POROSITY[i]:
                self.SOL_FC[i] = self.SOL_POROSITY[i] - 0.05
                self.SOL_WP[i] = self.SOL_FC[i] - self.AWC[i]
                if self.SOL_WP[i] <= 0.:
                    self.SOL_FC[i] = self.SOL_POROSITY[i] * 0.75
                    self.SOL_WP[i] = self.SOL_POROSITY[i] * 0.25
            # compute drainable porosity and variable water table factor
            drpor = self.SOL_POROSITY[i] - self.SOL_FC[i]
            self.VWT.append((437.13 * drpor * drpor) - (95.08 * drpor) + 8.257)
        sa = self.SOL_SAND[0] / 100.
        cl = self.SOL_CLAY[0] / 100.
        si = self.SOL_SILT[0] / 100.
        # determine detached sediment size distribution typical for mid-western soils in USA
        #  (Foster et al., 1980) Based on SWRRB.
        self.DET_SAND = 2.49 * sa * (1. - cl)
        self.DET_SILT = 0.13 * si
        self.DET_CLAY = 0.2 * cl
        if cl < 0.25:
            self.DET_SMAGG = 2.0 * cl
        elif cl > 0.5:
            self.DET_SMAGG = 0.57
        else:
            self.DET_SMAGG = 0.28 * (cl - 0.25) + 0.5
        self.DET_LGAGG = 1. - self.DET_SAND - self.DET_SILT - self.DET_CLAY - self.DET_SMAGG
        # Error check, may happen for soils with more sand. The fraction should not added up to 1.0
        if self.DET_LGAGG < 0.:
            self.DET_SAND /= (1. - self.DET_LGAGG)
            self.DET_SILT /= (1. - self.DET_LGAGG)
            self.DET_CLAY /= (1. - self.DET_LGAGG)
            self.DET_SMAGG /= (1. - self.DET_LGAGG)
            self.DET_LGAGG = 0.
        # initialize water/drainage coefs for each soil layer
        sumpor = 0.
        self.SOL_SUMUL = 0.
        self.SOL_SUMAWC = 0.
        self.SOL_SUMWP = 0.
        for i in range(self.SOILLAYERS):
            pormm = self.SOL_POROSITY[i] * self.SOILTHICK[i]
            sumpor += pormm
            self.SOL_UL.append((self.SOL_POROSITY[i] - self.SOL_WP[i]) * self.SOILTHICK[i])
            self.SOL_SUMUL += self.SOL_UL[i]
            self.SOL_AWC.append((self.SOL_FC[i] - self.SOL_WP[i]) * self.SOILTHICK[i])
            self.SOL_SUMAWC += self.SOL_AWC[i]
            self.SOL_HK.append((self.SOL_UL[i] - self.SOL_AWC[i]) / self.SOL_K[i])
            if self.SOL_HK[i] < 1.:
                self.SOL_HK[i] = 1.
            self.SOL_WPMM.append(self.SOL_WP[i] * self.SOILTHICK[i])
            self.SOL_SUMWP += self.SOL_WPMM[i]
            self.CRDEP.append(self.SOL_CRK * 0.916 * math.exp(-0.0012 * self.SOL_Z[i])
                              * self.SOILTHICK[i])

        self.SOL_AVPOR = sumpor / self.SOL_Z[self.SOILLAYERS - 1]
        self.SOL_AVBD = 2.65 * (1. - self.SOL_AVPOR)
        # calculate infiltration parameters for sub-daily time step
        self.WFSH = 10. * math.exp(6.5309 - 7.32561 * self.SOL_POROSITY[0] + 3.809479
                                   * math.pow(self.SOL_POROSITY[0], 2) + 0.001583
                                   * math.pow(self.SOL_CLAY[0], 2) + 0.000344 * self.SOL_SAND[0]
                                   * self.SOL_CLAY[0] - 0.049837 * self.SOL_POROSITY[0] * self.SOL_SAND[0]
                                   + 0.001608 * math.pow(self.SOL_POROSITY[0], 2)
                                   * math.pow(self.SOL_SAND[0], 2) + 0.001602
                                   * math.pow(self.SOL_POROSITY[0], 2) * math.pow(self.SOL_CLAY[0], 2)
                                   - 0.0000136 * math.pow(self.SOL_SAND[0], 2) * self.SOL_CLAY[0]
                                   - 0.003479 * math.pow(self.SOL_CLAY[0], 2) * self.SOL_POROSITY[0]
                                   - 0.000799 * math.pow(self.SOL_SAND[0], 2) * self.SOL_POROSITY[0])
        if self.SOL_ALB == DEFAULT_NODATA:
            self.SOL_ALB = 0.2227 * math.exp(-1.8672 * self.SOL_CBN[0])
        if self.ESCO == DEFAULT_NODATA:
            self.ESCO = 0.95
        if self.USLE_K and len(self.USLE_K) != self.SOILLAYERS:
            raise IndexError("USLE K factor must have a size equal to NLAYERS!")
        elif not self.USLE_K or DEFAULT_NODATA in self.USLE_K:
            tmp_usle_k = list()
            for i in range(self.SOILLAYERS):
                sand = self.SOL_SAND[i]
                silt = self.SOL_SILT[i]
                clay = self.SOL_CLAY[i]
                om = self.SOL_OM[i]
                tmp_usle_k.append(SoilProperty.usle_k_epic(sand, silt, clay, om))
            if not self.USLE_K:
                self.USLE_K = tmp_usle_k[:]
            elif DEFAULT_NODATA in self.USLE_K:
                for i in range(self.SOILLAYERS):
                    if self.USLE_K[i] == DEFAULT_NODATA:
                        self.USLE_K[i] = tmp_usle_k[i]
        if self.SOIL_TEXTURE == DEFAULT_NODATA or self.HYDRO_GROUP == DEFAULT_NODATA:
            st, hg, uslek = SoilUtilClass.getsoiltexture_usda(self.SOL_CLAY[0], self.SOL_SILT[0],
                                                              self.SOL_SAND[0])
            self.SOIL_TEXTURE = st
            self.HYDRO_GROUP = hg
        # Unit conversion for general soil chemical properties
        wt1 = list()
        for j in range(self.SOILLAYERS):
            # g/kg => kg/ha
            wt1.append(self.SOL_BD[j] * self.SOILTHICK[j] * 10.)
        if self.SOL_NO3 and len(self.SOL_NO3) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_NO3[j] = self.SOL_NO3[j] * wt1[j]
        if self.SOL_NH4 and len(self.SOL_NH4) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_NH4[j] = self.SOL_NH4[j] * wt1[j]
        if self.SOL_ORGN and len(self.SOL_ORGN) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_ORGN[j] = self.SOL_ORGN[j] * wt1[j]
        if self.SOL_SOLP and len(self.SOL_SOLP) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_SOLP[j] = self.SOL_SOLP[j] * wt1[j]
        if self.SOL_ORGP and len(self.SOL_ORGP) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_ORGP[j] = self.SOL_ORGP[j] * wt1[j]

    @staticmethod
    def usle_k_epic(sand, silt, clay, om):
        """Calculate USLE_K factor according to EPIC (Erosion Productivity Impact Calculator).

        References:
            1. Sharply, A. N., & Williams, J. R. (1990).
               EPIC-erosion/productivity impact calculator I, Model documentation.
               U.S. Department of Agriculture Technical Bulletin, No. 1768, page 26, Eq. 2.96.
            2. Equation. 4:1.1.5 - 4:1.1.9 in SWAT Theory 2009.
               Note that one number is wrong, i.e., 0.0256 rather than 0.256 in SWAT theory doc.

        TODO: Add more improved algorithms such as Wang et al. (2016, ISWCR)
        """
        cbn = om * 0.58
        sn = 1. - sand * 0.01
        a = (0.2 + 0.3 * math.exp(-0.0256 * sand * (1. - silt * 0.01)))
        b = math.pow(silt / (clay + silt), 0.3)
        c = (1. - 0.25 * cbn / (cbn + math.exp(3.72 - 2.95 * cbn)))
        d = (1. - 0.7 * sn / (sn + math.exp(-5.51 + 22.9 * sn)))
        k = a * b * c * d
        return k


class SoilUtilClass(object):
    """Soil parameters related utility functions."""
    _SEQN = "SEQN"
    _NAME = "SNAM"
    _NLYRS = "SOILLAYERS"

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def getsoiltexture_usda(clay, silt, sand):
        """The soil texture code system is from WetSpa Extension and SWAT model which is
        based on the soil texture triangle developed by USDA.
        The unit is percentage, silt + sand + clay [+ Rock] = 100.
            The corresponding default soil parameters (e.g. Ks, porosity) are stored in
        `seims/database/SoilLookup.csv`.
        Args:
            clay: clay content percentage
            silt: silt content percentage
            sand: sand content percentage

        Returns:
            [Soil texture ID, Hydrological soil group, USLE K factor]
        """
        if clay >= 40 >= silt and sand <= 45:
            return [12, 1, 0.22]  # clay / nian tu
        elif clay >= 40 and silt >= 40:
            return [11, 1, 0.26]  # silt caly / fen nian tu
        elif clay >= 35 and sand >= 45:
            return [10, 1, 0.28]  # sandy clay / sha nian tu
        elif clay >= 25 and 20 <= sand <= 45:
            return [9, 2, 0.3]  # clay loam / nian rang tu
        elif clay >= 25 and sand <= 20:
            return [8, 2, 0.32]  # silt clay loam / fen zhi nian rang tu
        elif clay >= 20 and silt <= 30 and sand >= 45:
            return [7, 2, 0.2]  # sandy clay loam / sha zhi nian rang tu
        elif clay >= 10 and 30 <= silt <= 50 and sand <= 50:
            return [6, 3, 0.3]  # loam / rang tu
        elif 50 <= silt <= 80 or clay >= 15 and silt >= 80:
            return [4, 3, 0.38]  # silt loam / fen zhi rang tu
        elif silt >= 80 and clay <= 15:
            return [5, 3, 0.34]  # silt / fen tu
        elif clay <= 10 and sand <= 50 or 50 <= sand <= 80:
            return [3, 4, 0.13]  # sandy loam / sha zhi rang tu
        elif sand <= 90:
            return [2, 4, 0.04]  # loamy sand / rang zhi sha tu
        else:
            return [1, 4, 0.02]  # sand / sha tu

    @staticmethod
    def lookup_soil_parameters(soil_lookup_file):
        """Reclassify soil parameters by lookup table.

        Returns:
            recls_dict: dict, e.g., {'OM': '201:1.3|1.2|0.6,202:1.4|1.1|0.8'}
        """
        #  Read soil properties from txt file
        soil_lookup_data = read_data_items_from_txt(soil_lookup_file)
        soil_instances = list()
        soil_prop_flds = soil_lookup_data[0][:]

        def find_and_set_attr(obj, attr, value):
            if hasattr(obj, attr):
                setattr(obj, attr, value)

        for i in range(1, len(soil_lookup_data)):
            cur_soil_data_item = soil_lookup_data[i][:]
            cur_seqn = cur_soil_data_item[0]
            cur_sname = cur_soil_data_item[1]
            cur_soil_ins = SoilProperty(cur_seqn, cur_sname)
            for j in range(2, len(soil_prop_flds)):
                cur_flds = StringClass.split_string(cur_soil_data_item[j], '-')  # Get field values
                for k, tmpfld in enumerate(cur_flds):
                    cur_flds[k] = float(tmpfld)  # Convert to float
                if len(cur_flds) == 1:  # e.g., SOL_ZMX
                    cur_flds = cur_flds[0]
                find_and_set_attr(cur_soil_ins, soil_prop_flds[j], cur_flds)
                # special cases
                cur_soil_ins.SOILLAYERS = int(cur_soil_ins.SOILLAYERS)

            cur_soil_ins.check_data_validation()
            soil_instances.append(cur_soil_ins)

        soil_prop_dict = dict()
        for sol in soil_instances:
            cur_sol_dict = sol.soil_dict()
            for fld in cur_sol_dict:
                if fld in soil_prop_dict:
                    soil_prop_dict[fld].append(cur_sol_dict[fld])
                else:
                    soil_prop_dict[fld] = [cur_sol_dict[fld]]

        recls_dict = dict()
        seqns = soil_prop_dict[SoilUtilClass._SEQN]
        for key in soil_prop_dict:
            if key == SoilUtilClass._SEQN or key == SoilUtilClass._NAME:
                continue
            key_l = 1  # maximum layer number
            for key_v in soil_prop_dict[key]:
                if isinstance(key_v, list):
                    if len(key_v) > key_l:
                        key_l = len(key_v)
            cur_dict = dict()
            for i, tmpseq in enumerate(seqns):
                cur_dict[tmpseq] = [DEFAULT_NODATA] * key_l
                if key_l == 1:
                    cur_dict[tmpseq][0] = soil_prop_dict[key][i]
                    continue
                for j in range(soil_prop_dict[SoilUtilClass._NLYRS][i]):
                    cur_dict[tmpseq][j] = soil_prop_dict[key][i][j]
            recls_dict[key] = ','.join('%s:%s' % (k, '|'.join(repr(vv) for vv in v))
                                       for k, v in cur_dict.items())
        return recls_dict

    @staticmethod
    def initial_soil_moisture(acc_file, slope_file, out_file):
        """Initialize soil moisture fraction of field capacity, based on TWI"""
        acc_r = RasterUtilClass.read_raster(acc_file)
        data_acc = acc_r.data
        xsize = acc_r.nCols
        ysize = acc_r.nRows
        nodata_value = acc_r.noDataValue
        srs = acc_r.srs
        geotrans = acc_r.geotrans
        dx = acc_r.dx
        data_slope = RasterUtilClass.read_raster(slope_file).data
        cell_area = dx * dx

        def wi_grid_cal(accvalue, slpvalue):
            """TWI, ln(acc_file/tan(slp))"""
            if abs(accvalue - nodata_value) < UTIL_ZERO:
                return DEFAULT_NODATA
            else:
                if abs(slpvalue) < MINI_SLOPE:
                    slpvalue = MINI_SLOPE
                return math.log((accvalue + 1.) * cell_area / slpvalue)

        wi_grid_cal_numpy = numpy.frompyfunc(wi_grid_cal, 2, 1)
        wi_grid = wi_grid_cal_numpy(data_acc, data_slope)
        # wiGrid_valid = numpy.where(acc_r.validZone, wi_grid, numpy.nan)
        # wi_max = numpy.nanmax(wiGrid_valid)
        # wi_min = numpy.nanmin(wiGrid_valid)
        # WARNING: numpy.nanmax and numpy.nanmin are un-stable in Linux, so
        # replaced by the for loops. By LJ
        wi_max = -numpy.inf
        wi_min = numpy.inf
        for i in range(0, ysize):
            for j in range(0, xsize):
                if wi_max < wi_grid[i][j]:
                    wi_max = wi_grid[i][j]
                if DEFAULT_NODATA != wi_grid[i][j] < wi_min:
                    wi_min = wi_grid[i][j]
        # print('TWIMax:%f, TWIMin:%f' % (wi_max, wi_min))
        soil_mois_fr_min = 0.6  # minimum relative saturation
        soil_mois_fr_max = 1.0

        wi_uplimit = wi_max
        a = (soil_mois_fr_max - soil_mois_fr_min) / (wi_uplimit - wi_min)
        b = soil_mois_fr_min - a * wi_min

        def moisture_cal(acc, wigrid):
            """calculate soil moisture"""
            if abs(acc - nodata_value) < UTIL_ZERO:
                return DEFAULT_NODATA
            else:
                tmp = a * wigrid + b
                if tmp > soil_mois_fr_max:
                    return soil_mois_fr_max
                elif tmp < soil_mois_fr_min:
                    return soil_mois_fr_min
                else:
                    return tmp

        moisture_cal_numpy = numpy.frompyfunc(moisture_cal, 2, 1)
        moisture = moisture_cal_numpy(data_acc, wi_grid)
        RasterUtilClass.write_gtiff_file(out_file, ysize, xsize, moisture, geotrans, srs,
                                         DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def parameters_extraction(cfg):
        """Soil spatial parameters extraction."""
        f = cfg.logs.extract_soil
        status_output('Calculating initial soil physical and chemical parameters...', 30, f)
        recls_dict = SoilUtilClass.lookup_soil_parameters(cfg.soil_property)

        status_output('Decomposing to MongoDB and exclude nodata values to save space...', 50, f)
        inoutcfg = list()
        for k, v in recls_dict.items():
            inoutcfg.append([cfg.spatials.soil_type, k,
                             DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE', v])
        mongoargs = [cfg.hostname, cfg.port, cfg.spatial_db, 'SPATIAL']
        mask_rasterio(cfg.seims_bin, inoutcfg, mongoargs=mongoargs,
                      maskfile=cfg.spatials.subbsn, cfgfile=cfg.logs.reclasssoil_cfg,
                      include_nodata=False, mode='MASKDEC')

        # other soil related spatial parameters
        status_output('Calculating initial soil moisture...', 90, f)
        SoilUtilClass.initial_soil_moisture(cfg.spatials.d8acc, cfg.spatials.slope,
                                            cfg.spatials.init_somo)

        status_output('Soil related spatial parameters extracted done!', 100, f)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    SoilUtilClass.parameters_extraction(seims_cfg)


if __name__ == "__main__":
    main()
