"""Construct information of common units, e.g., subbasin, HRU, spatially unique HRU.

    @author   : Liangjun Zhu

    @changelog:
    - 18-11-06  lj - initial implementation.
"""
from __future__ import absolute_import, unicode_literals, division

import json
import os
import sys
from io import open
from future.utils import viewitems

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from typing import List, Dict, Union, AnyStr
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import FileClass

from utility.io_plain_text import read_data_items_from_txt


def main(landusef, unitsf, jsonout):
    """Construct common spatial units data in JSON file format."""
    # Check the file existence
    FileClass.check_file_exists(landusef)
    FileClass.check_file_exists(unitsf)
    # read raster data and check the extent based on landuse.
    landuser = RasterUtilClass.read_raster(landusef)
    data_landuse = landuser.data
    nrows = landuser.nRows
    ncols = landuser.nCols
    dx = landuser.dx
    nodata_landuse = landuser.noDataValue

    fieldr = RasterUtilClass.read_raster(unitsf)
    if fieldr.nRows != nrows or fieldr.nCols != ncols:
        raise ValueError('The spatial units raster MUST have the same dimensions'
                         ' with landuse!')
    data_units = fieldr.data
    nodata_units = fieldr.noDataValue

    units_info = dict()  # type: Dict[AnyStr, Dict[Union[int, AnyStr], Dict[AnyStr, Union[int, float, List[Union[int,float]], AnyStr, Dict[int, float]]]]]

    units_info.setdefault('units', dict())
    units_info.setdefault('overview', dict())

    units_ids = list()  # type: List[int]

    for m in range(nrows):
        for n in range(ncols):
            cur_lu = int(data_landuse[m][n])
            cur_unit = int(data_units[m][n])
            if cur_unit == nodata_units or cur_lu == nodata_landuse or cur_lu <= 0:
                continue
            if cur_unit not in units_ids:
                units_ids.append(cur_unit)
            if cur_unit not in units_info['units']:
                units_info['units'].setdefault(cur_unit, {'landuse': dict(),
                                                          'primarylanduse': 0,
                                                          'area': 0.})
            if cur_lu not in units_info['units'][cur_unit]['landuse']:
                units_info['units'][cur_unit]['landuse'][cur_lu] = 1
            else:
                units_info['units'][cur_unit]['landuse'][cur_lu] += 1
    for k, v in viewitems(units_info['units']):
        area_field = 0.
        area_max = 0.
        area_max_lu = 0
        for luid, luarea in viewitems(v['landuse']):
            v['landuse'][luid] = luarea * dx * dx * 1.e-6
            area_field += v['landuse'][luid]
            if v['landuse'][luid] > area_max:
                area_max = v['landuse'][luid]
                area_max_lu = luid
        v['area'] = area_field
        v['primarylanduse'] = area_max_lu

    units_info['overview'].setdefault('all_units', len(units_ids))

    # save to json
    json_data = json.dumps(units_info, indent=4)
    with open(json_out, 'w', encoding='utf-8') as f:
        f.write('%s' % json_data)


if __name__ == '__main__':
    landuse_file = r'C:\z_data_m\SEIMS2018\youwuzhen_10m\spatial_raster\landuse.tif'
    ws = r'D:\data_m\youwuzhen\seims_models_phd\data_prepare\spatial\spatial_units'
    unit_raster = ws + os.sep + 'spatial_nonunique_hrus.tif'
    json_out = ws + os.sep + 'nonunique_hru_units.json'
    main(landuse_file, unit_raster, json_out)
