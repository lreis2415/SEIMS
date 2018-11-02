#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Configuration BMPs optimization based on slope position units.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:

    - 16-12-30  hr - initial implementation.
    - 17-08-18  lj - reorganize as basic class.
    - 18-02-09  lj - compatible with Python3.
    - 18-11-01  lj - Config class should not do extra operation, e.g., read database.
"""
from __future__ import absolute_import, unicode_literals

from future.utils import viewitems
from configparser import ConfigParser
import os
import sys
import json
import operator
from collections import OrderedDict
from io import open

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '../..')))

from typing import List
from pygeoc.utils import FileClass, UtilClass, get_config_parser

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
        # type: (ConfigParser) -> None
        """Initialization."""
        SAConfig.__init__(self, cf)  # initialize base class first
        # Handling self.bmps_info for specific application
        # 1. Check the required key and values
        requiredkeys = ['COLLECTION', 'DISTRIBUTION', 'SUBSCENARIO', 'UPDOWNJSON',
                        'ENVEVAL', 'BASE_ENV']
        for k in requiredkeys:
            if k not in self.bmps_info:
                raise ValueError('[%s]: MUST be provided in BMPs_info!' % k)
        # 2. Slope position units information
        updown_json = self.bmps_info.get('UPDOWNJSON')
        updownf = self.model.model_dir + os.sep + updown_json
        FileClass.check_file_exists(updownf)
        with open(updownf, 'r', encoding='utf-8') as updownfo:
            self.units_infos = json.load(updownfo)
        self.units_infos = UtilClass.decode_strs_in_dict(self.units_infos)
        # 3. Get slope position sequence
        if not cf.has_option('BMPs', 'slppos_tag_name'):
            raise ValueError('slppos_tag_name MUST be provided in BMPs section!')
        sptags = cf.get('BMPs', 'slppos_tag_name')
        self.slppos_tags = json.loads(sptags)
        self.slppos_tags = UtilClass.decode_strs_in_dict(self.slppos_tags)
        self.slppos_tagnames = sorted(list(self.slppos_tags.items()), key=operator.itemgetter(0))
        self.slppos_unit_num = self.units_infos['overview']['all_units']  # type: int

        # gene index: 0, 1, 2, ..., n
        # slppos units: rdg1, bks2, vly1,..., rdgn, bksn, vlyn
        self.slppos_to_gene = OrderedDict()
        self.gene_to_slppos = dict()
        idx = 0
        spname = self.slppos_tagnames[0][1]
        for uid, udict in viewitems(self.units_infos[spname]):
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

        # 4. Collection name and subscenario IDs
        self.bmps_coll = self.bmps_info.get('COLLECTION')  # type: str
        self.bmps_subids = self.bmps_info.get('SUBSCENARIO')  # type: List[int]


if __name__ == '__main__':
    cf = get_config_parser()
    cfg = SASPUConfig(cf)

    # test the picklable of SASPUConfig class.
    import pickle

    s = pickle.dumps(cfg)
    # print(s)
    new_cfg = pickle.loads(s)  # type: SASPUConfig
    print(new_cfg.slppos_unit_num)
    print(new_cfg.slppos_tagnames)
