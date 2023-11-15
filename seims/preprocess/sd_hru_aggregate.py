import logging

import numpy as np
import osgeo
import rasterio
from pathlib import Path

from preprocess.text import ParamAbstractionTypes
from utility import DEFAULT_NODATA, mask_rasterio


class HruAggregationFunctions:
    """HRU aggregation types"""

    AS_IS = 'as_is'
    ARITHMETIC_MEAN = 'arithmetic_mean'
    GEOMETRIC_MEAN = 'geometric_mean'
    SUM = 'sum'
    MAX = 'max'
    MIN = 'min'
    MEDIAN = 'median'
    MODE = 'mode'
    COUNT = 'count'

    @staticmethod
    def get_type_by_param_name(param_name):
        if param_name in ['some params']:
            return HruAggregationFunctions.AS_IS
        else:
            return HruAggregationFunctions.ARITHMETIC_MEAN

    @staticmethod
    def get_aggr_func_by_type_str(aggregation_type):
        if aggregation_type == HruAggregationFunctions.AS_IS:
            return HruAggregationFunctions.as_is
        elif aggregation_type == HruAggregationFunctions.ARITHMETIC_MEAN:
            return HruAggregationFunctions.arithmetic_mean
        elif aggregation_type == HruAggregationFunctions.GEOMETRIC_MEAN:
            return HruAggregationFunctions.geometric_mean
        elif aggregation_type == HruAggregationFunctions.SUM:
            return HruAggregationFunctions.sum
        elif aggregation_type == HruAggregationFunctions.MAX:
            return HruAggregationFunctions.max
        elif aggregation_type == HruAggregationFunctions.MIN:
            return HruAggregationFunctions.min
        elif aggregation_type == HruAggregationFunctions.MEDIAN:
            return HruAggregationFunctions.median
        elif aggregation_type == HruAggregationFunctions.MODE:
            return HruAggregationFunctions.mode
        elif aggregation_type == HruAggregationFunctions.COUNT:
            return HruAggregationFunctions.count
        else:
            raise ValueError('ErrorInput: Invalid aggregation type: %s' % aggregation_type)

    @staticmethod
    def as_is(hru_property):
        return hru_property

    @staticmethod
    def arithmetic_mean(hru_property):
        return np.mean(hru_property)

    @staticmethod
    def geometric_mean(hru_property):
        return np.exp(np.mean(np.log(hru_property)))

    @staticmethod
    def sum(hru_property):
        return np.sum(hru_property)

    @staticmethod
    def max(hru_property):
        return np.max(hru_property)

    @staticmethod
    def min(hru_property):
        return np.min(hru_property)

    @staticmethod
    def median(hru_property):
        return np.median(hru_property)

    @staticmethod
    def mode(hru_property):
        return np.mode(hru_property)

    @staticmethod
    def count(hru_property):
        return len(hru_property)


def _hru_aggregate(hru_property_raster, hru_id_raster, hru_distribution_raster, aggr_func, multiplier=1, hru_nodata=DEFAULT_NODATA):
    hru_raster = hru_id_raster.copy()
    # for every unique value in hru_id_raster, aggregate hru_property_raster
    for hru in np.unique(hru_id_raster):
        if hru == hru_nodata:
            continue
        # get hru_property_raster whose positions in hru_distribution_raster are hru
        hru_property = hru_property_raster[hru_distribution_raster == hru]
        hru_raster[(hru_distribution_raster == hru) & (hru_raster != hru_nodata)] \
            = aggr_func(hru_property) * multiplier
    return hru_raster


def hru_rasterio(
    out_property_name,
    hru_property_raster_path,
    hru_id_raster_path,
    hru_distribution_raster_path,
    seims_bin,
    mongoargs,
    hru_subbasin_id_path,
    aggregation_type: str = HruAggregationFunctions.ARITHMETIC_MEAN,
    aggregation_multiplier=1,
):
    logging.info(f'Aggregating HRU property {out_property_name}...')
    hru_id_map_dataset = rasterio.open(hru_id_raster_path)
    hru_id_map = hru_id_map_dataset.read(1)
    hru_id_map = hru_id_map.astype(np.float64)


    hru_dist_map_dataset = rasterio.open(hru_distribution_raster_path)
    hru_dist_map = hru_dist_map_dataset.read(1)
    hru_id_map_dataset.close()
    hru_dist_map_dataset.close()

    hru_property_dataset = rasterio.open(hru_property_raster_path)
    out_meta = hru_property_dataset.meta.copy()
    hru_property = hru_property_dataset.read(1)
    aggregation_function = HruAggregationFunctions.get_aggr_func_by_type_str(aggregation_type)
    out_raster = _hru_aggregate(hru_property,
                                hru_id_map,
                                hru_dist_map,
                                aggregation_function,
                                aggregation_multiplier,
                                hru_nodata=hru_property_dataset.nodata)

    out_path = Path(hru_property_raster_path).parent / (Path(hru_property_raster_path).stem + "_hru.tif")
    out_meta.update(compress='lzw')
    with rasterio.open(out_path, 'w', **out_meta) as dst:
        dst.write(out_raster, 1)

    hru_cfg = [[out_path, out_property_name,
                DEFAULT_NODATA, DEFAULT_NODATA, out_meta.get('dtype')]]

    mask_rasterio(seims_bin, hru_cfg, mongoargs=mongoargs,
                  maskfile=hru_subbasin_id_path, include_nodata=False, mode='MASKDEC',
                  abstraction_type=ParamAbstractionTypes.CONCEPTUAL)

#
# def hru_rasterio(
#     out_property_dict: dict,
#     hru_id_raster_path,
#     hru_distribution_raster_path,
#     hru_data_type,
#     seims_bin,
#     mongoargs,
#     hru_subbasin_id_path,
#     aggregation_type: str = HruAggregationFunctions.ARITHMETIC_MEAN,
#     aggregation_multiplier=1,
# ):
#     """Output HRU to raster files with values distributed to single points.
#     Args:
#         out_property_dict: {out_property_name: hru_property_raster_path}
#         hru_distribution_raster_path: HRU distribution map path
#         aggregation_type: HRU aggregation type
#         hru_data_type: HRU value type, 'FLOAT32' or 'INT32'
#         reclassify_dict: reclassify dict, e.g.: {'MANNING': '1:0.15,2:0.15,10:0.2'}
#             when this is given, out_property_name should be None.
#     """
#     # if is a list
#     for
#             hru_rasterio(
#                 out_property_names=out_property_names,
#                 hru_property_raster_paths=p,
#                 hru_id_raster_path=hru_id_raster_path,
#                 hru_distribution_raster_path=hru_distribution_raster_path,
#                 hru_data_type=hru_data_type,
#                 seims_bin=seims_bin,
#                 mongoargs=mongoargs,
#                 hru_subbasin_id_path=hru_subbasin_id_path,
#                 aggregation_type=aggregation_type,
#                 aggregation_multiplier=aggregation_multiplier
#             )
#             # hru_rasterio(out_property_name, p, hru_id_raster_path, hru_distribution_raster_path,
#             #              aggregation_type, hru_data_type, seims_bin, mongoargs, hru_subbasin_id_path)
#
#
