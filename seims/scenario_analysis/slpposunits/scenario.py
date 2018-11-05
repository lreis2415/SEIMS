#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Scenario for optimizing BMPs based on slope position units.
    @author   : Liangjun Zhu, Huiran Gao

    @changelog:

    - 16-10-29  hr - initial implementation.
    - 17-08-18  lj - redesign and rewrite.
    - 18-02-09  lj - compatible with Python3.
"""
from __future__ import absolute_import, division
from future.utils import viewitems

import array
from copy import deepcopy
import os
import sys
import random
import time
from struct import unpack

from typing import List, Dict, Optional, Any
import numpy
from gridfs import GridFS
from osgeo import osr
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import FileClass, StringClass, UtilClass, get_config_parser, is_string
from pymongo.errors import NetworkTimeout

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '../..')))

from utility import read_simulation_from_txt
from preprocess.db_mongodb import ConnectMongoDB
from preprocess.text import DBTableNames, RasterMetadata
from scenario_analysis import _DEBUG
from scenario_analysis.scenario import Scenario
from scenario_analysis.slpposunits.config import SASPUConfig


class SPScenario(Scenario):
    """Scenario analysis based on slope position units."""

    def __init__(self, cf):
        # type: (SASPUConfig) -> None
        """Initialization."""
        Scenario.__init__(self, cf)
        self.cfg = cf  # type: SASPUConfig
        self.gene_num = cf.slppos_unit_num  # type: int
        self.gene_values = [0] * self.gene_num  # type: List[int] # 0 means no BMP

        self.bmps_params = dict()  # type: Dict[int, Any] # {bmp_id: {...}}
        self.suit_bmps = dict()  # type: Dict[int, List[int]] # {slppos_id: [bmp_ids]}
        self.bmps_grade = dict()  # type: Dict[int, int] # {slppos_id: effectiveness_grade}

        self.read_bmp_parameters()
        self.get_suitable_bmps_for_slppos()

    def read_bmp_parameters(self):
        """Read BMP configuration from MongoDB.
        Each BMP is stored in Collection as one item identified by 'SUBSCENARIO' field,
        so the `self.bmps_params` is dict with BMP_ID ('SUBSCENARIO') as key.
        """
        client = ConnectMongoDB(self.modelcfg.host, self.modelcfg.port)
        conn = client.get_conn()
        scenariodb = conn[self.scenario_db]

        bmpcoll = scenariodb[self.cfg.bmps_coll]
        findbmps = bmpcoll.find({}, no_cursor_timeout=True)
        for fb in findbmps:
            fb = UtilClass.decode_strs_in_dict(fb)
            if 'SUBSCENARIO' not in fb:
                continue
            curid = fb['SUBSCENARIO']
            if curid not in self.cfg.bmps_subids:
                continue
            if curid not in self.bmps_params:
                self.bmps_params[curid] = dict()
            for k, v in fb.items():
                if k == 'SUBSCENARIO':
                    continue
                elif k == 'LANDUSE':
                    if isinstance(v, int):
                        v = [v]
                    elif v == 'ALL' or v == '':
                        v = None
                    else:
                        v = StringClass.extract_numeric_values_from_string(v)
                        v = [int(abs(nv)) for nv in v]
                    self.bmps_params[curid][k] = v[:]
                elif k == 'SLPPOS':
                    if isinstance(v, int):
                        v = [v]
                    elif v == 'ALL' or v == '':
                        v = list(self.cfg.slppos_tags.keys())
                    else:
                        v = StringClass.extract_numeric_values_from_string(v)
                        v = [int(abs(nv)) for nv in v]
                    self.bmps_params[curid][k] = v[:]
                else:
                    self.bmps_params[curid][k] = v
        client.close()

    def get_suitable_bmps_for_slppos(self):
        """Construct the suitable BMPs for each slope position."""
        for bid, bdict in self.bmps_params.items():
            if 'SLPPOS' not in bdict:
                continue
            suitsp = bdict['SLPPOS']
            for sp in suitsp:
                if sp not in self.suit_bmps:
                    self.suit_bmps[sp] = [bid]
                elif bid not in self.suit_bmps[sp]:
                    self.suit_bmps[sp].append(bid)
            if 'EFFECTIVENESS' in bdict:
                self.bmps_grade[bid] = bdict['EFFECTIVENESS']

    def rule_based_config(self, method, conf_rate=0.5):
        # type: (float, str) -> None
        """Config available BMPs on each spatial units by rule-based method.
        From the bottom slope position of each hillslope, trace upslope.

        The available rule methods are 'SUIT', 'UPDOWN', and 'SLPPOS'.

        See Also:
            :obj:`scenario_analysis.BMPS_RULE_METHODS`
        """

        def config_bmp_for_unit(unit_id, slppostag, upsid, upgid, downsid, downgid,
                                method='SUIT', bmpgrades=None):
            # type: (int, int, int, int, int, int, str, Optional[Dict[int, int]]) -> (bool, int)
            """Config suitable BMP for the given slope position unit.

            Args:
                unit_id(int): Slope position unit ID
                slppostag(int): Slope position tag, e.g. 1, 4, 16
                upsid(int): Upslope position ID
                upgid(int): Gene value of upslope position
                downsid(int): Downslope position ID
                downgid(int): Gene value of downslope position
                method(str): Knowledge-based rule method
                bmpgrades(dict): (Optional) Effectiveness grades of BMPs

            Returns:
                If configured, return (True, BMPID), otherwise return (False, 0).

            See Also:
                :func:`scenario_analysis.slpposunits.scenario.get_potential_bmps`
                :obj:`scenario_analysis.BMPS_RULE_METHODS`
            """
            # print('slppos: %d, unit: %d' % (slppostag, unit_id))
            bmps = get_potential_bmps(self.suit_bmps, slppostag, upsid, upgid,
                                      downsid, downgid, method, bmpgrades)
            # print('Config for unit: %d' % unitid)
            configured = False
            cfg_bmp = 0
            if random.random() > conf_rate:
                # Do not config BMP according to probability
                # But if no-BMP (i.e., 0) is not allowed for current unit,
                #    it is forced to config BMP.
                if 0 in bmps:
                    return configured, cfg_bmp

            # config BMP
            if len(bmps) >= 1:
                cfg_bmp = bmps[random.randint(0, len(bmps) - 1)]
                genidx = self.cfg.slppos_to_gene[unit_id]
                self.gene_values[genidx] = cfg_bmp
                if cfg_bmp != 0:
                    configured = True
            if configured and _DEBUG:
                print('Config for unit: %d, slppos: %d, upgv: %d, downgv: %d,'
                      ' potBMPs: %s, select: %d' % (unit_id, slppostag, upgid, downgid,
                                                    bmps.__str__(), cfg_bmp))
            return configured, cfg_bmp

        spname = self.cfg.slppos_tagnames[-1][1]  # bottom slope position name, e.g., 'valley'
        for unitid, spdict in viewitems(self.cfg.units_infos[spname]):
            up_spid = spdict['upslope']
            down_spid = spdict['downslope']
            spidx = len(self.cfg.slppos_tagnames) - 1
            sptag = self.cfg.slppos_tagnames[spidx][0]  # type: int
            up_gv = 0  # the upslope position has not configured
            down_gv = -1  # -1 indicate there is no downslope position
            config_bmp_for_unit(unitid, sptag, up_spid, up_gv, down_spid, down_gv,
                                self.rule_mtd, self.bmps_grade)
            while True:  # trace upstream units
                if up_spid < 0:
                    break
                unitid = up_spid
                up_gv = -1
                down_gv = -1
                spidx -= 1
                sptag = self.cfg.slppos_tagnames[spidx][0]
                spname = self.cfg.slppos_tagnames[spidx][1]
                spdict = self.cfg.units_infos[spname][up_spid]
                up_spid = spdict['upslope']
                down_spid = spdict['downslope']
                if up_spid >= 0:
                    up_gv = self.gene_values[self.cfg.slppos_to_gene[up_spid]]
                if down_spid >= 0:
                    down_gv = self.gene_values[self.cfg.slppos_to_gene[down_spid]]
                config_bmp_for_unit(unitid, sptag, up_spid, up_gv,
                                    down_spid, down_gv, self.rule_mtd, self.bmps_grade)

    def random_based_config(self, conf_rate=0.5):
        # type: (float) -> None
        """Config BMPs on each spatial unit randomly."""
        pot_bmps = self.cfg.bmps_subids[:]
        if 0 not in pot_bmps:
            pot_bmps.append(0)
        for i in range(self.gene_num):
            if random.random() >= conf_rate:
                continue
            self.gene_values[i] = pot_bmps[random.randint(0, len(pot_bmps) - 1)]

    def decoding(self):
        """Decode gene values to Scenario item, i.e., `self.bmp_items`."""
        if self.ID < 0:
            self.set_unique_id()
        if self.bmp_items:
            self.bmp_items.clear()
        bmp_units = dict()  # type: Dict[int, List[int]] # {BMPs_ID: [units list]}
        for i, gene_v in enumerate(self.gene_values):
            if gene_v == 0:
                continue
            if gene_v not in bmp_units:
                bmp_units[gene_v] = list()
            unit_id = self.cfg.gene_to_slppos[i]
            bmp_units[gene_v].append(unit_id)
        sce_item_count = 0
        for k, v in viewitems(bmp_units):
            curd = dict()
            curd['BMPID'] = self.bmps_info['BMPID']
            curd['NAME'] = 'S%d' % self.ID
            curd['COLLECTION'] = self.bmps_info['COLLECTION']
            curd['DISTRIBUTION'] = self.bmps_info['DISTRIBUTION']
            curd['LOCATION'] = '-'.join(repr(uid) for uid in v)
            curd['SUBSCENARIO'] = k
            curd['ID'] = self.ID
            self.bmp_items[sce_item_count] = curd
            sce_item_count += 1
        # if BMPs_retain is not empty, append it.
        if len(self.bmps_retain) > 0:
            for k, v in self.bmps_retain.items():
                curd = deepcopy(v)
                curd['NAME'] = 'S%d' % self.ID
                curd['ID'] = self.ID
                self.bmp_items[sce_item_count] = curd
                sce_item_count += 1

    def import_from_mongodb(self, sid):
        pass

    def import_from_txt(self, sid):
        pass

    def calculate_economy(self):
        """Calculate economic benefit by simple cost-benefit model, see Qin et al. (2018)."""
        self.economy = 0.
        capex = 0.
        opex = 0.
        income = 0.
        for idx, gene_v in enumerate(self.gene_values):
            if gene_v == 0:
                continue
            unit_id = self.cfg.gene_to_slppos[idx]
            unit_lu = dict()
            for spname, spunits in self.cfg.units_infos.items():
                if unit_id in spunits:
                    unit_lu = spunits[unit_id]['landuse']
                    break
            bmpparam = self.bmps_params[gene_v]
            for luid, luarea in unit_lu.items():
                if luid in bmpparam['LANDUSE'] or bmpparam['LANDUSE'] is None:
                    capex += luarea * bmpparam['CAPEX']
                    opex += luarea * bmpparam['OPEX'] * self.cfg.runtime_years
                    income += luarea * bmpparam['INCOME'] * self.cfg.runtime_years

        # self.economy = capex
        # self.economy = capex + opex
        self.economy = capex + opex - income
        return self.economy

    def calculate_environment(self):
        """Calculate environment benefit based on the output and base values predefined in
        configuration file.
        """
        if not self.modelrun:  # no evaluate done
            self.economy = self.worst_econ
            self.environment = self.worst_env
            return
        rfile = self.modelout_dir + os.path.sep + self.bmps_info['ENVEVAL']

        if not FileClass.is_file_exists(rfile):
            time.sleep(0.5)  # sleep 0.5 seconds wait for the outputs
        if not FileClass.is_file_exists(rfile):
            print('WARNING: Although SEIMS model has been executed, the desired output: %s'
                  ' cannot be found!' % rfile)
            self.economy = self.worst_econ
            self.environment = self.worst_env
            return

        base_amount = self.bmps_info['BASE_ENV']
        if StringClass.string_match(rfile.split('.')[-1], 'tif'):  # Raster data
            rr = RasterUtilClass.read_raster(rfile)
            soil_erosion_amount = rr.get_sum() / self.timerange  # unit: year
            # reduction rate of soil erosion
            self.environment = (base_amount - soil_erosion_amount) / base_amount
            # print exception values
            if self.environment > 1. or self.environment < 0.:
                print('Exception Information: Scenario ID: %d, '
                      'SUM(%s): %s' % (self.ID, rfile, soil_erosion_amount))
                self.environment = self.worst_env
        elif StringClass.string_match(rfile.split('.')[-1], 'txt'):  # Time series data
            sed_sum = read_simulation_from_txt(self.modelout_dir,
                                               ['SED'], self.model.OutletID,
                                               self.cfg.eval_stime, self.cfg.eval_etime)
            self.environment = (base_amount - sed_sum) / base_amount
        else:
            self.economy = self.worst_econ
            self.environment = self.worst_env
            return

    def export_scenario_to_gtiff(self, outpath=None):
        # type: (Optional[str]) -> None
        """Export scenario to GTiff.

        TODO: Read Raster from MongoDB should be extracted to pygeoc.
        """
        if not self.export_sce_tif:
            return
        dist = self.bmps_info['DISTRIBUTION']
        dist_list = StringClass.split_string(dist, '|')
        if len(dist_list) >= 2 and dist_list[0] == 'RASTER':
            dist_name = '0_' + dist_list[1]  # prefix 0_ means the whole basin
            # read dist_name from MongoDB
            client = ConnectMongoDB(self.modelcfg.host, self.modelcfg.port)
            conn = client.get_conn()
            maindb = conn[self.modelcfg.db_name]
            spatial_gfs = GridFS(maindb, DBTableNames.gridfs_spatial)
            # read file from mongodb
            if not spatial_gfs.exists(filename=dist_name):
                print('WARNING: %s is not existed, export scenario failed!' % dist_name)
                return
            try:
                slpposf = maindb[DBTableNames.gridfs_spatial].files.find({'filename': dist_name},
                                                                         no_cursor_timeout=True)[0]
            except NetworkTimeout or Exception:
                # In case of unexpected raise
                client.close()
                return

            ysize = int(slpposf['metadata'][RasterMetadata.nrows])
            xsize = int(slpposf['metadata'][RasterMetadata.ncols])
            xll = slpposf['metadata'][RasterMetadata.xll]
            yll = slpposf['metadata'][RasterMetadata.yll]
            cellsize = slpposf['metadata'][RasterMetadata.cellsize]
            nodata_value = slpposf['metadata'][RasterMetadata.nodata]
            srs = slpposf['metadata'][RasterMetadata.srs]
            if is_string(srs):
                srs = str(srs)
            srs = osr.GetUserInputAsWKT(srs)
            geotransform = [0] * 6
            geotransform[0] = xll - 0.5 * cellsize
            geotransform[1] = cellsize
            geotransform[3] = yll + (ysize - 0.5) * cellsize  # yMax
            geotransform[5] = -cellsize

            slppos_data = spatial_gfs.get(slpposf['_id'])
            total_len = xsize * ysize
            fmt = '%df' % (total_len,)
            slppos_data = unpack(fmt, slppos_data.read())
            slppos_data = numpy.reshape(slppos_data, (ysize, xsize))

            v_dict = dict()
            for idx, gene_v in enumerate(self.gene_values):
                v_dict[self.cfg.gene_to_slppos[idx]] = gene_v

            for k, v in v_dict.items():
                slppos_data[slppos_data == k] = v
            if outpath is None:
                outpath = self.scenario_dir + os.path.sep + 'Scenario_%d.tif' % self.ID
            RasterUtilClass.write_gtiff_file(outpath, ysize, xsize, slppos_data, geotransform,
                                             srs, nodata_value)
            client.close()


def get_potential_bmps(suitbmps, sptag, up_sid, up_gvalue, down_sid, down_gvalue,
                       method='SUIT', bmpgrades=None):
    # type: (Dict[int, List[int]], int, int, int, int, int, str, Optional[Dict[int, int]]) -> List[int]
    """Get potential BMPs based on the specific knowledge-based rule method.

    Args:
        suitbmps(dict): All available BMPs IDs of each slope position
        sptag(int): Slope position tag, e.g. 1, 4, 16
        up_sid(int): Upslope position ID
        up_gvalue(int): Gene value of upslope position
        down_sid(int): Downslope position ID
        down_gvalue(int): Gene value of downslope position
        method(str): Knowledge-based rule method.
        bmpgrades(dict): (Optional) Effectiveness grades of BMPs

    Returns:
        Potential BMPs IDs.
    """
    bmps = suitbmps[sptag][:]
    bmps = list(set(bmps))  # ascending
    if bmpgrades is None:  # By default, the effectiveness grade should be equal for all BMPs.
        bmpgrades = {bid: 1 for bid in bmps}
    if 0 not in bmps:
        bmps.append(0)
    if 0 not in bmpgrades:
        bmpgrades[0] = 0
    if method == 'SUIT':  # Without any special rule
        pass
    elif method == 'UPDOWN':
        # If not bottom slppos and the downslope unit is configured BMP, then remove 0
        if down_sid > 0 and down_gvalue == 0:
            bmps.remove(0)
    elif method == 'SLPPOS':
        up_grade = bmpgrades[up_gvalue] if up_gvalue in bmpgrades else 0
        down_grade = bmpgrades[down_gvalue] if down_gvalue in bmpgrades else 0
        new_bmps = list()
        if up_sid < 0 < down_gvalue:  # 1. the top slppos, and downslope with BMP
            for _bid, _bgrade in viewitems(bmpgrades):
                if _bgrade <= down_grade:
                    new_bmps.append(_bid)
        elif down_sid < 0 < up_gvalue:  # 2. the bottom slppos, and upslope with BMP
            for _bid, _bgrade in viewitems(bmpgrades):
                if up_grade <= _bgrade:
                    new_bmps.append(_bid)
        elif down_sid > 0 and up_sid > 0:  # 3. middle slppos
            for _bid, _bgrade in viewitems(bmpgrades):
                if down_gvalue == 0 and up_gvalue <= _bgrade:  # 3.1. downslope no BMP
                    new_bmps.append(_bid)
                elif up_grade <= _bgrade <= down_grade:  # 3.2. downslope with BMP
                    new_bmps.append(_bid)
        else:  # Do nothing
            pass
        if len(new_bmps) > 0:
            bmps = list(set(new_bmps))
    else:
        pass
    return bmps


def initialize_scenario(cf):
    # type: (SASPUConfig) -> List[int]
    """Initialize gene values"""
    sce = SPScenario(cf)
    return sce.initialize()


def scenario_effectiveness(cf, ind):
    # type: (SASPUConfig, array.array) -> (float, float, int)
    """Run SEIMS-based model and calculate economic and environmental effectiveness."""
    # 1. instantiate the inherited Scenario class.
    sce = SPScenario(cf)
    curid = sce.set_unique_id()
    setattr(sce, 'gene_values', ind)
    # 2. decoding gene values to BMP items and exporting to MongoDB.
    sce.decoding()
    sce.export_to_mongodb()
    # 3. execute SEIMS model
    sce.execute_seims_model()
    # Get timespan
    ind.io_time, ind.comp_time, ind.simu_time, ind.runtime = sce.model.GetTimespan()
    # 4. calculate scenario effectiveness
    sce.calculate_economy()
    sce.calculate_environment()
    # 5. Export scenarios information
    sce.export_scenario_to_txt()
    sce.export_scenario_to_gtiff()

    return sce.economy, sce.environment, curid


def main_multiple(eval_num):
    # type: (int) -> None
    """Test of multiple evaluations of scenarios."""
    cf = get_config_parser()
    cfg = SASPUConfig(cf)

    cost = list()
    for _ in range(eval_num):
        sce = SPScenario(cfg)
        sce.initialize()
        sceid = sce.set_unique_id()
        print(sceid, sce.gene_values.__str__())
        sce.calculate_economy()
        cost.append(sce.economy)
    print(max(cost), min(cost), sum(cost) / len(cost))


def main_single():
    """Test of single evaluation of scenario."""
    cf = get_config_parser()
    cfg = SASPUConfig(cf)
    sce = SPScenario(cfg)
    sce.initialize()
    sceid = sce.set_unique_id()
    print(sceid, sce.gene_values.__str__())
    sce.decoding()
    sce.export_to_mongodb()
    sce.execute_seims_model()
    sce.calculate_economy()
    sce.calculate_environment()

    print('Scenario %d: %s\n' % (sceid, ', '.join(repr(v) for v in sce.gene_values)))
    print('Effectiveness:\n\teconomy: %f\n\tenvironment: %f\n' % (sce.economy, sce.environment))


def main_manual():
    """Test of set scenario manually."""
    cf = get_config_parser()
    cfg = SASPUConfig(cf)
    sce = SPScenario(cfg)

    sceid = 200206028
    sce.set_unique_id(sceid)
    gene_values = [0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 1.0, 0.0, 4.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 3.0, 0.0, 1.0, 3.0, 0.0, 1.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 1.0, 1.0, 4.0, 1.0, 0.0, 0.0]

    sce.set_gene_values(gene_values)

    sce.decoding()
    sce.export_to_mongodb()
    sce.execute_seims_model()
    sce.export_sce_tif = True
    sce.export_scenario_to_gtiff(sce.model.OutputDirectory + os.sep + 'scenario_%d.tif' % sceid)
    sce.calculate_economy()
    sce.calculate_environment()

    print('Scenario %d: %s\n' % (sceid, ', '.join(repr(v) for v in sce.gene_values)))
    print('Effectiveness:\n\teconomy: %f\n\tenvironment: %f\n' % (sce.economy, sce.environment))


if __name__ == '__main__':
    main_manual()
    # main_single()
    # main_multiple(4)
