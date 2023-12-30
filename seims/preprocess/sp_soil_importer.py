import os
import sys
import logging
import numpy as np
import rasterio

from preprocess.config import PreprocessConfig
from preprocess.sp_soil_utils import SoilUtilClass
from preprocess.sp_soil_base import SoilPropertyBase
from preprocess.sp_soil_conceptual import SoilPropertyConceptual
from preprocess.sp_soil_physical import SoilPropertyPhysical

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import StringClass

from utility import DEFAULT_NODATA
from utility import read_data_items_from_txt
from utility import mask_rasterio
from preprocess.text import ParamAbstractionTypes


class SoilImporter(object):

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
    def _get_soil_property_instances(cfg, soil_property_class, soil_lookup_file, has_septic_layer=True):
        soil_lookup_data = read_data_items_from_txt(soil_lookup_file)
        soil_instances = list()
        soil_prop_flds = soil_lookup_data[0][:]

        if soil_property_class.soil_param_type() == ParamAbstractionTypes.PHYSICAL:
            workspace_soil_file = cfg.spatials.soil_type_physical
        else:
            workspace_soil_file = cfg.spatials.soil_type_conceptual
        SoilImporter._reclassify_missing_soil_types_in_lookup(cfg.soil,
                                                              workspace_soil_file,
                                                              [int(row[0]) for row in soil_lookup_data[1:]],
                                                              cfg.default_soil)

        for i in range(1, len(soil_lookup_data)):
            cur_soil_data_item = soil_lookup_data[i][:]
            cur_seqn = cur_soil_data_item[0]
            cur_sname = cur_soil_data_item[1]
            cur_soil_ins = soil_property_class(cur_seqn, cur_sname)
            for j in range(2, len(soil_prop_flds)):
                if not hasattr(cur_soil_ins, soil_prop_flds[j]):
                    continue
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

            cur_soil_ins.construct(has_septic_layer=has_septic_layer)
            soil_instances.append(cur_soil_ins)
        return soil_instances

    @staticmethod
    def lookup_soil_parameters(cfg, soil_property_class, soil_lookup_file, has_septic_layer=True):
        """Reclassify soil parameters by lookup table.

        Returns:
            recls_dict: dict, e.g., {'OM': '201:1.3|1.2|0.6,202:1.4|1.1|0.8'}
        """
        #  Read soil properties from txt file

        soil_instances = SoilImporter._get_soil_property_instances(cfg, soil_property_class, soil_lookup_file, has_septic_layer=has_septic_layer)

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
                    if isinstance(soil_prop_dict[key][i], list):
                        cur_dict[tmpseq] = soil_prop_dict[key][i]
                    else:
                        cur_dict[tmpseq][0] = soil_prop_dict[key][i]
                    continue
                for j in range(soil_prop_dict[SoilPropertyBase.NLYRS][i]):
                    cur_dict[tmpseq][j] = soil_prop_dict[key][i][j]
            kstrings = list()
            for k, v in cur_dict.items():
                kstring = '%s:%s' % (k, '|'.join(repr(vv) for vv in v))
                kstrings.append(kstring)
            new_kstring = ','.join(kstrings)
            recls_dict[key] = new_kstring
            # recls_dict[key] = ','.join('%s:%s' % (k, '|'.join(repr(vv) for vv in v))
            #                            for k, v in cur_dict.items())
        return recls_dict

    @staticmethod
    def parameters_extraction(cfg):
        """Soil spatial parameters extraction."""
        # 1. Calculate conceptual soil parameters
        conceptual_soil_layers = 0
        mongoargs = [cfg.hostname, cfg.port, cfg.spatial_db, 'SPATIAL']
        if cfg.soil_property_conceptual:
            recls_dict_conceptual = SoilImporter.lookup_soil_parameters(cfg, SoilPropertyConceptual, cfg.soil_property_conceptual, has_septic_layer=False)
            conceptual_soil_layers = int(recls_dict_conceptual[SoilPropertyBase.NLYRS].split(',')[0].split(':')[1])
            inoutcfg_conceptual = list()
            for k, v in recls_dict_conceptual.items():
                inoutcfg_conceptual.append([cfg.spatials.soil_type_conceptual, k,
                                            DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE', v])
            mask_rasterio(cfg.seims_bin, inoutcfg_conceptual, mongoargs=mongoargs,
                          maskfile=cfg.conceptual_mask_file, cfgfile=cfg.logs.reclasssoil_conceptual_cfg,
                          include_nodata=False, mode='MASKDEC',
                          abstraction_type=SoilPropertyConceptual.soil_param_type())
        # 2. Calculate physical soil parameters
        if cfg.soil_property_physical:
            recls_dict = SoilImporter.lookup_soil_parameters(cfg, SoilPropertyPhysical, cfg.soil_property_physical, has_septic_layer=True)
            inoutcfg = list()
            for k, v in recls_dict.items():
                inoutcfg.append([cfg.spatials.soil_type_physical, k,
                                 DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE', v])
            mask_rasterio(cfg.seims_bin, inoutcfg, mongoargs=mongoargs,
                          maskfile=cfg.spatials.subbsn, cfgfile=cfg.logs.reclasssoil_physical_cfg,
                          include_nodata=False, mode='MASKDEC',
                          abstraction_type=SoilPropertyPhysical.soil_param_type())
            SoilUtilClass.initial_soil_moisture(cfg.spatials.d8acc, cfg.spatials.slope,
                                                cfg.spatials.init_somo)

            # 3. if both conceptual and physical soil parameters are required, then copy & convert the physical params to conceptual
            # if cfg.soil_property_conceptual and conceptual_soil_layers > 0:
            #     # Re-calculate physical parameters, but do not calculate the septic layer
            #     recls_dict = SoilImporter.lookup_soil_parameters(cfg, SoilPropertyPhysical, cfg.soil_property_physical, has_septic_layer=False)
            #     inoutcfg = list()
            #     key_to_del = [SoilPropertyBase.SEQN, SoilPropertyBase.SNAM, SoilPropertyBase.NLYRS]
            #     for k, v in recls_dict.items():
            #         if k in key_to_del:
            #             continue
            #         inoutcfg.append([cfg.spatials.soil_type_physical, k,
            #                          DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE', v])
            #     for i in range(len(inoutcfg)):
            #         v = inoutcfg[i][5]
            #         soil_types = v.split(',')
            #         new_soil_types_list = list()
            #         for soil_type in soil_types:
            #             p, recls_value = soil_type.split(':')
            #             new_soil_type = f"{p}:{'|'.join(recls_value.split('|')[:conceptual_soil_layers])}"
            #             new_soil_types_list.append(new_soil_type)
            #         inoutcfg[i][5] = ','.join(new_soil_types_list)
            #         print(i)
            #
            #     mask_rasterio(cfg.seims_bin, inoutcfg, mongoargs=mongoargs,
            #                   maskfile=cfg.conceptual_mask_file, cfgfile=cfg.logs.reclasssoil_physical_cfg,
            #                   include_nodata=False, mode='MASKDEC',
            #                   abstraction_type=SoilPropertyConceptual.soil_param_type())




def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    SoilImporter.parameters_extraction(seims_cfg)


if __name__ == "__main__":
    main()
