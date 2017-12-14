#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Configuration BMPs optimization based on slope position units.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-12-30  hr - initial implementation.\n
                17-08-18  lj - reorganize as basic class.\n
"""
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '../..')))

import json
import operator
from collections import OrderedDict

from pygeoc.utils import FileClass, UtilClass, StringClass, get_config_parser

from preprocess.db_mongodb import ConnectMongoDB
from scenario_analysis.config import SAConfig


class SASPUConfig(SAConfig):
    """Configuration of scenario analysis based on slope position units.

    Attributes:
        slppos_tags(dict): Slope position tags and names read from config file.
            e.g., {16: 'valley', 1: 'summit', 4: 'backslope'}
        slppos_tagnames(list): Slope position tags and names along the hillslope sequence.
            e.g., [(1, 'summit'), (4, 'backslope'), (16, 'valley')]

    """

    def __init__(self, cf):
        """Initialization."""
        SAConfig.__init__(self, cf)  # initialize base class first
        # Handling self.bmps_info for specific application
        # 1. Check the required key and values
        requiredkeys = ['COLLECTION', 'DISTRIBUTION', 'SUBSCENARIO', 'UPDOWNJSON',
                        'ENVEVAL', 'BASE_ENV']
        for k in requiredkeys:
            if k not in self.bmps_info:
                raise ValueError('[%s]: MUST be provided!' % k)
        # 2. Slope position units information
        updownf = self.bmps_info.get('UPDOWNJSON')
        FileClass.check_file_exists(updownf)
        updownfo = open(updownf, 'r')
        self.units_infos = json.load(updownfo)
        self.units_infos = UtilClass.decode_strs_in_dict(self.units_infos)
        updownfo.close()
        # 3. Get slope position sequence
        sptags = cf.get('BMPs', 'slppos_tag_name')
        self.slppos_tags = json.loads(sptags)
        self.slppos_tags = UtilClass.decode_strs_in_dict(self.slppos_tags)
        self.slppos_tagnames = sorted(self.slppos_tags.items(), key=operator.itemgetter(0))
        self.slppos_unit_num = self.units_infos['overview']['all_units']
        self.slppos_to_gene = OrderedDict()
        self.gene_to_slppos = dict()

        # method 1: (deprecated)
        #     gene index: 0, 1, 2, ..., n
        #     slppos unit: rdg1, rdg2,..., bks1, bks2,..., vly1, vly2...
        # idx = 0
        # for tag, sp in self.slppos_tagnames:
        #     for uid in self.units_infos[sp]:
        #         self.gene_to_slppos[idx] = uid
        #         self.slppos_to_gene[uid] = idx
        #         idx += 1
        # method 2:
        #     gene index: 0, 1, 2, ..., n
        #     slppos unit: rdg1, bks2, vly1,..., rdgn, bksn, vlyn
        idx = 0
        spname = self.slppos_tagnames[0][1]
        for uid, udict in self.units_infos[spname].iteritems():
            spidx = 0
            self.gene_to_slppos[idx] = uid
            self.slppos_to_gene[uid] = idx
            idx += 1
            next_uid = udict['downslope']
            while next_uid > 0:
                self.gene_to_slppos[idx] = next_uid
                self.slppos_to_gene[next_uid] = idx
                idx += 1
                spidx += 1
                spname = self.slppos_tagnames[spidx][1]
                next_uid = self.units_infos[spname][next_uid]['downslope']

        assert (idx == self.slppos_unit_num)

        # 4. SubScenario IDs and parameters read from MongoDB
        self.bmps_subids = self.bmps_info.get('SUBSCENARIO')
        self.bmps_coll = self.bmps_info.get('COLLECTION')
        self.bmps_params = dict()
        self.slppos_suit_bmps = dict()

        self.read_bmp_parameters()
        self.get_suitable_bmps_for_slppos()

    def read_bmp_parameters(self):
        """Read BMP configuration from MongoDB."""
        client = ConnectMongoDB(self.hostname, self.port)
        conn = client.get_conn()
        scenariodb = conn[self.bmp_scenario_db]

        bmpcoll = scenariodb[self.bmps_coll]
        findbmps = bmpcoll.find({}, no_cursor_timeout=True)
        for fb in findbmps:
            fb = UtilClass.decode_strs_in_dict(fb)
            if 'SUBSCENARIO' not in fb:
                continue
            curid = fb['SUBSCENARIO']
            if curid not in self.bmps_subids:
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
                        v = self.slppos_tags.keys()
                    else:
                        v = StringClass.extract_numeric_values_from_string(v)
                        v = [int(abs(nv)) for nv in v]
                    self.bmps_params[curid][k] = v[:]
                else:
                    self.bmps_params[curid][k] = v

        client.close()

    def get_suitable_bmps_for_slppos(self):
        """Construct the suitable BMPs for each slope position."""
        for bid, bdict in self.bmps_params.iteritems():
            suitsp = bdict['SLPPOS']
            for sp in suitsp:
                if sp not in self.slppos_suit_bmps:
                    self.slppos_suit_bmps[sp] = [bid]
                elif bid not in self.slppos_suit_bmps[sp]:
                    self.slppos_suit_bmps[sp].append(bid)


if __name__ == '__main__':
    cf = get_config_parser()
    cfg = SASPUConfig(cf)

    # test the picklable of SASPUConfig class.
    import pickle

    s = pickle.dumps(cfg)
    # print (s)
    new_cfg = pickle.loads(s)
    print (new_cfg.bmps_params)
