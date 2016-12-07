#! /usr/bin/env python
# coding=utf-8
#
# @Author: Junzhi Liu, 2013-1-10
# @Revised: Liang-Jun Zhu, Huiran Gao, Fang Shen
# @Revised date: 2016-7-22
# @Note: 1. Names and units of soil physical parameter are referred to readsol.f, soil_par.f, and soil_phys.f in SWAT
#        2. Data validation checking is also conducted here.
# 3. Basic protocols: a. all names are Capitalized. b. output Geotiff
# names may be appended in text.py

from soil_texture import *
from text import *
from util import *


# SEQN              |None  : Unique identifier of soil
# SNAME             |None  : soil name
# SOILLAYERS        |None  : (nly) number of soil layers
# SOILDEPTH         |mm    : (sol_z) depth from the surface to bottom of soil layer
# SOILTHICK         |mm    : soil thickness for calculation convenient
# OM                |%     : organic matter content (weight percent)
# SOL_CBN           |%     : (sol_cbn) percent organic carbon in soil layer
# SOL_N             |%     : (sol_n) used when using CSWAT = 1, i.e, C-FARM one carbon pool model
# CLAY              |%     : (sol_clay) percent clay content in soil material,diameter < 0.002 mm
# SILT              |%     : (sol_silt) percent silt content in soil material,diameter between 0.002 mm and 0.05 mm
# SAND              |%     : (sol_sand) percent sand content in soil material,diameter between 0.05 mm and 2 mm
# ROCK              |%     : (sol_rock) percent of rock fragments content in soil material,diameter > 2 mm
# SOIL_TEXTURE      |None  : soil texture
# HYDRO_GROUP       |None  : Hydrological group, 1,2,3,and 4 to represent A,B,C,and D
# SOL_ZMX           |mm    : (sol_zmx) maximum rooting depth of soil profile
# ANION_EXCL        |None  : (anion_excl) fraction of porosity from which anions are excluded,default is 0.5
# SOL_CRK           |None  : (sol_crk) crack volume potential of soil expressed as a fraction of the total soil volume
# DENSITY           |Mg/m3 or g/cm3: (sol_bd) bulk density of each soil layer
# SOL_AVBD          |Mg/m3 or g/cm3: (sol_avbd) average bulk density for soil profile
# CONDUCTIVITY      |mm/hr : (sol_k) saturated hydraulic conductivity
# SOL_HK            |None  : (sol_hk) beta coefficent to calculate hydraulic conductivity
# WILTINGPOINT      |mm H2O / mm soil: (sol_wp) water content of soil at -1.5 MPa (wilting point)
# SOL_WPMM          |mm H2O: (sol_wpmm) water content of soil at -1.5 MPa (wilting point)
# SOL_SUMWP         |mm H2O: (sol_sumwp) amount of water held in the soil profile at wilting point
# FIELDCAP          |mm H2O / mm soil  : (sol_up) water content of soil at -0.033 MPa (field capacity)
# AWC               |mm H2O / mm soil : (sol_awc) available water capacity of soil layer
# SOL_AWC           |mm H2O: (sol_fc) amount of water available to plants in soil layer at field capacity (fc - wp)
# SOL_SUMAWC        |mm H2O: (sol_sumfc) amount of water held in soil profile at field capacity
# POROSITY          |None  : (sol_por) total porosity of soil layer expressed as a fraction of the total volume
# POREINDEX         |None  : pore size distribution index
# SOL_AVPOR         |None  : (sol_avpor) average porosity for entire soil profile
# SOL_UL            |mm H2O: (sol_ul) amount of water held in the soil layer at saturation (sat - wp water)
# SOL_SUMUL         |mm H2O: (sol_sumul) amount of water held in soil profile at saturation
# USLE_K            |None  :  USLE K factor
# SOL_ALB           |None  : albedo of top soil surface
# WFSH              |mm    : wetting front matric potential (usde in Green-Ampt method)
# ESCO              |None  : soil evaporation compensation factor
# VWT               |None  : (vwt) variable water table factor, used in percolation modules
# DET_SAND          |None  : (det_san) detached sediment size distribution, sand fraction
# DET_SILT          |None  : (det_sil) detached sediment size distribution, silt fraction
# DET_CLAY          |None  : (det_cla) detached sediment size distribution, clay fraction
# DET_SMAGG         |None  : (det_sag) detached sediment size distribution, small aggregation fraction
# DET_LGAGG         |None  : (det_lag) detached sediment size distribution, large aggregation fraction
# CRDEP             |mm    : (crdep) maximum or potential crack volume
# VOLCR             |mm    : (volcr) crack volume for soil layer, should
# be calculated in SEIMS, using moist_ini

# SOL_NO3           |kg/ha : (sol_no3) concentration of nitrate in soil layers
# SOL_NH4           |kg/ha : (sol_nh4) concentration of ammonium-N in soil layers
# SOL_ORGN          |kg/ha : (sol_orgn) organic N concentration in soil layers
# SOL_ORGP          |kg/ha : (sol_orgp) organic P concentration in soil layers
# SOL_SOLP          |kg/ha : (sol_solp) soluble P concentration in soil layers
class SoilProperty:
    '''
    base class of Soil physical and general chemical properties
    :method: init(SEQN, SNAM)
    '''

    def __init__(self, SEQN, SNAM):
        self.SEQN = SEQN
        self.SNAM = SNAM
        self.SOILLAYERS = DEFAULT_NODATA
        self.SOILDEPTH = []
        self.SOILTHICK = []
        self.OM = []
        self.SOL_CBN = []
        self.SOL_N = []
        self.CLAY = []
        self.SILT = []
        self.SAND = []
        self.ROCK = []
        self.SOIL_TEXTURE = DEFAULT_NODATA
        self.HYDRO_GROUP = DEFAULT_NODATA
        self.SOL_ZMX = DEFAULT_NODATA
        self.ANION_EXCL = DEFAULT_NODATA
        self.SOL_CRK = DEFAULT_NODATA
        self.DENSITY = []
        self.SOL_AVBD = []
        self.CONDUCTIVITY = []
        self.SOL_HK = []
        self.WILTINGPOINT = []
        self.SOL_WPMM = []
        self.SOL_SUMWP = 0.
        self.FIELDCAP = []
        self.AWC = []
        self.SOL_AWC = []
        self.SOL_SUMAWC = 0.
        self.POROSITY = []
        self.POREINDEX = []
        self.SOL_AVPOR = DEFAULT_NODATA
        self.SOL_UL = []
        self.SOL_SUMUL = 0.
        self.USLE_K = []
        self.SOL_ALB = DEFAULT_NODATA
        self.WFSH = DEFAULT_NODATA
        self.VWT = []
        self.DET_SAND = DEFAULT_NODATA
        self.DET_SILT = DEFAULT_NODATA
        self.DET_CLAY = DEFAULT_NODATA
        self.DET_SMAGG = DEFAULT_NODATA
        self.DET_LGAGG = DEFAULT_NODATA
        self.CRDEP = []
        self.ESCO = DEFAULT_NODATA
        # Here after are general soil chemical properties
        self.SOL_NO3 = []
        self.SOL_NH4 = []
        self.SOL_ORGN = []
        self.SOL_ORGP = []
        self.SOL_SOLP = []

    def SoilDict(self):
        solDict = self.__dict__
        solDict.pop(SOL_NAME)
        # remove the empty element
        for ele in solDict.keys():
            if solDict[ele] == []:
                solDict.pop(ele)
        # print solDict
        return solDict

    def CheckData(self):  # check the required input, and calculate all physical and general chemical properties
        # set a soil layer at dep_new and adjust all lower layers
        # a septic layer:0-10mm,accordig to swat layersplit.f
        if self.SOILLAYERS == DEFAULT_NODATA:
            raise ValueError(
                "Soil layers number must be provided, please check the input file!")
        dep_new = 10.
        if self.SOILDEPTH[0] - dep_new >= 10.:
            self.SOILLAYERS += 1
            self.SOILDEPTH.insert(0, dep_new)
            if self.OM != []:
                self.OM.insert(0, self.OM[0])
            else:
                raise ValueError("Organic matter must be provided!")
            if self.CLAY != []:
                self.CLAY.insert(0, self.CLAY[0])
            else:
                raise ValueError("Clay content must be provided!")
            if self.SILT != []:
                self.SILT.insert(0, self.SILT[0])
            else:
                raise ValueError("Silt content must be provided!")
            if self.SAND != []:
                self.SAND.insert(0, self.SAND[0])
            else:
                raise ValueError("Sand content must be provided!")
            if self.ROCK != []:
                self.ROCK.insert(0, self.ROCK[0])
            else:
                raise ValueError("Rock content must be provided!")
            if self.FIELDCAP != []:
                self.FIELDCAP.insert(0, self.FIELDCAP[0])
            else:
                raise ValueError("Available water capacity must be provided!")
            if self.DENSITY != []:
                self.DENSITY.insert(0, self.DENSITY[0])
            else:
                raise ValueError("Bulk density must be provided!")
            if self.CONDUCTIVITY != []:
                self.CONDUCTIVITY.insert(0, self.CONDUCTIVITY[0])
            if self.WILTINGPOINT != []:
                self.WILTINGPOINT.insert(0, self.WILTINGPOINT[0])
            if self.AWC != []:
                self.AWC.insert(0, self.AWC[0])
                for i in range(self.SOILLAYERS):
                    if self.AWC[i] <= 0.:
                        self.AWC[i] = 0.005
                    elif self.AWC[i] <= 0.01:
                        self.AWC[i] = 0.01
                    elif self.AWC[i] >= 0.8:
                        self.AWC[i] = 0.8
            if self.POROSITY != []:
                self.POROSITY.insert(0, self.POROSITY[0])
            if self.USLE_K != []:
                self.USLE_K.insert(0, self.USLE_K[0])
            if self.SOL_NO3 != []:
                self.SOL_NO3.insert(0, self.SOL_NO3[0])
            else:
                self.SOL_NO3 = list(numpy.zeros(self.SOILLAYERS))
            if self.SOL_NH4 != []:
                self.SOL_NH4.insert(0, self.SOL_NH4[0])
            else:
                self.SOL_NH4 = list(numpy.zeros(self.SOILLAYERS))
            if self.SOL_ORGN != []:
                self.SOL_ORGN.insert(0, self.SOL_ORGN[0])
            else:
                self.SOL_ORGN = list(numpy.zeros(self.SOILLAYERS))
            if self.SOL_SOLP != []:
                self.SOL_SOLP.insert(0, self.SOL_SOLP[0])
            else:
                self.SOL_SOLP = list(numpy.zeros(self.SOILLAYERS))
            if self.SOL_ORGP != []:
                self.SOL_ORGP.insert(0, self.SOL_ORGP[0])
            else:
                self.SOL_ORGP = list(numpy.zeros(self.SOILLAYERS))
        if self.SOILDEPTH == [] or len(self.SOILDEPTH) != self.SOILLAYERS or DEFAULT_NODATA in self.SOILDEPTH:
            raise IndexError(
                "Soil depth must have a size equal to NLAYERS and should not include NODATA (-9999)!")
        # Calculate soil thickness of each layer
        for l in range(self.SOILLAYERS):
            if l == 0:
                self.SOILTHICK.append(self.SOILDEPTH[l])
            else:
                self.SOILTHICK.append(
                    self.SOILDEPTH[l] - self.SOILDEPTH[l - 1])
        if self.SOL_ZMX == DEFAULT_NODATA or self.SOL_ZMX > self.SOILDEPTH[-1]:
            self.SOL_ZMX = self.SOILDEPTH[-1]
        if self.ANION_EXCL == DEFAULT_NODATA:
            self.ANION_EXCL = 0.5
        if self.OM == [] or len(self.OM) != self.SOILLAYERS:
            raise IndexError(
                "Soil organic matter must have a size equal to NLAYERS!")
        elif DEFAULT_NODATA in self.OM and self.OM.index(DEFAULT_NODATA) >= 2 and self.SOILLAYERS >= 3:
            for i in range(2, self.SOILLAYERS):
                if self.OM[i] == DEFAULT_NODATA:
                    self.OM[i] = self.OM[i - 1] * \
                        numpy.exp(-self.SOILTHICK[i])  # mm
        # Calculate sol_cbn = sol_om * 0.58
        if self.SOL_CBN == [] or len(self.SOL_CBN) != self.SOILLAYERS:
            self.SOL_CBN = []
            for i in range(self.SOILLAYERS):
                if self.OM[i] * 0.58 < UTIL_ZERO:
                    self.SOL_CBN.append(0.1)
                else:
                    self.SOL_CBN.append(self.OM[i] * 0.58)
        # Calculate sol_n = sol_cbn/11.
        if self.SOL_N == [] or len(self.SOL_N) != self.SOILLAYERS:
            self.SOL_N = []
            for i in range(self.SOILLAYERS):
                self.SOL_N.append(self.SOL_CBN[i] / 11.)
        if self.CLAY == [] or len(self.CLAY) != self.SOILLAYERS or DEFAULT_NODATA in self.CLAY:
            raise IndexError(
                "Clay content must have a size equal to NLAYERS and should not include NODATA (-9999)!")
        if self.SILT == [] or len(self.SILT) != self.SOILLAYERS or DEFAULT_NODATA in self.SILT:
            raise IndexError(
                "Silt content must have a size equal to NLAYERS and should not include NODATA (-9999)!")
        if self.SAND == [] or len(self.SAND) != self.SOILLAYERS or DEFAULT_NODATA in self.SAND:
            raise IndexError(
                "Sand content must have a size equal to NLAYERS and should not include NODATA (-9999)!")
        if self.ROCK == [] or len(self.ROCK) != self.SOILLAYERS or DEFAULT_NODATA in self.ROCK:
            raise IndexError(
                "Rock content must have a size equal to NLAYERS and should not include NODATA (-9999)!")

        # temperory variables
        tmp_fc = []
        tmp_sat = []
        tmp_wp = []
        for i in range(self.SOILLAYERS):
            s = self.SAND[i] * 0.01  # % -> decimal
            c = self.CLAY[i] * 0.01
            om = self.OM[i]
            wpt = -0.024 * s + 0.487 * c + 0.006 * om + 0.005 * \
                s * om - 0.013 * c * om + 0.068 * s * c + 0.031
            tmp_wp.append(1.14 * wpt - 0.02)
            fct = -0.251 * s + 0.195 * c + 0.011 * om + 0.006 * \
                s * om - 0.027 * c * om + 0.452 * s * c + 0.299
            fc = fct + 1.283 * fct * fct - 0.374 * fct - 0.015
            s33t = 0.278 * s + 0.034 * c + 0.022 * om - 0.018 * \
                s * om - 0.027 * c * om - 0.584 * s * c + 0.078
            s33 = 1.636 * s33t - 0.107
            sat = fc + s33 - 0.097 * s + 0.043
            tmp_fc.append(fc)
            tmp_sat.append(sat)

        if self.WILTINGPOINT != [] and len(self.WILTINGPOINT) != self.SOILLAYERS:
            raise IndexError(
                "Wilting point must have a size equal to soil layers number!")
        elif self.WILTINGPOINT == []:
            self.WILTINGPOINT = tmp_wp[:]
        elif DEFAULT_NODATA in self.WILTINGPOINT:
            for i in range(self.SOILLAYERS):
                if self.WILTINGPOINT[i] == DEFAULT_NODATA:
                    self.WILTINGPOINT[i] = tmp_wp[i]
        if self.DENSITY != [] and len(self.DENSITY) != self.SOILLAYERS:
            raise IndexError(
                "Bulk density must have a size equal to soil layers number!")
        elif self.DENSITY == [] or DEFAULT_NODATA in self.DENSITY:
            tmp_bd = []
            for i in range(self.SOILLAYERS):
                sat = tmp_sat[i]
                fc = tmp_fc[i]
                if self.FIELDCAP != [] and len(self.FIELDCAP) == self.SOILLAYERS:
                    sat = sat - fc + self.FIELDCAP[i]
                tmp_bd.append(2.65 * (1.0 - sat))
            if DEFAULT_NODATA in self.DENSITY:
                for i in range(self.SOILLAYERS):
                    if self.DENSITY[i] == DEFAULT_NODATA:
                        self.DENSITY[i] = tmp_bd[i]
            elif self.DENSITY == []:
                self.DENSITY = tmp_bd[:]
        if self.FIELDCAP != [] and len(self.FIELDCAP) != self.SOILLAYERS:
            raise IndexError(
                "Field capacity must have a size equal to soil layers number!")
        elif self.FIELDCAP == [] or DEFAULT_NODATA in self.FIELDCAP:
            tmp_fc_bdeffect = []
            for i in range(self.SOILLAYERS):
                fc = tmp_fc[i]
                sat = tmp_sat[i]
                if self.DENSITY != [] and len(self.DENSITY) == self.SOILLAYERS:
                    p_df = self.DENSITY[i]
                else:
                    p_df = 2.65 * (1.0 - sat)
                sat_df = 1. - p_df / 2.65
                tmp_fc_bdeffect.append(fc - 0.2 * (sat - sat_df))
            if DEFAULT_NODATA in self.FIELDCAP:
                for i in range(self.SOILLAYERS):
                    if self.FIELDCAP[i] == DEFAULT_NODATA:
                        self.FIELDCAP[i] = tmp_fc_bdeffect[i]
            elif self.FIELDCAP == []:
                self.FIELDCAP = tmp_fc_bdeffect[:]
        if self.AWC != [] and len(self.AWC) != self.SOILLAYERS:
            raise IndexError(
                "Available water capacity must have the size equal to soil layers number!")
        elif self.AWC == []:
            for i in range(self.SOILLAYERS):
                self.AWC.append(self.FIELDCAP[i] - self.WILTINGPOINT[i])
        elif DEFAULT_NODATA in self.AWC:
            for i in range(self.SOILLAYERS):
                if self.AWC[i] == DEFAULT_NODATA:
                    self.AWC[i] = self.FIELDCAP[i] - self.WILTINGPOINT[i]

        if self.POREINDEX != [] and len(self.POREINDEX) != self.SOILLAYERS:
            raise IndexError(
                "Pore disconnectedness index must have a size equal to soil layers number!")
        elif self.POREINDEX == []:
            for i in range(self.SOILLAYERS):
                # An fitted equation proposed by Cosby et al. (1984) is
                # adopted. By LJ, 2016-9-22
                b = 0.159 * self.CLAY[i] + 2.91
                self.POREINDEX.append(b)
                # previous version, currently deprecated by LJ
                # fc = self.FIELDCAP[i]
                # wp = self.WILTINGPOINT[i]
                # b = (math.log(1500.) - math.log(33.)) / (math.log(fc) - math.log(wp))
                # self.POREINDEX.append(1.0 / b)
        if self.POROSITY != [] and len(self.POROSITY) != self.SOILLAYERS:
            raise IndexError(
                "Soil Porosity must have a size equal to soil layers number!")
        elif self.POROSITY == []:
            for i in range(self.SOILLAYERS):
                # from the theroy of swat
                self.POROSITY.append(1. - self.DENSITY[i] / 2.65)
        elif DEFAULT_NODATA in self.POROSITY:
            for i in range(self.SOILLAYERS):
                if self.POROSITY[i] == DEFAULT_NODATA:
                    self.POROSITY[i] = 1. - self.DENSITY[i] / 2.65
        if self.CONDUCTIVITY != [] and len(self.CONDUCTIVITY) != self.SOILLAYERS:
            raise IndexError(
                "Saturated conductivity must have a size equal to soil layers number!")
        elif self.CONDUCTIVITY == [] or DEFAULT_NODATA in self.CONDUCTIVITY:
            tmp_k = []
            for i in range(self.SOILLAYERS):
                lamda = self.POREINDEX[i]
                fc = tmp_fc[i]
                sat = tmp_sat[i]
                tmp_k.append(1930. * pow(sat - fc, 3. - lamda))
            if self.CONDUCTIVITY == []:
                self.CONDUCTIVITY = tmp_k[:]
            elif DEFAULT_NODATA in self.CONDUCTIVITY:
                for i in range(self.SOILLAYERS):
                    if self.CONDUCTIVITY[i] == DEFAULT_NODATA:
                        self.CONDUCTIVITY[i] = tmp_k[i]

        # calculate water content of soil at -1.5 MPa and -0.033 MPa
        # (soil_phys.f)
        if self.WILTINGPOINT == []:
            for i in range(self.SOILLAYERS):
                tmpwp = 0.4 * self.CLAY[i] * self.DENSITY[i] / 100.
                if tmpwp <= 0.:
                    tmpwp = 0.005
                self.WILTINGPOINT.append(tmpwp)
        # calculate field capcity (sol_up)
        if self.FIELDCAP == []:
            for i in range(self.SOILLAYERS):
                self.FIELDCAP.append(self.WILTINGPOINT[i] + self.AWC[i])
        # calculate porosity
        if self.POROSITY == []:
            for i in range(self.SOILLAYERS):
                self.POROSITY.append(1. - self.DENSITY[i] / 2.65)
        if self.SOL_CRK == DEFAULT_NODATA:
            self.SOL_CRK = numpy.mean(self.POROSITY)
        for i in range(self.SOILLAYERS):
            if self.FIELDCAP[i] >= self.POROSITY[i]:
                self.FIELDCAP[i] = self.POROSITY[i] - 0.05
                self.WILTINGPOINT[i] = self.FIELDCAP[i] - self.AWC[i]
                if self.WILTINGPOINT[i] <= 0.:
                    self.FIELDCAP[i] = self.POROSITY[i] * 0.75
                    self.WILTINGPOINT[i] = self.POROSITY[i] * 0.25
            # compute drainable porosity and variable water table factor
            drpor = self.POROSITY[i] - self.FIELDCAP[i]
            self.VWT.append((437.13 * drpor * drpor) - (95.08 * drpor) + 8.257)
        sa = self.SAND[0] / 100.
        cl = self.CLAY[0] / 100.
        si = self.SILT[0] / 100.
        # determine detached sediment size distribution typical for mid-western soils in USA (Foster et al., 1980)
        # Based on SWRRB.
        self.DET_SAND = 2.49 * sa * (1. - cl)
        self.DET_SILT = 0.13 * si
        self.DET_CLAY = 0.2 * cl
        if cl < 0.25:
            self.DET_SMAGG = 2.0 * cl
        elif cl > 0.5:
            self.DET_SMAGG = 0.57
        else:
            self.DET_SMAGG = 0.28 * (cl - 0.25) + 0.5
        self.DET_LGAGG = 1. - self.DET_SAND - \
            self.DET_SILT - self.DET_CLAY - self.DET_SMAGG
        # Error check, may happen for soils with more sand. The fraction wont
        # add upto 1.0
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
            pormm = self.POROSITY[i] * self.SOILTHICK[i]
            sumpor += pormm
            self.SOL_UL.append(
                (self.POROSITY[i] - self.WILTINGPOINT[i]) * self.SOILTHICK[i])
            self.SOL_SUMUL += self.SOL_UL[i]
            self.SOL_AWC.append(
                (self.FIELDCAP[i] - self.WILTINGPOINT[i]) * self.SOILTHICK[i])
            self.SOL_SUMAWC += self.SOL_AWC[i]
            self.SOL_HK.append(
                (self.SOL_UL[i] - self.SOL_AWC[i]) / self.CONDUCTIVITY[i])
            if self.SOL_HK[i] < 1.:
                self.SOL_HK[i] = 1.
            self.SOL_WPMM.append(self.WILTINGPOINT[i] * self.SOILTHICK[i])
            self.SOL_SUMWP += self.SOL_WPMM[i]
            self.CRDEP.append(
                self.SOL_CRK * 0.916 * math.exp(-0.0012 * self.SOILDEPTH[i]) * self.SOILTHICK[i])

        self.SOL_AVPOR = sumpor / self.SOILDEPTH[i]
        self.SOL_AVBD = 2.65 * (1. - self.SOL_AVPOR)
        # calculate infiltration parameters for subdaily time step
        self.WFSH = 10. * math.exp(6.5309 - 7.32561 * self.POROSITY[0] + 3.809479 * math.pow(self.POROSITY[0], 2) +
                                   0.001583 * math.pow(self.CLAY[0], 2) + 0.000344 * self.SAND[0] * self.CLAY[
                                       0] - 0.049837 *
                                   self.POROSITY[0] * self.SAND[0] + 0.001608 * math.pow(self.POROSITY[0], 2) *
                                   math.pow(self.SAND[0], 2) + 0.001602 * math.pow(self.POROSITY[0], 2) * math.pow(
            self.CLAY[0], 2) -
            0.0000136 * math.pow(self.SAND[0], 2) * self.CLAY[0] - 0.003479 * math.pow(
            self.CLAY[0], 2) *
            self.POROSITY[0] - 0.000799 * math.pow(self.SAND[0], 2) * self.POROSITY[0])

        if self.SOL_ALB == DEFAULT_NODATA:
            self.SOL_ALB = 0.2227 * math.exp(-1.8672 * self.SOL_CBN[0])
        if self.ESCO == DEFAULT_NODATA:
            self.ESCO = 0.95
        if self.USLE_K != [] and len(self.USLE_K) != self.SOILLAYERS:
            raise IndexError(
                "USLE K factor must have a size equal to NLAYERS!")
        elif self.USLE_K == [] or DEFAULT_NODATA in self.USLE_K:
            tmp_usle_k = []
            for i in range(self.SOILLAYERS):  # According to Liu BY et al., (1999)
                sand = self.SAND[i]
                silt = self.SILT[i]
                clay = self.CLAY[i]
                cbn = self.OM[i] * 0.58
                sn = 1. - sand * 0.01
                a = (0.2 + 0.3 * math.exp(-0.0256 * sand * (1. - silt * 0.01)))
                b = math.pow(silt / (clay + silt), 0.3)
                c = (1. - 0.25 * cbn / (cbn + math.exp(3.72 - 2.95 * cbn)))
                d = (1. - 0.25 * sn / (sn + math.exp(-5.51 + 22.9 * sn)))
                k = a * b * c * d
                tmp_usle_k.append(k)
            if self.USLE_K == []:
                self.USLE_K = tmp_usle_k[:]
            elif DEFAULT_NODATA in self.USLE_K:
                for i in range(self.SOILLAYERS):
                    if self.USLE_K[i] == DEFAULT_NODATA:
                        self.USLE_K[i] = tmp_usle_k[i]
        if self.SOIL_TEXTURE == DEFAULT_NODATA or self.HYDRO_GROUP == DEFAULT_NODATA:
            st, hg, uslek = GetTexture(
                self.CLAY[0], self.SILT[0], self.SAND[0])
            self.SOIL_TEXTURE = st
            self.HYDRO_GROUP = hg
        # Unit conversion for general soil chemical properties
        wt1 = []
        for j in range(self.SOILLAYERS):
            # g/kg => kg/ha
            wt1.append(self.DENSITY[j] * self.SOILTHICK[j] * 10.)
        if self.SOL_NO3 != [] and len(self.SOL_NO3) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_NO3[j] = self.SOL_NO3[j] * wt1[j]
        if self.SOL_NH4 != [] and len(self.SOL_NH4) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_NH4[j] = self.SOL_NH4[j] * wt1[j]
        if self.SOL_ORGN != [] and len(self.SOL_ORGN) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_ORGN[j] = self.SOL_ORGN[j] * wt1[j]
        if self.SOL_SOLP != [] and len(self.SOL_SOLP) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_SOLP[j] = self.SOL_SOLP[j] * wt1[j]
        if self.SOL_ORGP != [] and len(self.SOL_ORGP) == self.SOILLAYERS:
            for j in range(self.SOILLAYERS):
                self.SOL_ORGP[j] = self.SOL_ORGP[j] * wt1[j]


# Calculate soil properties from sand, clay and organic matter.
# TODO, add reference.
def GetProperties(s, c, om):
    # wilting point (SOL_WP)
    wpt = -0.024 * s + 0.487 * c + 0.006 * om + 0.005 * \
        s * om - 0.013 * c * om + 0.068 * s * c + 0.031
    wp = 1.14 * wpt - 0.02

    # bulk density according to field capacity
    fct = -0.251 * s + 0.195 * c + 0.011 * om + 0.006 * \
        s * om - 0.027 * c * om + 0.452 * s * c + 0.299
    fc = fct + 1.283 * fct * fct - 0.374 * fct - 0.015

    s33t = 0.278 * s + 0.034 * c + 0.022 * om - 0.018 * \
        s * om - 0.027 * c * om - 0.584 * s * c + 0.078
    s33 = 1.636 * s33t - 0.107
    sat = fc + s33 - 0.097 * s + 0.043
    pn = 2.65 * (1.0 - sat)

    # field capacity (SOL_FC) with density effects (df)
    p_df = pn
    sat_df = 1. - p_df / 2.65  # porosity
    fc_df = fc - 0.2 * (sat - sat_df)

    # available water capacity (SOL_AWC)
    awc = fc_df - wp

    # pore disconnectedness index
    b = (math.log(1500.) - math.log(33.)) / (math.log(fc) - math.log(wp))
    lamda = 1.0 / b

    # saturated conductivity
    # print s, c, sat, fc, 3-lamda
    ks = 1930 * pow(sat - fc, 3. - lamda)

    # print wp, fc_df, awc,
    return wp, fc_df, sat_df, p_df, ks, lamda
    # "WILTINGPOINT", "FIELDCAP", "Porosity","DENSITY","CONDUCTIVITY", "POREINDEX"


def GetValue(geoMask, geoMap, data, i, j):
    # pGeo = Proj("+proj=longlat +ellps=krass +no_defs")
    # pAlbers = Proj("+proj=aea +ellps=krass +lon_0=105 +lat_0=0 +lat_1=25 +lat_2=47")
    # xMask = geoMask[0] + (j+0.5)*geoMask[1]
    # yMask = geoMask[3] + (i+0.5)*geoMask[5]
    # xMap, yMap = transform(pAlbers, pGeo, xMask, yMask)

    xMap = geoMask[0] + (j + 0.5) * geoMask[1]
    yMap = geoMask[3] + (i + 0.5) * geoMask[5]

    jMap = (xMap - geoMap[0]) / geoMap[1]
    iMap = (yMap - geoMap[3]) / geoMap[5]

    return data[iMap][jMap]
