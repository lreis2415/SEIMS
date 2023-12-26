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

from preprocess.sp_soil_base import SoilPropertyBase
from preprocess.sp_soil_utils import SoilUtilClass
from preprocess.text import ParamAbstractionTypes

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import numpy

from utility import DEFAULT_NODATA, UTIL_ZERO


class SoilPropertyPhysical(SoilPropertyBase):
    """
    base class of Soil physical and general chemical properties
    Attributes:

    """

    def __init__(self, seq_num, seq_name):
        """
        Initialize a soil property object.
        Args:
            seq_num (int): Soil sequence number, Unique identifier
            seq_name (str): The corresponding soil name
        """
        super().__init__(seq_num, seq_name)

    @staticmethod
    def soil_param_type():
        return ParamAbstractionTypes.PHYSICAL

    def _add_default_septic_layer(self, dep_new=10.):
        """
        For every `list` type soil property,
        add a septic layer at the top of soil profile whose value is the same as the first layer.
        """
        self.SOILLAYERS += 1
        for k, v in self.__dict__.items():
            if isinstance(v, list) and v:
                v.insert(0, v[0])
        self.SOL_Z[0] = dep_new

    def _add_septic_layer(self):
        # set a soil layer at dep_new and adjust all lower layers
        # add a septic layer:0-10mm, refers to layersplit.f in SWAT
        septic_depth = 10.
        if self.SOL_Z[0] < 2 * septic_depth:
            return
        self.check_layers(self.SOL_OM, is_required=True, nodata_allowed=False)
        self.check_layers(self.SOL_CLAY, is_required=True, nodata_allowed=False)
        self.check_layers(self.SOL_SILT, is_required=True, nodata_allowed=False)
        self.check_layers(self.SOL_SAND, is_required=True, nodata_allowed=False)
        self.check_layers(self.SOL_ROCK, is_required=True, nodata_allowed=False)
        self._add_default_septic_layer(septic_depth)

        if self.SOL_AWC:
            for i in range(self.SOILLAYERS):
                if self.SOL_AWC[i] <= 0.:
                    self.SOL_AWC[i] = 0.005
                elif self.SOL_AWC[i] <= 0.01:
                    self.SOL_AWC[i] = 0.01
                elif self.SOL_AWC[i] >= 0.8:
                    self.SOL_AWC[i] = 0.8

        if not self.SOL_NO3:
            self.SOL_NO3 = list(numpy.zeros(self.SOILLAYERS))
        if not self.SOL_NH4:
            self.SOL_NH4 = list(numpy.zeros(self.SOILLAYERS))
        if not self.SOL_ORGN:
            self.SOL_ORGN = list(numpy.zeros(self.SOILLAYERS))
        if not self.SOL_SOLP:
            self.SOL_SOLP = list(numpy.zeros(self.SOILLAYERS))
        if not self.SOL_ORGP:
            self.SOL_ORGP = list(numpy.zeros(self.SOILLAYERS))

    def _pre_construct(self, *args, **kwargs):
        """
        Pre-construct the soil parameters.
        args:
            *args:
            **kwargs:
                has_septic_layer (bool): Whether to add a septic layer at the top of soil profile.
        """
        if not kwargs.get('has_septic_layer', False):
            return
        self._add_septic_layer()

    def _construct(self):
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
        if self.SOL_AWC and len(self.SOL_AWC) != self.SOILLAYERS:
            raise IndexError("Available water capacity must have the size equal to"
                             " soil layers number!")
        elif not self.SOL_AWC:
            for i in range(self.SOILLAYERS):
                self.SOL_AWC.append(self.SOL_FC[i] - self.SOL_WP[i])
                self.SOL_AWC_AMOUNT.append(self.SOL_AWC[i] * self.SOILTHICK[i])
        elif DEFAULT_NODATA in self.SOL_AWC:
            for i in range(self.SOILLAYERS):
                if self.SOL_AWC[i] == DEFAULT_NODATA:
                    self.SOL_AWC[i] = self.SOL_FC[i] - self.SOL_WP[i]
                    self.SOL_AWC_AMOUNT[i] = self.SOL_AWC[i] * self.SOILTHICK[i]

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
        if not self.SOL_K or len(self.SOL_K) != self.SOILLAYERS:
            raise IndexError("Saturated conductivity is required, and should have a size equal to soil layers!")
            # raise IndexError("Saturated conductivity must have a size equal to soil layers number!")
        # elif not self.SOL_K or DEFAULT_NODATA in self.SOL_K:
        #     tmp_k = list()
        #     for i in range(self.SOILLAYERS):
        #         lamda = self.POREINDEX[i]
        #         fc = tmp_fc[i]
        #         sat = tmp_sat[i]
        #         tmp_k.append(1930. * pow(sat - fc, 3. - lamda))
        #     if not self.SOL_K:
        #         self.SOL_K     = tmp_k[:]
        #     elif DEFAULT_NODATA in self.SOL_K:
        #         for i in range(self.SOILLAYERS):
        #             if self.SOL_K[i] == DEFAULT_NODATA:
        #                 self.SOL_K[i] = tmp_k[i]
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
                self.SOL_FC.append(self.SOL_WP[i] + self.SOL_AWC[i])
        # calculate porosity
        if not self.SOL_POROSITY:
            for i in range(self.SOILLAYERS):
                self.SOL_POROSITY.append(1. - self.SOL_BD[i] / 2.65)
        if self.SOL_CRK == DEFAULT_NODATA:
            self.SOL_CRK = numpy.mean(self.SOL_POROSITY)
        for i in range(self.SOILLAYERS):
            if self.SOL_FC[i] >= self.SOL_POROSITY[i]:
                self.SOL_FC[i] = self.SOL_POROSITY[i] - 0.05
                self.SOL_WP[i] = self.SOL_FC[i] - self.SOL_AWC[i]
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
            self.SOL_AWC_AMOUNT.append((self.SOL_FC[i] - self.SOL_WP[i]) * self.SOILTHICK[i])
            self.SOL_SUMAWC += self.SOL_AWC_AMOUNT[i]
            self.SOL_HK.append((self.SOL_UL[i] - self.SOL_AWC_AMOUNT[i]) / self.SOL_K[i])
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
                tmp_usle_k.append(SoilUtilClass.usle_k_epic(sand, silt, clay, om))
            if not self.USLE_K:
                self.USLE_K = tmp_usle_k[:]
            elif DEFAULT_NODATA in self.USLE_K:
                for i in range(self.SOILLAYERS):
                    if self.USLE_K[i] == DEFAULT_NODATA:
                        self.USLE_K[i] = tmp_usle_k[i]
        if self.SOIL_TEXTURE == DEFAULT_NODATA or self.HYDRO_GROUP == DEFAULT_NODATA:
            st, hg, uslek = SoilUtilClass.get_soil_texture_usda(self.SOL_CLAY[0], self.SOL_SILT[0],
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


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    SoilUtilClass.parameters_extraction(seims_cfg, SoilPropertyPhysical)


if __name__ == "__main__":
    main()
