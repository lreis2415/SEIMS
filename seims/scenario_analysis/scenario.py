#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base class of Scenario for coupling NSAG-II.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:

    - 16-10-29  hr - initial implementation.
    - 17-08-18  lj - redesign and rewrite.
    - 18-02-09  lj - compatible with Python3.
    - 18-10-30  lj - Update according to new config parser structure.
"""
from __future__ import absolute_import, unicode_literals

from copy import deepcopy
from datetime import timedelta
from io import open
import os
import sys
import random
import shutil
import uuid

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from bson.objectid import ObjectId
from pygeoc.utils import MathClass, get_config_parser
from pymongo.errors import NetworkTimeout
from typing import List, Iterator

from scenario_analysis.config import SAConfig
from preprocess.db_mongodb import ConnectMongoDB
from preprocess.text import DBTableNames
from run_seims import MainSEIMS
from utility.scoop_func import scoop_log


def generate_uniqueid():
    # type: () -> Iterator[int]
    """Generate unique integer ID for Scenario using uuid.

    Usage:
        uniqueid = next(generate_uniqueid())
    """
    uid = int(str(uuid.uuid4().fields[-1])[:9])
    while True:
        yield uid
        uid += 1


def delete_scenarios_by_ids(hostname, port, dbname, sids):
    # type: (str, int, str, List[int]) -> None
    """Delete scenario data by ID in MongoDB."""
    client = ConnectMongoDB(hostname, port)
    conn = client.get_conn()
    db = conn[dbname]
    collection = db[DBTableNames.scenarios]
    for _id in sids:
        collection.remove({'ID': _id})
        print('Delete scenario: %d in MongoDB completed!' % _id)
    client.close()


def delete_model_outputs(model_workdir, hostname, port, dbname):
    # type: (str, str, int, str) -> None
    """Delete model outputs and scenario in MongoDB."""
    f_list = os.listdir(model_workdir)
    sids = list()
    for f in f_list:
        outfilename = model_workdir + os.path.sep + f
        if os.path.isdir(outfilename):
            if len(f) > 9:
                if MathClass.isnumerical(f[-9:]):
                    shutil.rmtree(outfilename)
                    try:
                        sid = int(f[-9:])
                        sids.append(sid)
                    except ValueError:
                        pass
    if len(sids) > 0:
        delete_scenarios_by_ids(hostname, port, dbname, sids)


class Scenario(object):
    """Base class of Scenario Analysis.

    Attributes:
        ID(integer): Unique ID in BMPScenario database -> BMP_SCENARIOS collection
        timerange(float): Simulation time range, read from MongoDB, the unit is year.
        economy(float): Economical effectiveness, e.g., income minus expenses
        environment(float): Environmental effectiveness, e.g., reduction rate of soil erosion
        gene_num(integer): The number of genes of one chromosome, i.e., an individual
        gene_values(list): BMP identifiers on each location of gene. The length is gen_num.
        bmp_items(dict): BMP configuration items that can be imported to MongoDB directly.
                         The key is `bson.objectid.ObjectId`, the value is scenario item dict.
        rules(boolean): Config BMPs randomly or rule-based.
        modelrun(boolean): Has SEIMS model run successfully?
    """

    def __init__(self, cfg):
        # type: (SAConfig) -> None
        """Initialize."""
        self.ID = -1
        self.timerange = 1.  # unit: year
        self.economy = 0.
        self.environment = 0.
        self.worst_econ = cfg.worst_econ
        self.worst_env = cfg.worst_env

        self.gene_num = 0
        self.gene_values = list()  # type: List[int]
        self.bmp_items = dict()

        self.rule_mtd = cfg.bmps_rule_method
        self.bmps_info = cfg.bmps_info
        self.bmps_retain = cfg.bmps_retain
        self.export_sce_txt = cfg.export_sce_txt
        self.export_sce_tif = cfg.export_sce_tif
        self.scenario_dir = cfg.scenario_dir  # predefined directories to store scenarios related

        # SEIMS-based model related
        self.modelcfg = cfg.model
        self.modelcfg_dict = self.modelcfg.ConfigDict
        self.model = MainSEIMS(args_dict=self.modelcfg_dict)
        self.scenario_db = self.model.ScenarioDBName
        # (Re)Calculate timerange in the unit of year
        self.model.ResetSimulationPeriod()
        dlt = self.model.end_time - self.model.start_time + timedelta(seconds=1)
        self.timerange = (dlt.days * 86400. + dlt.seconds) / 86400. / 365.
        self.modelout_dir = None  # determined in `execute_seims_model` based on unique scenario ID
        self.modelrun = False

    def set_unique_id(self):
        # type: () -> int
        """Set unique ID."""
        self.ID = next(generate_uniqueid())
        return self.ID

    def rule_based_config(self, method, conf_rate):
        # type: (float, str) -> None
        """Config available BMPs to each gene of the chromosome by rule-based method.

        Virtual function that should be overridden in inherited Scenario class.
        """
        pass

    def random_based_config(self, conf_rate):
        # type: (float) -> None
        """Config available BMPs to each gene of the chromosome by random-based method.

        Virtual function that should be overridden in inherited Scenario class.
        """
        pass

    def decoding(self):
        """Decoding gene_values to bmp_items

        This function should be overridden.
        """
        pass

    def export_to_mongodb(self):
        """Export current scenario to MongoDB.
        Delete the same ScenarioID if existed.
        """
        client = ConnectMongoDB(self.modelcfg.host, self.modelcfg.port)
        conn = client.get_conn()
        db = conn[self.scenario_db]
        collection = db[DBTableNames.scenarios]
        try:
            # find ScenarioID, remove if existed.
            if collection.find({'ID': self.ID}, no_cursor_timeout=True).count():
                collection.remove({'ID': self.ID})
        except NetworkTimeout or Exception:
            # In case of unexpected raise
            pass
        for objid, bmp_item in self.bmp_items.items():
            bmp_item['_id'] = ObjectId()
            collection.insert_one(bmp_item)
        client.close()

    def export_scenario_to_txt(self):
        """Export current scenario information to text file.

        This function is better be called after `calculate_environment` and `calculate_environment`
            or in static method, e.g., `scenario_effectiveness`.
        """
        if not self.export_sce_txt:
            return
        ofile = self.scenario_dir + os.path.sep + 'Scenario_%d.txt' % self.ID
        with open(ofile, 'w', encoding='utf-8') as outfile:
            outfile.write('Scenario ID: %d\n' % self.ID)
            outfile.write('Gene number: %d\n' % self.gene_num)
            outfile.write('Gene values: %s\n' % ', '.join((str(v) for v in self.gene_values)))
            outfile.write('Scenario items:\n')
            if len(self.bmp_items) > 0:
                header = list()
                for obj, item in self.bmp_items.items():
                    header = list(item.keys())
                    break
                outfile.write('\t'.join(header))
                outfile.write('\n')
                for obj, item in self.bmp_items.items():
                    outfile.write('\t'.join(str(v) for v in list(item.values())))
                    outfile.write('\n')
            outfile.write('Effectiveness:\n\teconomy: %f\n\tenvironment: %f\n' % (self.economy,
                                                                                  self.environment))

    def export_scenario_to_gtiff(self):
        """Export the areal BMPs to gtiff for further analysis.

        This function should be overridden in inherited class.
        """
        pass

    def import_from_mongodb(self, sid):
        """Import a specified Scenario (`sid`) from MongoDB.

        This function should be overridden in inherited class.
        Returns:
            True if succeed, otherwise False.
        """
        pass

    def import_from_txt(self, sid):
        """Import a specified Scenario (`sid`) from text file.

        This function should be overridden in inherited class.
        Returns:
            True if succeed, otherwise False.
        """
        pass

    def calculate_economy(self):
        """Calculate economical effectiveness, which is application specified."""
        pass

    def calculate_environment(self):
        """Calculate environment effectiveness, which is application specified."""
        pass

    def execute_seims_model(self):
        """Run SEIMS for evaluating environmental effectiveness.
        If execution fails, the `self.economy` and `self.environment` will be set the worst values.
        """
        scoop_log('Scenario ID: %d, running SEIMS model...' % self.ID)
        self.model.scenario_id = self.ID
        self.modelout_dir = self.model.OutputDirectory
        self.modelrun = self.model.run()
        return self.modelrun

    def initialize(self):
        # type: () -> List[int]
        """Initialize a scenario.

        Returns:
            A list contains BMPs identifier of each gene location.
        """
        # Create configuration rate for each location randomly, 0.4 ~ 0.6
        cr = random.randint(40, 60) / 100.
        if self.rule_mtd == 'RDM':
            self.random_based_config(cr)
        else:
            self.rule_based_config(self.rule_mtd, cr)
        if len(self.gene_values) == self.gene_num > 0:
            return self.gene_values
        else:
            raise RuntimeError('Initialize Scenario failed, please check the inherited scenario'
                               ' class, especially the overwritten rule_based_config and'
                               ' random_based_config!')


if __name__ == '__main__':
    cf = get_config_parser()
    cfg = SAConfig(cf)
    sceobj = Scenario(cfg)

    # test the picklable of Scenario class.
    import pickle

    s = pickle.dumps(sceobj)
    # print(s)
    new_cfg = pickle.loads(s)  # type: Scenario
    print(new_cfg.modelcfg.ConfigDict)
    print(new_cfg.model.start_time, new_cfg.model.end_time)
    print(new_cfg.model.scenario_id, new_cfg.ID)
    new_cfg.set_unique_id()
    print(new_cfg.ID)
