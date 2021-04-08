"""Extract fuzzy slope positions along flow path from ridge to valley.

    - 1. Read ridge sources, by default, ridge means there are no cells flow in.
    - 2. Trace down and extract the similarities of fuzzy slope positions.
    - 3. Construct the output ESRI Shapefile.

    @author   : Liangjun Zhu

    @changelog:
    - 15-09-08  - lj - initial implementation.
    - 17-07-30  - lj - reorganize and incorporate with pygeoc.
"""
from __future__ import absolute_import, unicode_literals
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.raster import RasterUtilClass

from autofuzslppos.Config import get_input_cfgs


def fuzslppos_profiles(rdgfile, flowdirfile, streamfile, attr_dict, outattrtxt, shpfile):
    """Extract fuzzy slope positions and other attributes along flow path

    TODO:
        This function is not finished yet.

    Args:
        rdgfile: ridge source raster file.
        flowdirfile: used to trace downslope.
        streamfile: used to determining termination of each flow path.
        attr_dict: Topographic attributes desired to be extracted.
        outattrtxt: Topographic attributes associated with each profile.
        shpfile: results ESRI Shapefile.
    """
    # The basic structure is:
    # {rdgCellID: {'profileCoors': [[100,20], [100, 21], ...],
    #              'attributes': {'attr1': [223, 220, ...],
    #                             'attr2': [0.99, 0.89, ...],
    #                             }
    #             }
    # }
    profile_attr_dict = dict()
    # 1. Read ridge source and store the coordinates
    rdg_cell_id = 0
    profile_attr_dict[rdg_cell_id] = {'profileCoors': [], 'attributes': {}}

    # 2. Trace down from each ridge cell to get profile coordinates and attributes

    directionr = RasterUtilClass.read_raster(flowdirfile)
    rows = directionr.nRows
    cols = directionr.nCols
    direc = directionr.data
    geo = directionr.geotrans
    nodata = directionr.noDataValue
    streamr = RasterUtilClass.read_raster(streamfile)
    stream = streamr.data
    stream_nodata = streamr.noDataValue


def main():
    """TEST CODE"""
    fuzslppos_cfg = get_input_cfgs()
    rdg_file = fuzslppos_cfg.pretaudem.rdgsrc
    d8dir_file = fuzslppos_cfg.pretaudem.d8flow
    d8strm_file = fuzslppos_cfg.pretaudem.stream_raster
    attr_list = {'dem': fuzslppos_cfg.pretaudem.filldem}
    outattrtxt = fuzslppos_cfg.ws.pre_dir + os.sep + 'profiles_attributes.txt'
    outshp = fuzslppos_cfg.ws.pre_dir + os.sep + 'profiles.shp'

    fuzslppos_profiles(rdg_file, d8dir_file, d8strm_file, attr_list, outattrtxt, outshp)


if __name__ == '__main__':
    main()
