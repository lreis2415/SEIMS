#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Scenario for optimizing BMPs based on slope position units.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-10-29  hr - initial implementation.\n
                17-08-18  lj - redesign and rewrite.\n
"""
import os
import random
import time
from struct import unpack

import numpy
from gridfs import GridFS
from osgeo import osr
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import FileClass, StringClass, get_config_parser
from pymongo.errors import NetworkTimeout

from config import SASPUConfig
from preprocess.db_mongodb import ConnectMongoDB
from preprocess.text import DBTableNames, RasterMetadata
from preprocess.utility import sum_outlet_output
from scenario_analysis.scenario import Scenario


class SPScenario(Scenario):
    """Scenario analysis based on slope position units."""

    def __init__(self, cf):
        """Initialization."""
        Scenario.__init__(self, cf)
        self.gene_num = cf.slppos_unit_num
        self.gene_values = [0] * self.gene_num  # 0 means not config BMP
        self.unit2gene = cf.slppos_to_gene
        self.slppos_tagnames = cf.slppos_tagnames
        self.units_infos = cf.units_infos
        self.bmp_ids = cf.bmps_subids
        self.bmp_params = cf.bmps_params
        self.suit_bmps = cf.slppos_suit_bmps
        self.unit_to_gene = cf.slppos_to_gene
        self.gene_to_unit = cf.gene_to_slppos
        self.cfg_years = cf.runtime_years

    def rule_based_config(self, conf_rate=0.5):
        # from the bottom slope position of each hillslope, trace upslope
        #    to config BMPs.
        # Method 1: Config each slope position unit by corresponding suitable BMPs separately.
        # Method 2: If downslope unit is not configured, the upslope unit should choose one BMP.
        # Method 3: Based method 2, the base scheme shoule be upBMPID <= midBMPID <= downBMPID.
        def config_bmp_for_unit(unit_id, slppostag, upsid, upgid, downsid, downgid, method=1):
            """Config suitable BMP for the given slope position unit.
            Args:
                unit_id(int): Slope position unit ID
                slppostag(int): Slope positon tag, e.g. 1, 4, 16
                force(boolean): Force to config BMP
                method(int): Domain knowledge based rule method.
            Returns:
                If configured, return (True, BMPID), otherwise return (False, 0).
            """
            # print ('slppos: %d, unit: %d' % (slppostag, unit_id))
            genidx = self.unit_to_gene[unit_id]

            bmps = get_potential_bmps(self.suit_bmps, slppostag, upsid, upgid,
                                      downsid, downgid, method)
            # print ('Config for unit: %d' % unitid)
            configed = False
            cfg_bmp = 0
            if random.random() > conf_rate:
                # Do not config BMP according to probability
                # But if no-BMP (i.e., 0) is not allowed for current unit,
                #    it is forced to config BMP.
                if 0 in bmps:
                    return configed, cfg_bmp

            # config BMP
            if len(bmps) >= 1:
                cfg_bmp = bmps[random.randint(0, len(bmps) - 1)]
                self.gene_values[genidx] = cfg_bmp
                if cfg_bmp != 0:
                    configed = True
            # print ('Config for unit: %d, slppos: %d, upgv: %d, downgv: %d, potBMPs: %s, '
            #                'select: %d' % (unit_id, slppostag, upgid, downgid, bmps.__str__(), cfg_bmp))
            # else:
            #     print ('No suitable BMP for unit: %d, slppos: %d, upgv: %d, downgv: %d'
            #            % (unit_id, slppostag, upgid, downgid))
            return configed, cfg_bmp

        spname = self.slppos_tagnames[-1][1]  # the bottom slope position
        for unitid, spdict in self.units_infos[spname].iteritems():
            up_spid = spdict['upslope']
            down_spid = spdict['downslope']
            spidx = len(self.slppos_tagnames) - 1
            sptag = self.slppos_tagnames[spidx][0]
            up_gv = 0
            down_gv = -1
            config_bmp_for_unit(unitid, sptag, up_spid, up_gv, down_spid, down_gv, self.rule_mtd)
            while True:  # trace upstream units
                if up_spid < 0:
                    break
                unitid = up_spid
                up_gv = -1
                down_gv = -1
                spidx -= 1
                sptag = self.slppos_tagnames[spidx][0]
                spname = self.slppos_tagnames[spidx][1]
                spdict = self.units_infos[spname][up_spid]
                up_spid = spdict['upslope']
                down_spid = spdict['downslope']
                if up_spid >= 0:
                    up_gv = self.gene_values[self.unit_to_gene[up_spid]]
                if down_spid >= 0:
                    down_gv = self.gene_values[self.unit_to_gene[down_spid]]
                config_bmp_for_unit(unitid, sptag, up_spid, up_gv,
                                    down_spid, down_gv, self.rule_mtd)

    def random_based_config(self, conf_rate=0.5):
        pot_bmps = self.bmp_ids[:]
        if 0 not in pot_bmps:
            pot_bmps.append(0)
        for i in range(self.gene_num):
            if random.random() >= conf_rate:
                continue
            self.gene_values[i] = pot_bmps[random.randint(0, len(pot_bmps) - 1)]

    def decoding(self):
        if self.ID < 0:
            self.set_unique_id()
        bmp_units = dict()
        for i, gene_v in enumerate(self.gene_values):
            if gene_v == 0:
                continue
            if gene_v not in bmp_units:
                bmp_units[gene_v] = list()
            unit_id = self.gene_to_unit[i]
            bmp_units[gene_v].append(unit_id)
        sce_item_count = 0
        for k, v in bmp_units.iteritems():
            # obj = bson.objectid.ObjectId()
            curd = dict()
            curd['BMPID'] = self.bmps_info['BMPID']
            curd['NAME'] = 'S%d' % self.ID
            curd['COLLECTION'] = self.bmps_info['COLLECTION']
            curd['DISTRIBUTION'] = self.bmps_info['DISTRIBUTION']
            curd['LOCATION'] = '-'.join(str(uid) for uid in v)
            curd['SUBSCENARIO'] = k
            curd['ID'] = self.ID
            self.bmp_items[sce_item_count] = curd
            sce_item_count += 1
        # if BMPs_retain is not empty, append it.
        if len(self.bmps_retain) > 0:
            for k, v in self.bmps_retain.iteritems():
                # obj = bson.objectid.ObjectId()
                curd = v
                curd['NAME'] = 'S%d' % self.ID
                curd['ID'] = self.ID
                self.bmp_items[sce_item_count] = curd
                sce_item_count += 1

    def import_from_mongodb(self, sid):
        pass

    def import_from_txt(self, sid):
        pass

    def calculate_economy(self):
        self.economy = 0.
        capex = 0.
        opex = 0.
        income = 0.
        for idx, gene_v in enumerate(self.gene_values):
            if gene_v == 0:
                continue
            unit_id = self.gene_to_unit[idx]
            unit_lu = dict()
            for spname, spunits in self.units_infos.iteritems():
                if unit_id in spunits:
                    unit_lu = spunits[unit_id]['landuse']
                    break
            bmpparam = self.bmp_params[gene_v]
            for luid, luarea in unit_lu.iteritems():
                if luid in bmpparam['LANDUSE'] or bmpparam['LANDUSE'] is None:
                    capex += luarea * bmpparam['CAPEX']
                    opex += luarea * bmpparam['OPEX'] * self.cfg_years
                    income += luarea * bmpparam['INCOME'] * self.cfg_years

        # self.economy = capex
        # self.economy = capex + opex
        self.economy = capex + opex - income

    def calculate_environment(self):
        if not self.modelrun:  # no evaluate done
            self.economy = self.worst_econ
            self.environment = self.worst_env
            return
        rfile = self.modelout_dir + os.sep + self.bmps_info['ENVEVAL']

        if not FileClass.is_file_exists(rfile):
            time.sleep(5)  # sleep 5 seconds wait for the ouput
        if not FileClass.is_file_exists(rfile):
            print ('WARNING: Although SEIMS model runs successfully, the desired output: %s'
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
        elif StringClass.string_match(rfile.split('.')[-1], 'txt'):  # Time series data
            sed_sum = sum_outlet_output(rfile)
            self.environment = (base_amount - sed_sum) / base_amount
        else:
            self.economy = self.worst_econ
            self.environment = self.worst_env
            return

    def export_scenario_to_gtiff(self, outpath=None):
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
            client = ConnectMongoDB(self.hostname, self.port)
            conn = client.get_conn()
            maindb = conn[self.main_db]
            spatial_gfs = GridFS(maindb, DBTableNames.gridfs_spatial)
            # read file from mongodb
            if not spatial_gfs.exists(filename=dist_name):
                print ('WARNING: %s is not existed, export scenario failed!' % dist_name)
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
            if isinstance(srs, unicode):
                srs = srs.encode()
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
                v_dict[self.gene_to_unit[idx]] = gene_v

            for k, v in v_dict.iteritems():
                slppos_data[slppos_data == k] = v
            if outpath is None:
                outpath = self.scenario_dir + os.sep + 'Scenario_%d.tif' % self.ID
            RasterUtilClass.write_gtiff_file(outpath, ysize, xsize, slppos_data, geotransform,
                                             srs, nodata_value)
            client.close()


def get_potential_bmps(suitbmps, sptag, up_sid, up_gvalue, down_sid, down_gvalue, method=1):
    bmps = suitbmps[sptag][:]
    bmps = list(set(bmps))
    if 0 not in bmps:
        bmps.append(0)
    if method == 1:  # Without any special rule
        pass
    elif method == 2:
        # If not bottom slppos and the downslope unit is configured BMP, then remove 0
        if down_sid > 0 and down_gvalue == 0:
            bmps.remove(0)
    elif method == 3:
        new_bmps = list()
        if up_sid < 0 and down_gvalue > 0:  # the top slppos, and downslope with BMP
            for _bid in bmps:
                if _bid <= down_gvalue:
                    new_bmps.append(_bid)
        elif down_sid < 0 and up_gvalue > 0:  # the bottom slppos, and upslope with BMP
            for _bid in bmps:
                if up_gvalue <= _bid:
                    new_bmps.append(_bid)
        elif down_sid > 0 and up_sid > 0:  # middle slppos
            for _bid in bmps:
                if down_gvalue == 0 and up_gvalue <= _bid:
                    new_bmps.append(_bid)
                elif up_gvalue <= _bid <= down_gvalue:
                    new_bmps.append(_bid)
        else:  # Do nothing
            pass
        if len(new_bmps) > 0:
            bmps = list(set(new_bmps))
    return bmps


def initialize_scenario(cf):
    sce = SPScenario(cf)
    return sce.initialize()


def scenario_effectiveness(cf, individual):
    # 1. instantiate the inherited Scenario class.
    sce = SPScenario(cf)
    curid = sce.set_unique_id()
    setattr(sce, 'gene_values', individual)
    # 2. decoding gene values to BMP items and exporting to MongoDB.
    sce.decoding()
    sce.export_to_mongodb()
    # 3. execute SEIMS model
    sce.execute_seims_model()
    # 4. calculate scenario effectiveness
    sce.calculate_economy()
    sce.calculate_environment()
    # 5. Export scenarios information
    sce.export_scenario_to_txt()
    sce.export_scenario_to_gtiff()

    return sce.economy, sce.environment, curid


def main():
    """TEST CODE"""
    cf = get_config_parser()
    cfg = SASPUConfig(cf)

    # print (cfg.gene_to_slppos)
    # print (cfg.slppos_suit_bmps)

    cost = list()
    for i in range(100):
        init_gene_values = initialize_scenario(cfg)
        # print (init_gene_values.__str__())
        sce = SPScenario(cfg)
        curid = sce.set_unique_id()
        setattr(sce, 'gene_values', init_gene_values)
        sce.calculate_economy()
        cost.append(sce.economy)
    print (max(cost), min(cost), sum(cost) / len(cost))

    # import numpy
    #
    # re_genes = numpy.reshape(init_gene_values, (len(init_gene_values) / 3, 3))
    # print (re_genes)
    # econ, env, sceid = scenario_effectiveness(cfg, init_gene_values)
    # print ('Scenario %d: %s\n' % (sceid, ', '.join(str(v) for v in init_gene_values)))
    # print ('Effectiveness:\n\teconomy: %f\n\tenvironment: %f\n' % (econ, env))


if __name__ == '__main__':
    main()
