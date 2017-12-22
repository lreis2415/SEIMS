#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base class of Scenario for coupling NSAG-II.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-10-29  hr - initial implementation.\n
                17-08-18  lj - redesign and rewrite.\n
"""
import os
import random
from datetime import timedelta

from bson.objectid import ObjectId
from pygeoc.utils import StringClass, get_config_parser
from pymongo.errors import NetworkTimeout

from config import SAConfig
from preprocess.db_mongodb import ConnectMongoDB
from run_seims import MainSEIMS
from utility import generate_uniqueid, print_message


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
        """Initialize."""
        self.ID = -1
        self.timerange = 1.  # unit: year
        self.economy = 0.
        self.environment = 0.
        self.worst_econ = cfg.worst_econ
        self.worst_env = cfg.worst_env

        self.gene_num = 0
        self.gene_values = list()
        self.bmp_items = dict()

        self.rules = cfg.bmps_rule
        self.rule_mtd = cfg.rule_method
        self.bmps_info = cfg.bmps_info
        self.bmps_retain = cfg.bmps_retain
        self.export_sce_txt = cfg.export_sce_txt
        self.export_sce_tif = cfg.export_sce_tif
        # run seims related
        self.model_dir = cfg.model_dir
        self.modelout_dir = None
        self.bin_dir = cfg.seims_bin
        self.nthread = cfg.seims_nthread
        self.lyrmethod = cfg.seims_lyrmethod
        self.hostname = cfg.hostname
        self.port = cfg.port
        self.scenario_db = cfg.bmp_scenario_db
        self.main_db = cfg.spatial_db
        self.modelrun = False
        # predefined directories
        self.scenario_dir = cfg.scenario_dir

    def set_unique_id(self):
        """Set unique ID."""
        self.ID = generate_uniqueid().next()
        self.modelout_dir = '%s/OUTPUT%d' % (self.model_dir, self.ID)
        self.read_simulation_timerange()
        return self.ID

    def read_simulation_timerange(self):
        """Read simulation time range from MongoDB."""
        client = ConnectMongoDB(self.hostname, self.port)
        conn = client.get_conn()
        db = conn[self.main_db]
        collection = db['FILE_IN']
        try:
            stime_str = collection.find_one({'TAG': 'STARTTIME'}, no_cursor_timeout=True)['VALUE']
            etime_str = collection.find_one({'TAG': 'ENDTIME'}, no_cursor_timeout=True)['VALUE']
            stime = StringClass.get_datetime(stime_str)
            etime = StringClass.get_datetime(etime_str)
            dlt = etime - stime + timedelta(seconds=1)
            self.timerange = (dlt.days * 86400. + dlt.seconds) / 86400. / 365.
        except NetworkTimeout or Exception:
            # In case of unexpected raise
            self.timerange = 1.  # set default
            pass
        client.close()

    def rule_based_config(self, conf_rate):
        """Config available BMPs to each gene of the chromosome by rule-based method.

        Virtual function that should be overridden in inherited Scenario class.
        """
        pass

    def random_based_config(self, conf_rate):
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
        client = ConnectMongoDB(self.hostname, self.port)
        conn = client.get_conn()
        db = conn[self.scenario_db]
        collection = db['BMP_SCENARIOS']
        try:
            # find ScenarioID, remove if existed.
            if collection.find({'ID': self.ID}, no_cursor_timeout=True).count():
                collection.remove({'ID': self.ID})
        except NetworkTimeout or Exception:
            # In case of unexpected raise
            pass
        for objid, bmp_item in self.bmp_items.iteritems():
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
        ofile = self.scenario_dir + os.sep + 'Scenario_%d.txt' % self.ID
        outfile = open(ofile, 'w')
        outfile.write('Scenario ID: %d\n' % self.ID)
        outfile.write('Gene number: %d\n' % self.gene_num)
        outfile.write('Gene values: %s\n' % ', '.join((str(v) for v in self.gene_values)))
        outfile.write('Scenario items:\n')
        if len(self.bmp_items) > 0:
            for obj, item in self.bmp_items.iteritems():
                header = item.keys()
                break
            outfile.write('\t'.join(header))
            outfile.write('\n')
            for obj, item in self.bmp_items.iteritems():
                outfile.write('\t'.join(str(v) for v in item.values()))
                outfile.write('\n')

        outfile.write('Effectiveness:\n\teconomy: %f\n\tenvironment: %f\n' % (self.economy,
                                                                              self.environment))
        outfile.close()

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
        print_message('Scenario ID: %d, running SEIMS model...' % self.ID)
        seims_obj = MainSEIMS(self.bin_dir, self.model_dir, self.nthread,
                              self.lyrmethod, self.hostname, self.port, self.ID)
        self.modelrun = seims_obj.run()
        return self.modelrun

    def initialize(self):
        """Initialize a scenario.

        Returns:
            A list contains BMPs identifier of each gene location.
        """
        # Create configuration rate for each location randomly, 0.25 ~ 0.75
        cr = random.randint(40, 60) / 100.
        # cr = 0.75
        if self.rules:
            self.rule_based_config(cr)
        else:
            self.random_based_config(cr)
        if len(self.gene_values) == self.gene_num > 0:
            return self.gene_values
        else:
            raise RuntimeError('Initialize Scenario failed, please check the inherited scenario'
                               ' class, especially the overwritten rule_based_config and'
                               ' random_based_config!')


def initialize_scenario(cf):
    """Used for initial individual of population.

    Designed as static method, which should be overridden in inherited class.
    """
    pass


def scenario_effectiveness(cf, individual):
    """Used for evaluate the effectiveness of given individual.

    Designed as static method, which should be overridden in inherited class.
    """
    pass


if __name__ == '__main__':
    cf = get_config_parser()
    cfg = SAConfig(cf)
    sceobj = Scenario(cfg)

    # test the picklable of Scenario class.
    import pickle

    s = pickle.dumps(sceobj)
    # print (s)
    new_cfg = pickle.loads(s)
    print (new_cfg.bin_dir)
