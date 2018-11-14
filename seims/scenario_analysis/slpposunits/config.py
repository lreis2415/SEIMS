#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Configuration BMPs optimization based on various configuration units.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:

    - 16-12-30  hr - initial implementation.
    - 17-08-18  lj - reorganize as basic class.
    - 18-02-09  lj - compatible with Python3.
    - 18-11-01  lj - Config class should not do extra operation, e.g., read database.
    - 18-11-06  lj - Add supports of other BMPs configuration units.
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

from typing import List, Tuple, Union, Dict, AnyStr
from pygeoc.utils import FileClass, UtilClass, get_config_parser

from scenario_analysis import BMPS_CFG_UNITS
from scenario_analysis.config import SAConfig


class SACommUnitConfig(SAConfig):
    """Configuration of scenario analysis based on common spatial units without topology info.

    Attributes:
        units_num(int): Spatial units number.
    """

    def __init__(self, cf):
        # type: (ConfigParser) -> None
        """Initialization."""
        SAConfig.__init__(self, cf)  # initialize base class first
        # 1. Check the required key and values
        requiredkeys = ['COLLECTION', 'DISTRIBUTION', 'SUBSCENARIO',
                        'ENVEVAL', 'BASE_ENV', 'UNITJSON']
        for k in requiredkeys:
            if k not in self.bmps_info:
                raise ValueError('%s: MUST be provided in BMPs_cfg_units or BMPs_info!' % k)
        # 2. Slope position units information
        units_json = self.bmps_info.get('UNITJSON')
        unitsf = self.model.model_dir + os.sep + units_json
        if not FileClass.is_file_exists(unitsf):
            raise Exception('UNITJSON file %s is not existed!' % unitsf)
        with open(unitsf, 'r', encoding='utf-8') as updownfo:
            self.units_infos = json.load(updownfo)
        self.units_infos = UtilClass.decode_strs_in_dict(self.units_infos)
        if 'overview' not in self.units_infos:
            raise ValueError('overview MUST be existed in the UNITJSON file.')
        if 'all_units' not in self.units_infos['overview']:
            raise ValueError('all_units MUST be existed in overview dict of UNITJSON.')
        self.units_num = self.units_infos['overview']['all_units']  # type: int
        # 3. Collection name and subscenario IDs
        self.bmps_coll = self.bmps_info.get('COLLECTION')  # type: str
        self.bmps_subids = self.bmps_info.get('SUBSCENARIO')  # type: List[int]
        # 4. Construct the dict of gene index to unit ID, and unit ID to gene index
        self.unit_to_gene = OrderedDict()  # type: OrderedDict[int, int]
        self.gene_to_unit = dict()  # type: Dict[int, int]

    def construct_indexes_units_gene(self):
        """Construct the indexes between spatial units ID and gene index.
        This function can be override by inherited class.
        """
        if 'units' not in self.units_infos:
            raise ValueError('units MUST be existed in the UNITJSON file.')
        idx = 0
        for uid, udict in viewitems(self.units_infos['units']):
            self.gene_to_unit[idx] = uid
            self.unit_to_gene[uid] = idx
            idx += 1
        assert (idx == self.units_num)


class SAConnFieldConfig(SACommUnitConfig):
    """Configuration of scenario analysis based on hydrologically connected fields."""

    def __init__(self, cf):
        # type: (ConfigParser) -> None
        """Initialization."""
        SACommUnitConfig.__init__(self, cf)  # initialize base class


class SASlpPosConfig(SACommUnitConfig):
    """Configuration of scenario analysis based on Slope Position Units.

    Attributes:
        slppos_tags(dict): Slope position tags and names read from config file.
            e.g., {16: 'valley', 1: 'summit', 4: 'backslope'}
        slppos_tagnames(list): Slope position tags and names along the hillslope sequence.
            e.g., [(1, 'summit'), (4, 'backslope'), (16, 'valley')]

    """

    def __init__(self, cf):
        # type: (ConfigParser) -> None
        """Initialization."""
        # 1. initialize base class
        SACommUnitConfig.__init__(self, cf)
        # 2. Check additional required key and values
        requiredkeys = ['SLPPOS_TAG_NAME']
        for k in requiredkeys:
            if k not in self.bmps_info:
                raise ValueError('%s: MUST be provided in BMPs_cfg_units or BMPs_info '
                                 'for SLPPOS method!' % k)
        # 3. Get slope position sequence
        self.slppos_tags = self.bmps_info.get('SLPPOS_TAG_NAME')  # type: Dict[int, AnyStr]
        self.slppos_tagnames = sorted(list(self.slppos_tags.items()),
                                      key=operator.itemgetter(0))  # type: List[Tuple[int, AnyStr]]

    def construct_indexes_units_gene(self):
        """Override this function for slope position units."""
        # gene index: 0, 1, 2, ..., n
        # slppos units: rdg1, bks2, vly1,..., rdgn, bksn, vlyn
        idx = 0
        spname = self.slppos_tagnames[0][1]  # the top slope position name
        for uid, udict in viewitems(self.units_infos[spname]):
            spidx = 0
            self.gene_to_unit[idx] = uid
            self.unit_to_gene[uid] = idx
            idx += 1
            next_uid = udict['downslope']
            while next_uid > 0:
                self.gene_to_unit[idx] = next_uid
                self.unit_to_gene[next_uid] = idx
                idx += 1
                spidx += 1
                spname = self.slppos_tagnames[spidx][1]
                next_uid = self.units_infos[spname][next_uid]['downslope']

        assert (idx == self.units_num)


if __name__ == '__main__':
    cf = get_config_parser()
    base_cfg = SAConfig(cf)  # type: SAConfig
    if base_cfg.bmps_cfg_unit == BMPS_CFG_UNITS[3]:  # SLPPOS
        cfg = SASlpPosConfig(cf)
    elif base_cfg.bmps_cfg_unit == BMPS_CFG_UNITS[2]:  # CONNFIELD
        cfg = SAConnFieldConfig(cf)
    else:  # Common spatial units, e.g., HRU and UNIQHRU
        cfg = SACommUnitConfig(cf)
    cfg.construct_indexes_units_gene()

    # test the picklable of SASPUConfig class.
    import pickle

    s = pickle.dumps(cfg)
    new_cfg = pickle.loads(s)  # type: Union[SASlpPosConfig, SAConnFieldConfig, SACommUnitConfig]
    print('BMPs configuration units number: %d ' % new_cfg.units_num)
