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
import logging
import numpy as np
from osgeo import gdal
import rasterio

from preprocess.config import PreprocessConfig
from preprocess.sd_hru_aggregate import hru_rasterio

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import numpy
from osgeo.gdal import GDT_Float32
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import StringClass

from utility import DEFAULT_NODATA, UTIL_ZERO, MINI_SLOPE
from utility import read_data_items_from_txt
from utility import mask_rasterio
from preprocess.text import ParamAbstractionTypes

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
        """
        self.SEQN = seq_num
        self.SNAM = seq_name
        self.SOILLAYERS = DEFAULT_NODATA
        self.SOILDEPTH = list()
        self.SOL_Z = list()
        self.SOILTHICK = list()

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
            if isinstance(sol_dict[ele], list) and not sol_dict[ele]:
                # del sol_dict[ele]
                remove_indices.append(ele)
        for ele in remove_indices:
            del sol_dict[ele]
        return sol_dict

    def _pre_construct(self):
        pass

    def _construct(self):
        pass

    def construct(self):
        if self.SOILLAYERS == DEFAULT_NODATA:
            raise ValueError("Soil layer numbers is REQUIRED, please check the input file!")

        self._pre_construct()

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


class SoilUtilClass(object):

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def _reclassify_missing_soil_types_in_lookup(original_soil_file, workspace_soil_file, soil_types_in_lookup, default_soil_seq):
        """Reclassify missing soil types in lookup table to default soil type."""
        with rasterio.open(original_soil_file, 'r') as src_origin:
            data = src_origin.read(1)
            unique_values = np.unique(data)
            missing_values = set(unique_values) - set(soil_types_in_lookup) - {src_origin.nodata}
            if not missing_values:
                return
            if default_soil_seq is None or default_soil_seq not in soil_types_in_lookup:
                default_soil_seq = soil_types_in_lookup[0]
                logging.info('Default soil type is not specified or not in the lookup table, '
                      f'use the first soil type in the lookup table as default soil type {default_soil_seq}.')
        with rasterio.open(workspace_soil_file, 'r+') as src_workspace:
            data = src_workspace.read(1)
            for missing_type in missing_values:
                data[data == missing_type] = default_soil_seq
            src_workspace.write(data, 1)
            logging.info(f'Missing soil types in lookup table: {missing_values} '
                  f'have been written as {default_soil_seq} into {workspace_soil_file}.')

    @staticmethod
    def _get_soil_property_instances(cfg, soil_property_class, soil_lookup_file):
        soil_lookup_data = read_data_items_from_txt(soil_lookup_file)
        soil_instances = list()
        soil_prop_flds = soil_lookup_data[0][:]

        if soil_property_class.soil_param_type() == ParamAbstractionTypes.PHYSICAL:
            workspace_soil_file = cfg.spatials.soil_type_physical
        else:
            workspace_soil_file = cfg.spatials.soil_type_conceptual
        SoilUtilClass._reclassify_missing_soil_types_in_lookup(cfg.soil,
                                                               workspace_soil_file,
                                                               [int(row[0]) for row in soil_lookup_data[1:]],
                                                               cfg.default_soil)

        for i in range(1, len(soil_lookup_data)):
            cur_soil_data_item = soil_lookup_data[i][:]
            cur_seqn = cur_soil_data_item[0]
            cur_sname = cur_soil_data_item[1]
            cur_soil_ins = soil_property_class(cur_seqn, cur_sname)
            for j in range(2, len(soil_prop_flds)):
                cur_flds = StringClass.split_string(cur_soil_data_item[j], '-')  # Get field values
                for k, tmpfld in enumerate(cur_flds):
                    cur_flds[k] = float(tmpfld)  # Convert to float
                if len(cur_flds) == 1:  # e.g., SOL_ZMX
                    cur_flds = cur_flds[0]
                cur_soil_ins.find_and_set_attr(soil_prop_flds[j], cur_flds)
                # special cases
                cur_soil_ins.SOILLAYERS = int(cur_soil_ins.SOILLAYERS)
            soil_dict_items = cur_soil_ins.soil_dict().items()
            if_size_equal = [len(v) == cur_soil_ins.SOILLAYERS
                             for k, v in soil_dict_items
                             if isinstance(v, list)]
            if if_size_equal.count(False) > 0:
                raise IndexError("Sizes of soil properties and soil layers must be equal!")

            cur_soil_ins.construct()
            soil_instances.append(cur_soil_ins)
        return soil_instances

    @staticmethod
    def lookup_soil_parameters(cfg, soil_property_class, soil_lookup_file):
        """Reclassify soil parameters by lookup table.

        Returns:
            recls_dict: dict, e.g., {'OM': '201:1.3|1.2|0.6,202:1.4|1.1|0.8'}
        """
        #  Read soil properties from txt file

        soil_instances = SoilUtilClass._get_soil_property_instances(cfg, soil_property_class, soil_lookup_file)

        soil_prop_dict = dict()
        for sol in soil_instances:
            cur_sol_dict = sol.soil_dict()
            for fld in cur_sol_dict:
                if fld in soil_prop_dict:
                    soil_prop_dict[fld].append(cur_sol_dict[fld])
                else:
                    soil_prop_dict[fld] = [cur_sol_dict[fld]]

        recls_dict = dict()
        seqns = soil_prop_dict[SoilPropertyBase.SEQN]
        for key in soil_prop_dict:
            if key == SoilPropertyBase.SEQN or key == SoilPropertyBase.SNAM:
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
                for j in range(soil_prop_dict[SoilPropertyBase.NLYRS][i]):
                    cur_dict[tmpseq][j] = soil_prop_dict[key][i][j]
            recls_dict[key] = ','.join('%s:%s' % (k, '|'.join(repr(vv) for vv in v))
                                       for k, v in cur_dict.items())
        return recls_dict

    @staticmethod
    def get_soil_texture_usda(clay, silt, sand):
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
    def parameters_extraction(cfg, soil_property_class):
        """Soil spatial parameters extraction."""
        # if conceptual
        soil_property_file = cfg.soil_property_conceptual
        inoutcfg_soilfile = cfg.spatials.soil_type_conceptual
        maskfile = cfg.conceptual_mask_file
        cfg_file_name = cfg.logs.reclasssoil_conceptual_cfg
        if soil_property_class.soil_param_type() == ParamAbstractionTypes.PHYSICAL:
            soil_property_file = cfg.soil_property_physical
            inoutcfg_soilfile = cfg.spatials.soil_type_physical
            maskfile = cfg.spatials.subbsn
            cfg_file_name = cfg.logs.reclasssoil_physical_cfg

        SoilUtilClass._parameters_extraction(cfg, inoutcfg_soilfile, maskfile, soil_property_file, cfg_file_name, soil_property_class)


    @staticmethod
    def _parameters_extraction(cfg:PreprocessConfig, inoutcfg_soilfile, maskfile, soil_property_file, cfg_file_name, soil_property_class):
        recls_dict = SoilUtilClass.lookup_soil_parameters(cfg, soil_property_class, soil_property_file)
        inoutcfg = list()
        for k, v in recls_dict.items():
            inoutcfg.append([inoutcfg_soilfile, k,
                             DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE', v])
        mongoargs = [cfg.hostname, cfg.port, cfg.spatial_db, 'SPATIAL']
        if soil_property_class.soil_param_type() == ParamAbstractionTypes.CONCEPTUAL:
            hru_rasterio(
                out_property_name=None,
                hru_property_raster_path=cfg.spatials.soil_texture_conceptual,
                hru_id_raster_path=cfg.spatials.hru_id,
                hru_distribution_raster_path=cfg.spatials.hru_dist,
                aggregation_type=None,
                hru_data_type=rasterio.float64,
                seims_bin=cfg.seims_bin,
                mongoargs=mongoargs,
                hru_subbasin_id_path=cfg.spatials.hru_subbasin_id,
                reclassify_dict=recls_dict,
            )
        elif soil_property_class.soil_param_type() == ParamAbstractionTypes.PHYSICAL:
            mask_rasterio(cfg.seims_bin, inoutcfg, mongoargs=mongoargs,
                          maskfile=maskfile, cfgfile=cfg_file_name,
                          include_nodata=False, mode='MASKDEC',
                          abstraction_type=soil_property_class.soil_param_type())
        SoilUtilClass.initial_soil_moisture(cfg.spatials.d8acc, cfg.spatials.slope,
                                            cfg.spatials.init_somo)



def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    SoilUtilClass.parameters_extraction(seims_cfg, SoilPropertyBase)


if __name__ == "__main__":
    main()
