#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Configuration BMPs optimization based on slope position units.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-12-30  hr - initial implementation.\n
                17-08-18  lj - reorganize as basic class.\n
"""
import json

from seims.preprocess.db_mongodb import ConnectMongoDB

try:
    from ConfigParser import ConfigParser  # py2
except ImportError:
    from configparser import ConfigParser  # py3

from seims.pygeoc.pygeoc.utils.utils import FileClass, UtilClass, get_config_file
from seims.scenario_analysis.config import SAConfig


class SASPUConfig(SAConfig):
    """Configuration of scenario analysis based on slope position units."""

    def __init__(self, cf):
        """Initialization."""
        SAConfig.__init__(self, cf)  # initialize base class first
        # Handling self.bmps_info for specific application
        # 1. Check the required key and values
        requiredkeys = ['COLLECTION', 'DISTRIBUTION', 'SUBSCENARIOID', 'UPDOWNJSON',
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
        # 3. SubScenario IDs and parameters read from MongoDB
        self.bmps_subids = self.bmps_info.get('SUBSCENARIOID')
        self.bmps_coll = self.bmps_info.get('COLLECTION')
        self.bmps_params = dict()
        self.read_bmp_parameters()

    def read_bmp_parameters(self):
        client = ConnectMongoDB(self.hostname, self.port)
        conn = client.get_conn()
        scenariodb = conn[self.bmp_scenario_db]

        bmpcoll = scenariodb[self.bmps_coll]
        findbmps = bmpcoll.find({})
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
                self.bmps_params[curid][k] = v

        client.close()


def parse_ini_configuration():
    """Load model configuration from *.ini file"""
    cf = ConfigParser()
    ini_file = get_config_file()
    cf.read(ini_file)
    return SASPUConfig(cf)


if __name__ == '__main__':
    cfg = parse_ini_configuration()
    print (cfg.model_dir)
