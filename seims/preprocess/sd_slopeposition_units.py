#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Construct hillslope-slope position units.

    The main procedure:\n
      1. Assign unique ID to each type of slope position unit with hillslopes, and
         assign the up-down relationships according to the slope position sequence.\n
      2. Statistics the landuse types and areas within each slope position units.\n
      3. Merge hillslope with incomplete slope position sequences to other hillslopes
         of the same subbasin.\n
      4. Merge subbasin with incomplete slope position sequences to its downstream,
         for the outlet subbasin merge to its upstream.

    @author   : Liangjun Zhu, Huiran Gao
    @changelog: 17-08-14  lj - initial implementation.\n
"""
import json
import os
from collections import OrderedDict

import numpy
from pygeoc.raster import RasterUtilClass, DEFAULT_NODATA
from pygeoc.utils import FileClass
from pygeoc.vector import VectorUtilClass

from db_import_stream_parameters import ImportReaches2Mongo
from sd_hillslope import DelineateHillslope


class SlopePositionUnits(object):
    """Construct hillslope-slope position units.
    """

    def __init__(self, tag_names, slpposf, reach_shp, hillslpf, landusef):
        """Initialization.

        Args:
            tag_names: [tag(integer), name(str)], tag should be ascending from up to bottom.
            slpposf: Crisp classification of slope position.
            reach_shp: Reach shapefile used to extract the up-down relationships of subbasins
            hillslpf: Delineated hillslope file by sd_hillslope.py.
            landusef: Landuse, used to statistics areas of each landuse types within
                      slope position units

        Attributes:
            slppos_tags(OrderedDict): {tag: name}
            subbsin_tree: up-down stream relationships of subbasins.
                          {subbsnID: {'upstream': [], 'downstream': []}}
            units_updown_info: Output json data of slope position units.
                {"slppos_1": {id:{"downslope": [ids], "upslope": [ids], "landuse": {luID: area}
                                  "hillslope": [hillslpID], "subbasin": [subbsnID], "area": area
                                 }
                             }
                 "slppos_2": ...
                }
        """
        # Check the file existance
        FileClass.check_file_exists(slpposf)
        FileClass.check_file_exists(reach_shp)
        FileClass.check_file_exists(hillslpf)
        FileClass.check_file_exists(landusef)
        # Set inputs
        self.ws = os.path.dirname(slpposf)
        tag_names = sorted(tag_names, key=lambda x: x[0])
        # initialize slope position dict with up-down relationships
        self.slppos_tags = OrderedDict()
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
        self.subbsin_tree = dict()
        self.units_updwon = OrderedDict()
        for tag in self.slppos_tags:
            self.units_updwon[self.slppos_tags.get(tag).get('name')] = dict()
        self.slppos_ids = numpy.ones((self.nrows, self.ncols)) * DEFAULT_NODATA
        self.hierarchy_units = dict()

        # Set gene_values of outputs
        self.outf_units_origin = self.ws + os.sep + 'slppos_units_origin_uniqueid.tif'
        self.outshp_units_origin = self.ws + os.sep + 'origin_uniqueid.shp'
        self.json_units_origin = self.ws + os.sep + 'original_updown.json'
        self.outf_units_merged = self.ws + os.sep + 'slppos_units.tif'
        self.outshp_units_merged = self.ws + os.sep + 'slppos_units_merged.shp'
        self.json_units_merged = self.ws + os.sep + 'updown.json'

    def extract_subbasin_updown(self):
        """Extract the up-down relationship of subbasins."""
        self.subbsin_tree = ImportReaches2Mongo.down_stream(self.reach)[0]
        self.subbsin_num = len(self.subbsin_tree)

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
                        self.units_updwon.get(cur_spname)[cur_unit]['landuse'][cur_lu] = 1
                    else:
                        self.units_updwon.get(cur_spname)[cur_unit]['landuse'][cur_lu] += 1

    def check_unit_id(self):
        """check the existence of upslope and downslope units."""
        if 'overview' not in self.units_updwon:
            self.units_updwon['overview'] = OrderedDict()
        all_units_ids = list()
        for sp in self.slppos_tags:
            name = self.slppos_tags.get(sp).get('name')
            cur_units_ids = self.units_updwon.get(name).keys()
            self.units_updwon.get('overview')[name] = len(cur_units_ids)
            all_units_ids += cur_units_ids
        self.units_updwon.get('overview')['all_units'] = len(all_units_ids)

        for sp in self.slppos_tags:
            name = self.slppos_tags.get(sp).get('name')
            for unitid, unitdict in self.units_updwon.get(name).iteritems():
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
            all_units_ids += self.units_updwon.get(name).keys()
        # loop each slope position units
        hillslp_elim = dict()
        for sp, spdict in self.slppos_tags.iteritems():
            name = spdict.get('name')
            for unitid, unitdict in self.units_updwon.get(name).iteritems():
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
        print ('Hillslope with incomplete slope position sequences: ' + hillslp_elim.__str__())
        # Basic procedure:
        #   1. if header is incomplete, search left and right;
        #      if left/right is incomplete, search header and right/left;
        #   2. if still not incomplete, check the subbasin,
        #   3. if still not satisfied, merge the left and right to the left or right of downstream
        #      (or upstream for outlet subbasin). The header always with the left hillslope.
        hillslope_merge_order = {0: [1, 2], 1: [0, 2], 2: [0, 1]}
        units_pair = dict()  # unit id pairs, key is merged unit, value is destination unit.
        for hillid, unitids in hillslp_elim.iteritems():
            subbsnid = DelineateHillslope.get_subbasin_from_hillslope_id(hillid, self.subbsin_num)
            hillids = DelineateHillslope.cal_hs_codes(self.subbsin_num, subbsnid)
            idx = hillids.index(hillid)
            # slope positions that existed in current hillslope
            sp_seq = list()
            for _id in unitids:
                _id /= hillid
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
                    for k, v in self.subbsin_tree.iteritems():
                        if v == subbsnid:
                            dst_subbsn = k
                            break
                if dst_subbsn > 0:  # although dst_subbsn may not less than 0, just make sure that!
                    dst_hillids = DelineateHillslope.cal_hs_codes(self.subbsin_num, dst_subbsn)
                    dst_downstream_hs = dst_hillids[downstream_merge_idx[idx]]
                    for _id in sp_seq:
                        dst_unit = _id * dst_downstream_hs
                        if dst_unit in all_units_ids:
                            cur_units_pair[_id * hillid] = dst_unit
                units_pair.update(cur_units_pair)

        print ('Slope position unit merge-destination pairs: ' + units_pair.__str__())

        # begin to merge
        for srcid, dstid in units_pair.iteritems():
            # replace values in slope position units raster
            for k, v in units_pair.iteritems():
                self.slppos_ids[self.slppos_ids == k] = v

            for sp in self.slppos_tags:
                name = self.slppos_tags.get(sp).get('name')
                if srcid in self.units_updwon.get(name).keys():  # dstid MUST be here too.
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
            cur_units_count = len(self.units_updwon.get(name).keys())
            self.units_updwon.get('overview')[name] = cur_units_count
            units_count += cur_units_count
        self.units_updwon.get('overview')['all_units'] = units_count

    def extract_subbasin_hillslope_slppos(self):
        """Extract the hierarchical relationship of spatial units, i.e.,
           Subbasin-Hillslope-Slope position
        """
        self.hierarchy_units = dict()
        for sp, spdict in self.slppos_tags.iteritems():
            name = spdict.get('name')
            for unitid, unitdict in self.units_updwon.get(name).iteritems():
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
        f = open(jfile, 'w')
        f.write(json_updown_data)
        f.close()
        RasterUtilClass.write_gtiff_file(unitraster, self.nrows, self.ncols,
                                         self.slppos_ids, self.geotrans, self.srs,
                                         DEFAULT_NODATA, self.datatype)
        VectorUtilClass.raster2shp(unitraster, unitshp)
        print ("Original unique spatial units ID raster saved as '%s'" % unitraster)

    def run(self):
        """Workflow."""
        self.extract_subbasin_updown()
        self.assign_uniqueid_slppos_units()
        self.check_unit_id()
        self.output(self.json_units_origin, self.outf_units_origin, self.outshp_units_origin)
        self.merge_slopeposition_units()
        self.extract_subbasin_hillslope_slppos()
        self.output(self.json_units_merged, self.outf_units_merged, self.outshp_units_merged)


def main():
    """Delineation slope position units with the associated information."""
    # inputs from preprocess
    from config import parse_ini_configuration
    seims_cfg = parse_ini_configuration()

    reach_shp = seims_cfg.vecs.reach
    hillslp_file = seims_cfg.spatials.hillslope
    landuse_file = seims_cfg.spatials.landuse

    # additional inputs
    slppos_tag_name = [(1, 'summit'), (4, 'backslope'), (16, 'valley')]
    slppos_file = r'C:\z_data_m\SEIMS2017\fuzslppos_ywz10m\slope_position_units\SLOPEPOSITION.tif'

    obj = SlopePositionUnits(slppos_tag_name, slppos_file, reach_shp, hillslp_file, landuse_file)
    obj.run()


if __name__ == '__main__':
    main()
