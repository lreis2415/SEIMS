"""Construct hillslope-slope position units.

The main procedure:
- 1. Assign unique ID to each type of slope position unit with hillslopes, and
   assign the up-down relationships according to the slope position sequence.
- 2. Statistics the landuse types and areas within each slope position units.
- 3. Merge hillslope with incomplete slope position sequences to other hillslopes
   of the same subbasin.
- 4. Merge subbasin with incomplete slope position sequences to its downstream,
   for the outlet subbasin merge to its upstream.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:
    - 17-08-14  lj - initial implementation.
    - 18-02-08  lj - compatible with Python3.
    - 18-11-05  lj - update according to :func:`ImportReaches2Mongo:read_reach_downstream_info`.
                     Add type hints based on typing.
"""
from __future__ import absolute_import, unicode_literals, division

from future.utils import viewitems
import json
from collections import OrderedDict
import os
import sys
from io import open
from struct import unpack, pack

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import numpy
from gridfs import GridFS
from osgeo import osr
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import FileClass, StringClass, MathClass, get_config_parser, is_string
from pymongo.errors import NetworkTimeout

from typing import List, Tuple, Dict, Union, AnyStr
from pygeoc.raster import Raster, RasterUtilClass, DEFAULT_NODATA
from pygeoc.utils import FileClass
from pygeoc.vector import VectorUtilClass
from preprocess.text import DBTableNames, RasterMetadata
from preprocess.db_import_stream_parameters import ImportReaches2Mongo
from preprocess.sd_hillslope import DelineateHillslope
from preprocess.db_mongodb import ConnectMongoDB

from run_seims import ParseSEIMSConfig


class SlopePositionUnits(object):
    """Construct hillslope-slope position units.
    """

    def __init__(self, tag_names,  # type: List[Tuple[int, AnyStr]]
                 slpposf,  # type: AnyStr
                 reach_shp,  # type: AnyStr
                 hillslpf,  # type: AnyStr
                 landusef  # type: AnyStr
                 ):
        # type: (...) -> None
        """Initialization.

        Args:
            tag_names: [tag(integer), name(str)], tag should be ascending from up to bottom.
            slpposf: Crisp classification of slope position full filename.
            reach_shp: Reach shapefile used to extract the up-down relationships of subbasins
            hillslpf: Delineated hillslope file by sd_hillslope.py.
            landusef: Landuse, used to statistics areas of each landuse types within
                      slope position units

        Attributes:
            slppos_tags(OrderedDict): {tag: name}
            subbsin_tree: up-down stream relationships of subbasins.
                          {subbsnID: {'upstream': [], 'downstream': []}}
            units_updwon: Output json data of slope position units.
                {"slppos_1": {id:{"downslope": [ids], "upslope": [ids], "landuse": {luID: area}
                                  "hillslope": [hillslpID], "subbasin": [subbsnID], "area": area
                                 }
                             }
                 "slppos_2": ...
                }
        """
        # Check the file existence
        FileClass.check_file_exists(slpposf)
        FileClass.check_file_exists(reach_shp)
        FileClass.check_file_exists(hillslpf)
        FileClass.check_file_exists(landusef)
        # Set inputs
        self.ws = os.path.dirname(slpposf)
        tag_names = sorted(tag_names, key=lambda x: x[0])
        # initialize slope position dict with up-down relationships
        self.slppos_tags = OrderedDict()  # type: Dict[int, Dict[AnyStr, Union[int, AnyStr]]]
        for idx, tagname in enumerate(tag_names):
            tag, name = tagname
            if len(tag_names) > 1:
                if idx == 0:
                    self.slppos_tags[int(tag)] = {'name': name, 'upslope': -1,
                                                  'downslope': tag_names[idx + 1][0]}
                elif idx == len(tag_names) - 1:
                    self.slppos_tags[int(tag)] = {'name': name, 'upslope': tag_names[idx - 1][0],
                                                  'downslope': -1}
                else:
                    self.slppos_tags[int(tag)] = {'name': name, 'upslope': tag_names[idx - 1][0],
                                                  'downslope': tag_names[idx + 1][0]}
            else:
                self.slppos_tags[int(tag)] = {'name': name, 'upslope': -1, 'downslope': -1}

        self.reach = reach_shp
        # read raster data and check the extent based on hillslope.
        hillslpr = RasterUtilClass.read_raster(hillslpf)
        self.data_hillslp = hillslpr.data
        self.nrows = hillslpr.nRows
        self.ncols = hillslpr.nCols
        self.dx = hillslpr.dx
        self.nodata_hillslp = hillslpr.noDataValue
        self.geotrans = hillslpr.geotrans
        self.srs = hillslpr.srs
        self.datatype = hillslpr.dataType
        slpposr = RasterUtilClass.read_raster(slpposf)
        if slpposr.nRows != self.nrows or slpposr.nCols != self.ncols:
            raise ValueError('The slopeposition raster MUST have the same dimensions'
                             ' with hillslope!')
        self.data_slppos = slpposr.data
        self.nodata_slppos = slpposr.noDataValue
        landuser = RasterUtilClass.read_raster(landusef)
        if landuser.nRows != self.nrows or landuser.nCols != self.ncols:
            raise ValueError('The landuser raster MUST have the same dimensions'
                             ' with hillslope!')
        self.data_landuse = landuser.data
        self.nodata_landuse = landuser.noDataValue

        # Set intermediate data
        self.subbsin_num = -1
        self.subbsin_tree = dict()  # type: Dict[int, int]  # {subbsnID: dst_subbsnID}
        self.units_updwon = OrderedDict()  # type: Dict[AnyStr, Dict[int, Dict[AnyStr, Union[List[float], AnyStr]]]]
        for tag in self.slppos_tags:
            self.units_updwon[self.slppos_tags.get(tag).get('name')] = dict()
        self.slppos_ids = numpy.ones((self.nrows, self.ncols)) * DEFAULT_NODATA
        self.hierarchy_units = dict()  # type: Dict[int, Dict[int, Dict[AnyStr, int]]]

        # Set gene_values of outputs
        self.outf_units_origin = self.ws + os.path.sep + 'slppos_units_origin_uniqueid.tif'
        self.outshp_units_origin = self.ws + os.path.sep + 'origin_uniqueid.shp'
        self.json_units_origin = self.ws + os.path.sep + 'original_updown.json'
        self.outf_units_merged = self.ws + os.path.sep + 'slppos_units.tif'
        self.outshp_units_merged = self.ws + os.path.sep + 'slppos_units_merged.shp'
        self.json_units_merged = self.ws + os.path.sep + 'updown.json'

    def extract_subbasin_updown(self):
        """Extract the up-down relationship of subbasins."""
        subbsn_info = ImportReaches2Mongo.read_reach_downstream_info(self.reach)
        for k, v in viewitems(subbsn_info):
            self.subbsin_tree.setdefault(k, v['downstream'])
        self.subbsin_num = len(self.subbsin_tree)  # type: int

    def assign_uniqueid_slppos_units(self):
        """Get unique ID by multiply slope position value and hillslope ID"""
        for m in range(self.nrows):
            for n in range(self.ncols):
                if self.data_slppos[m][n] == self.nodata_slppos or \
                    self.data_hillslp[m][n] == self.nodata_hillslp:
                    continue
                cur_slppos = int(self.data_slppos[m][n])
                cur_spname = self.slppos_tags.get(cur_slppos).get('name')
                cur_hs = int(self.data_hillslp[m][n])
                if cur_hs <= self.subbsin_num:  # which is stream
                    continue
                cur_subbsn = DelineateHillslope.get_subbasin_from_hillslope_id(cur_hs,
                                                                               self.subbsin_num)
                if cur_subbsn <= 0:
                    continue
                if cur_slppos not in self.slppos_tags:
                    continue
                cur_unit = cur_hs * cur_slppos
                self.slppos_ids[m][n] = cur_unit
                # calculate theoretical downslope and upslope unit ID (check later)
                down_id = self.slppos_tags.get(cur_slppos).get('downslope') * cur_hs
                up_id = self.slppos_tags.get(cur_slppos).get('upslope') * cur_hs
                if down_id < 0:
                    down_id = -1
                if up_id < 0:
                    up_id = -1
                if cur_unit not in self.units_updwon.get(cur_spname):
                    self.units_updwon.get(cur_spname)[cur_unit] = {'area': 1,
                                                                   'downslope': down_id,
                                                                   'upslope': up_id,
                                                                   'hillslope': [cur_hs],
                                                                   'subbasin': [cur_subbsn],
                                                                   'landuse': dict()}
                else:
                    self.units_updwon.get(cur_spname)[cur_unit]['area'] += 1
                # add landuse statistics
                cur_lu = self.data_landuse[m][n]
                if cur_lu != self.nodata_landuse and cur_lu > 0:
                    cur_lu = int(cur_lu)
                    if cur_lu not in self.units_updwon.get(cur_spname)[cur_unit]['landuse']:
                        self.units_updwon.get(cur_spname)[cur_unit]['landuse'][cur_lu] = 0
                    self.units_updwon.get(cur_spname)[cur_unit]['landuse'][cur_lu] += 1

    def check_unit_id(self):
        """check the existence of upslope and downslope units."""
        if 'overview' not in self.units_updwon:
            self.units_updwon['overview'] = OrderedDict()
        all_units_ids = list()
        for sp in self.slppos_tags:
            name = self.slppos_tags.get(sp).get('name')
            cur_units_ids = list(self.units_updwon.get(name).keys())
            self.units_updwon.get('overview')[name] = len(cur_units_ids)
            all_units_ids += cur_units_ids
        self.units_updwon.get('overview')['all_units'] = len(all_units_ids)

        for sp in self.slppos_tags:
            name = self.slppos_tags.get(sp).get('name')
            for unitid, unitdict in self.units_updwon.get(name).items():
                if unitdict.get('upslope') > 0 and unitdict.get('upslope') not in all_units_ids:
                    self.units_updwon.get(name).get(unitid)['upslope'] = -1
                if unitdict.get('downslope') > 0 and unitdict.get('downslope') not in all_units_ids:
                    self.units_updwon.get(name).get(unitid)['downslope'] = -1
                # calculate area
                area_km2 = self.dx * self.dx * 1.e-6
                self.units_updwon.get(name).get(unitid)['area'] *= area_km2
                for luid in self.units_updwon.get(name).get(unitid)['landuse']:
                    self.units_updwon.get(name).get(unitid).get('landuse')[luid] *= area_km2

    @staticmethod
    def check_slppos_sequence(seqs, slppos_tags):
        # type: (List[int], Dict[int, AnyStr]) -> bool
        """Check the slope position sequence is complete or not."""
        flag = True
        for tag in slppos_tags:
            if tag not in seqs:
                flag = False
                break
        return flag

    def merge_slopeposition_units(self):
        """Merge hillslope/subbasin with incomplete slope position sequences"""
        all_units_ids = list()
        for sp in self.slppos_tags:
            name = self.slppos_tags.get(sp).get('name')
            all_units_ids += list(self.units_updwon.get(name).keys())
        # loop each slope position units
        hillslp_elim = dict()
        for sp, spdict in self.slppos_tags.items():
            name = spdict.get('name')
            for unitid, unitdict in self.units_updwon.get(name).items():
                flag = True  # complete sequences or not
                if spdict.get('upslope') > 0 > unitdict.get('upslope'):
                    flag = False
                if spdict.get('downslope') > 0 > unitdict.get('downslope'):
                    flag = False
                cur_hillslp_id = unitdict.get('hillslope')[0]
                if not flag:
                    if cur_hillslp_id not in hillslp_elim:
                        hillslp_elim[cur_hillslp_id] = [unitid]
                if cur_hillslp_id in hillslp_elim and unitid not in hillslp_elim[cur_hillslp_id]:
                    hillslp_elim[cur_hillslp_id].append(unitid)
        print('Hillslope with incomplete slope position sequences: %s' % hillslp_elim.__str__())
        # Basic procedure:
        #   1. if header is incomplete, search left and right;
        #      if left/right is incomplete, search header and right/left;
        #   2. if still not incomplete, check the subbasin,
        #   3. if still not satisfied, merge the left and right to the left or right of downstream
        #      (or upstream for outlet subbasin). The header always with the left hillslope.
        hillslope_merge_order = {0: [1, 2], 1: [0, 2], 2: [0, 1]}
        hillslopes_pair = dict()  # TODO. ljzhu.
        units_pair = dict()  # unit id pairs, key is merged unit, value is destination unit.
        for hillid, unitids in hillslp_elim.items():
            subbsnid = DelineateHillslope.get_subbasin_from_hillslope_id(hillid, self.subbsin_num)
            hillids = DelineateHillslope.cal_hs_codes(self.subbsin_num, subbsnid)
            idx = hillids.index(hillid)
            # slope positions that existed in current hillslope
            sp_seq = list()
            for _id in unitids:
                _id //= hillid
                if _id not in sp_seq:
                    sp_seq.append(_id)

            satisfied = False
            cur_units_pair = dict()
            for curidx in hillslope_merge_order[idx]:
                dst_hillid = hillids[curidx]
                for _id in sp_seq:
                    dst_unit = _id * dst_hillid
                    if dst_unit in all_units_ids:
                        cur_units_pair[_id * hillid] = dst_unit
                for sp in self.slppos_tags:
                    _uid = sp * dst_hillid
                    if _uid in all_units_ids and sp not in sp_seq:
                        sp_seq.append(sp)
                if SlopePositionUnits.check_slppos_sequence(sp_seq, self.slppos_tags):
                    satisfied = True
                    break
            if satisfied:
                units_pair.update(cur_units_pair)
            else:  # means the current subbasin should be merged to downstream
                downstream_merge_idx = {0: 1, 1: 1, 2: 2}
                cur_units_pair = dict()
                dst_subbsn = self.subbsin_tree.get(subbsnid)
                if dst_subbsn < 0:  # outlet subbasin
                    for k, v in self.subbsin_tree.items():
                        if v == subbsnid:
                            dst_subbsn = k
                            break
                # although dst_subbsn may not less than 0, just make sure that!
                if dst_subbsn > 0:
                    dst_hillids = DelineateHillslope.cal_hs_codes(self.subbsin_num, dst_subbsn)
                    dst_downstream_hs = dst_hillids[downstream_merge_idx[idx]]
                    for _id in sp_seq:
                        dst_unit = _id * dst_downstream_hs
                        if dst_unit in all_units_ids:
                            cur_units_pair[_id * hillid] = dst_unit
                units_pair.update(cur_units_pair)

        print('Slope position unit merge-destination pairs: %s' % units_pair.__str__())

        # begin to merge
        for srcid, dstid in units_pair.items():
            # replace values in slope position units raster
            for k, v in units_pair.items():
                self.slppos_ids[self.slppos_ids == k] = v

            for sp in self.slppos_tags:
                name = self.slppos_tags.get(sp).get('name')
                if srcid in list(self.units_updwon.get(name).keys()):  # dstid MUST be here too.
                    srcdict = self.units_updwon.get(name)[srcid]
                    self.units_updwon[name][dstid]['area'] += srcdict['area']
                    for luid in srcdict['landuse']:
                        if luid not in self.units_updwon.get(name)[dstid]['landuse']:
                            self.units_updwon[name][dstid]['landuse'][luid] = \
                                srcdict['landuse'][luid]
                        else:
                            self.units_updwon[name][dstid]['landuse'][luid] += \
                                srcdict['landuse'][luid]
                    if srcdict['hillslope'][0] not in self.units_updwon[name][dstid]['hillslope']:
                        self.units_updwon[name][dstid]['hillslope'].append(srcdict['hillslope'][0])
                    if srcdict['subbasin'][0] not in self.units_updwon[name][dstid]['subbasin']:
                        self.units_updwon[name][dstid]['subbasin'].append(srcdict['subbasin'][0])
                    self.units_updwon.get(name).pop(srcid)
                    break

        # rewrite the overview
        units_count = 0
        for sp in self.slppos_tags:
            name = self.slppos_tags.get(sp).get('name')
            cur_units_count = len(list(self.units_updwon.get(name).keys()))
            self.units_updwon.get('overview')[name] = cur_units_count
            units_count += cur_units_count
        self.units_updwon.get('overview')['all_units'] = units_count

    def extract_subbasin_hillslope_slppos(self):
        """Extract the hierarchical relationship of spatial units, i.e.,
           Subbasin-Hillslope-Slope position
        """
        self.hierarchy_units = dict()
        for sp, spdict in self.slppos_tags.items():
            name = spdict.get('name')
            for unitid, unitdict in self.units_updwon.get(name).items():
                sid = unitdict['subbasin'][0]
                hsid = unitdict['hillslope'][0]
                if sid not in self.hierarchy_units:
                    self.hierarchy_units[sid] = {hsid: {name: unitid}}
                else:
                    if hsid not in self.hierarchy_units[sid]:
                        self.hierarchy_units[sid][hsid] = {name: unitid}
                    else:
                        self.hierarchy_units[sid][hsid][name] = unitid

        self.units_updwon['hierarchy_units'] = dict()
        self.units_updwon['hierarchy_units'].update(self.hierarchy_units)

    def output(self, jfile, unitraster, unitshp):
        """output json file and slope position units raster file"""
        json_updown_data = json.dumps(self.units_updwon, indent=4)
        with open(jfile, 'w', encoding='utf-8') as f:
            f.write('%s' % json_updown_data)
        RasterUtilClass.write_gtiff_file(unitraster, self.nrows, self.ncols,
                                         self.slppos_ids, self.geotrans, self.srs,
                                         DEFAULT_NODATA, self.datatype)
        VectorUtilClass.raster2shp(unitraster, unitshp)
        print("Original unique spatial units ID raster saved as '%s'" % unitraster)

    def run(self):
        """Workflow."""
        self.extract_subbasin_updown()
        self.assign_uniqueid_slppos_units()
        self.check_unit_id()
        self.output(self.json_units_origin, self.outf_units_origin, self.outshp_units_origin)
        self.merge_slopeposition_units()
        self.extract_subbasin_hillslope_slppos()
        self.output(self.json_units_merged, self.outf_units_merged, self.outshp_units_merged)


def ReadRasterFromMongoDB(ip, port, db_name, gfsname, gfilename):
    client = ConnectMongoDB(ip, port)
    conn = client.get_conn()
    maindb = conn[db_name]
    spatial_gfs = GridFS(maindb, gfsname)
    if not spatial_gfs.exists(filename=gfilename):
        raise ValueError('WARNING: %s is not existed in %s:%s!' % (gfilename, db_name, gfsname))
    try:
        gfsdata = maindb[DBTableNames.gridfs_spatial].files.find({'filename': gfilename},
                                                                 no_cursor_timeout=True)[0]
    except NetworkTimeout or Exception:
        # In case of unexpected raise
        client.close()
        return None

    ysize = int(gfsdata['metadata'][RasterMetadata.nrows])
    xsize = int(gfsdata['metadata'][RasterMetadata.ncols])
    xll = gfsdata['metadata'][RasterMetadata.xll]
    yll = gfsdata['metadata'][RasterMetadata.yll]
    cellsize = gfsdata['metadata'][RasterMetadata.cellsize]
    nodata = gfsdata['metadata'][RasterMetadata.nodata]
    srs = gfsdata['metadata'][RasterMetadata.srs]
    if is_string(srs):
        srs = str(srs)
    srs = osr.GetUserInputAsWKT(srs)
    geotransform = [0] * 6
    geotransform[0] = xll - 0.5 * cellsize
    geotransform[1] = cellsize
    geotransform[3] = yll + (ysize - 0.5) * cellsize  # yMax
    geotransform[5] = -cellsize

    array_data = spatial_gfs.get(gfsdata['_id'])
    total_len = xsize * ysize
    fmt = '%df' % (total_len,)
    array_data = unpack(fmt, array_data.read())
    array_data = numpy.reshape(array_data, (ysize, xsize))
    return Raster(ysize, xsize, array_data, nodata, geotransform, srs)


def DelinateSlopePositionByThreshold(modelcfg,  # type: ParseSEIMSConfig
                                     thresholds,  # type: Dict[int, List]
                                     fuzzyslppos_fnames,  # type: List[Tuple[int, AnyStr, AnyStr]]
                                     outfname,  # type: AnyStr
                                     subbsn_id=0  # type: int
                                     ):
    # type: (...) -> Dict
    """

    Args:
        model_cfg: Configuration of SEIMS-based model
        thresholds: {HillslopeID: {rdgID, bksID, vlyID, T_bks2rdg, T_bks2vly}, ...}
        fuzzyslppos_fnames: [(1, 'summit', 'rdgInf'), ...]
        outfname: output GridFS name
        subbsn_id: By default use the whole watershed data
    Returns:
        hillslp_data(dict): {}
    """
    # 1. Read raster data from MongoDB
    hillslpr = ReadRasterFromMongoDB(modelcfg.host, modelcfg.port,
                                     modelcfg.db_name, DBTableNames.gridfs_spatial,
                                     '%d_HILLSLOPE_MERGED' % subbsn_id)
    landuser = ReadRasterFromMongoDB(modelcfg.host, modelcfg.port,
                                     modelcfg.db_name, DBTableNames.gridfs_spatial,
                                     '%d_LANDUSE' % subbsn_id)
    fuzslppos_rs = list()
    for tag, tagname, gfsname in fuzzyslppos_fnames:
        fuzslppos_rs.append(ReadRasterFromMongoDB(modelcfg.host, modelcfg.port,
                                                  modelcfg.db_name, DBTableNames.gridfs_spatial,
                                                  '%d_%s' % (subbsn_id, gfsname.upper())))

    # Output for test
    # out_dir = r'D:\data_m\youwuzhen\seims_models_phd\data_prepare\spatial\spatial_units\tmp'
    # out_hillslp = out_dir + os.sep + 'hillslope.tif'
    # RasterUtilClass.write_gtiff_file(out_hillslp, hillslpr.nRows, hillslpr.nCols,
    #                                  hillslpr.data, hillslpr.geotrans, hillslpr.srs,
    #                                  hillslpr.noDataValue)
    # out_landuse = out_dir + os.sep + 'landuse.tif'
    # RasterUtilClass.write_gtiff_file(out_landuse, landuser.nRows, landuser.nCols,
    #                                  landuser.data, landuser.geotrans, landuser.srs,
    #                                  landuser.noDataValue)
    # for i, (tag, tagname, gfsname) in enumerate(fuzzyslppos_fnames):
    #     curname = out_dir + os.sep + '%s.tif' % gfsname
    #     RasterUtilClass.write_gtiff_file(curname, fuzslppos_rs[i].nRows, fuzslppos_rs[i].nCols,
    #                                      fuzslppos_rs[i].data, fuzslppos_rs[i].geotrans,
    #                                      fuzslppos_rs[i].srs,
    #                                      fuzslppos_rs[i].noDataValue)

    # 2. Initialize output
    outgfsname = '%d_%s' % (subbsn_id, outfname.upper())
    outdict = dict()  # type: Dict[AnyStr, Dict[int, Dict[AnyStr, Union[float, Dict[int, float]]]]]
    slppos_cls = numpy.ones((hillslpr.nRows, hillslpr.nCols)) * hillslpr.noDataValue
    valid_cells = 0

    # Get the fuzzy slope position values from up to bottom
    def GetFuzzySlopePositionValues(i_row, i_col):
        seqvalues = [-9999] * len(fuzslppos_rs)
        for iseq, fuzdata in enumerate(fuzslppos_rs):
            curv = fuzdata.data[i_row][i_col]
            if MathClass.floatequal(curv, fuzdata.noDataValue):
                return None
            if curv < 0:
                return None
            seqvalues[iseq] = curv
        return seqvalues

    # ACTUAL ALGORITHM
    for row in range(hillslpr.nRows):
        for col in range(hillslpr.nCols):
            # Exclude invalid situation
            hillslp_id = hillslpr.data[row][col]
            if MathClass.floatequal(hillslp_id, hillslpr.noDataValue):
                continue
            if hillslp_id not in thresholds:
                continue
            landuse_id = landuser.data[row][col]
            if MathClass.floatequal(landuse_id, landuser.noDataValue):
                continue
            fuzzyvalues = GetFuzzySlopePositionValues(row, col)
            if fuzzyvalues is None:
                continue

            # THIS PART SHOULD BE REVIEWED CAREFULLY LATER! --START
            # Step 1. Get the index of slope position with maximum similarity
            max_fuz = max(fuzzyvalues)
            max_idx = fuzzyvalues.index(max_fuz)
            tmpfuzzyvalues = fuzzyvalues[:]
            tmpfuzzyvalues.remove(max_fuz)
            sec_fuz = max(tmpfuzzyvalues)
            sec_idx = fuzzyvalues.index(sec_fuz)

            sel_idx = max_idx  # Select the maximum by default

            cur_threshs = thresholds[hillslp_id][1 - len(fuzzyvalues):]

            if max_idx == len(fuzzyvalues) - 1:  # the bottom position
                if sec_idx == len(fuzzyvalues) - 2 and 0 < max_fuz - sec_fuz < cur_threshs[-1]:
                    sel_idx = sec_idx  # change valley to backslope
            elif max_idx == 0:  # the upper position
                if sec_idx == 1 and 0 < max_fuz - sec_fuz < cur_threshs[0]:
                    sel_idx = sec_idx  # change ridge to backslope
            else:  # the middle positions
                # Two thresholds could be applied,
                #     i.e., cur_threshs[max_idx-1] and cur_threshs[max_idx]
                if sec_idx == max_idx - 1 and 0. > sec_fuz - max_fuz > cur_threshs[max_idx - 1]:
                    sel_idx = sec_idx
                elif sec_idx == max_idx + 1 and 0. > sec_fuz - max_fuz > cur_threshs[max_idx]:
                    sel_idx = sec_idx

            # Exception:
            if sec_fuz < 0.1 and sel_idx == sec_idx:
                sel_idx = max_idx

            # if sel_idx != max_idx:  # boundary has been adapted
            #     print('fuzzy values: %s, thresholds: %s, '
            #           'sel_idx: %d' % (fuzzyvalues.__str__(), cur_threshs.__str__(), sel_idx))

            slppos_id = thresholds[hillslp_id][sel_idx]
            # THIS PART SHOULD BE REVIEWED CAREFULLY LATER! --END

            slppos_cls[row][col] = slppos_id
            sel_tagname = fuzzyslppos_fnames[sel_idx][1]
            if sel_tagname not in outdict:
                outdict[sel_tagname] = dict()
            if slppos_id not in outdict[sel_tagname]:
                outdict[sel_tagname][slppos_id] = {'area': 0, 'landuse': dict()}
            outdict[sel_tagname][slppos_id]['area'] += 1
            if landuse_id not in outdict[sel_tagname][slppos_id]['landuse']:
                outdict[sel_tagname][slppos_id]['landuse'][landuse_id] = 0.
            outdict[sel_tagname][slppos_id]['landuse'][landuse_id] += 1.

            valid_cells += 1
    # Change cell counts to area
    area_km2 = hillslpr.dx * hillslpr.dx * 1.e-6
    for tagname, slpposdict in viewitems(outdict):
        for sid, datadict in viewitems(slpposdict):
            outdict[tagname][sid]['area'] *= area_km2
            for luid in outdict[tagname][sid]['landuse']:
                outdict[tagname][sid]['landuse'][luid] *= area_km2

    # 3. Write the classified slope positions data back to mongodb
    metadata = dict()
    metadata[RasterMetadata.subbasin] = subbsn_id
    metadata['ID'] = outgfsname
    metadata['TYPE'] = outfname.upper()
    metadata[RasterMetadata.cellsize] = hillslpr.dx
    metadata[RasterMetadata.nodata] = hillslpr.noDataValue
    metadata[RasterMetadata.ncols] = hillslpr.nCols
    metadata[RasterMetadata.nrows] = hillslpr.nRows
    metadata[RasterMetadata.xll] = hillslpr.xMin + 0.5 * hillslpr.dx
    metadata[RasterMetadata.yll] = hillslpr.yMin + 0.5 * hillslpr.dx
    metadata['LAYERS'] = 1.
    metadata[RasterMetadata.cellnum] = valid_cells
    metadata[RasterMetadata.srs] = hillslpr.srs

    client = ConnectMongoDB(modelcfg.host, modelcfg.port)
    conn = client.get_conn()
    maindb = conn[modelcfg.db_name]
    spatial_gfs = GridFS(maindb, DBTableNames.gridfs_spatial)
    # delete if the tablename gridfs file existed
    if spatial_gfs.exists(filename=outgfsname):
        x = spatial_gfs.get_version(filename=outgfsname)
        spatial_gfs.delete(x._id)
    # create and write new GridFS file
    new_gridfs = spatial_gfs.new_file(filename=outgfsname, metadata=metadata)
    new_gridfs_array = slppos_cls.reshape((1, hillslpr.nCols * hillslpr.nRows)).tolist()[0]

    fmt = '%df' % hillslpr.nCols * hillslpr.nRows
    s = pack(fmt, *new_gridfs_array)
    new_gridfs.write(s)
    new_gridfs.close()

    # Read and output for test
    # slpposcls_r = ReadRasterFromMongoDB(modelcfg.host, modelcfg.port,
    #                                     modelcfg.db_name, DBTableNames.gridfs_spatial, outgfsname)
    # out_slpposcls = out_dir + os.sep + '%s.tif' % outgfsname
    # RasterUtilClass.write_gtiff_file(out_slpposcls, slpposcls_r.nRows, slpposcls_r.nCols,
    #                                  slpposcls_r.data, slpposcls_r.geotrans, slpposcls_r.srs,
    #                                  slpposcls_r.noDataValue)
    client.close()

    return outdict


def main():
    """Delineation slope position units with the associated information."""
    from preprocess.config import parse_ini_configuration
    seims_cfg = parse_ini_configuration()

    reach_shp = seims_cfg.vecs.reach
    hillslp_file = seims_cfg.spatials.hillslope
    landuse_file = seims_cfg.spatials.landuse

    # additional inputs
    slppos_tag_name = [(1, 'summit'), (4, 'backslope'), (16, 'valley')]
    slppos_file = r'D:\data_m\youwuzhen\seims_models_phd\data_prepare\spatial\spatial_units\SLOPEPOSITION.tif'

    obj = SlopePositionUnits(slppos_tag_name, slppos_file, reach_shp, hillslp_file, landuse_file)
    obj.run()


def main2():
    seims_cfg = ParseSEIMSConfig()
    seims_cfg.db_name = 'youwuzhen10m_longterm_model'
    thresholds = {18: [18, 72, 288, 0.1, -0.1]}
    fuzzyslppos_fnames = [(1, 'summit', 'rdgInf'),
                          (4, 'backslope', 'bksInf'),
                          (16, 'valley', 'vlyInf')]
    outfname = 'SLPPOS_UNITS_10121314'
    hillslp_data = DelinateSlopePositionByThreshold(seims_cfg, thresholds,
                                                    fuzzyslppos_fnames, outfname)
    print(hillslp_data)


if __name__ == '__main__':
    main()
    # main2()
