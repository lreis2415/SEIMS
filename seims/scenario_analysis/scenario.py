#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base class of Scenario for coupling NSAG-II and SEIMS.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-10-29  hr - initial implementation.\n
                17-08-18  lj - redesign and rewrite.\n
"""
import os
import random
import sys
import time

from seims.scenario_analysis.utility import generate_uniqueid, print_message


class Scenario(object):
    """Base class of Scenario for SEIMS.

    Attributes:
        ID(integer): Unique ID in BMPScenario database -> BMP_SCENARIOS collection
        economy(float): Economical effectiveness, e.g., income minus expenses
        environment(float): Environmental effectiveness, e.g., reduction rate of soil erosion
        gen_num(integer): The number of genes of one chromosome, i.e., an individual
        gen_values(list): BMP identifiers on each location of gene. The length is gen_num.
        bmp_items(dict): BMP configuration items that can be imported to MongoDB directly.
                         The key is `bson.objectid.ObjectId`, the value is BMP parameters dict.
        rules(boolean): Config BMPs randomly or rule-based.
    """

    def __init__(self, cfg):
        """Initialize."""
        self.ID = 0.
        self.economy = 0.
        self.environment = 0.

        self.gen_num = 0
        self.gen_values = list()
        self.bmp_items = dict()

        self.rules = cfg.bmps_rule

    def set_unique_id(self):
        """Set unique ID."""
        self.ID = generate_uniqueid().next()

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
        """Decoding gen_values to bmp_items

        This function should be overridden.
        """
        pass

    def export_to_mongodb(self):
        """Export current scenario to MongoDB.
        Delete the same ScenarioID if existed.
        """
        client = MongoClient(hostname, port)
        db = client[dbname]
        collection = db.BMP_SCENARIOS
        keyarray = ["ID", "NAME", "BMPID", "SUBSCENARIO", "DISTRIBUTION", "COLLECTION", "LOCATION"]
        for line in self.sce_list:
            conf = {}
            li_list = line.split('\t')
            for i in range(len(li_list)):
                if isNumericValue(li_list[i]):
                    conf[keyarray[i]] = float(li_list[i])
                else:
                    conf[keyarray[i]] = str(li_list[i]).upper()
            collection.insert(conf)

    def export_to_txt(self):
        """Export current scenario information to text file."""
        outfile = open(txtfile, 'a')
        infoStr = str(self.id) + "\t" + str(self.economy) + "\t" + str(self.environment) \
                  + "\t" + str(self.gen_values) + LF
        outfile.write(infoStr)
        outfile.close()

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

    def evaluate_environment(self):
        """Run SEIMS for evaluating environmental effectiveness.
        If execution fails, the `self.economy` and `self.environment` will be set the worst values.
        """
        print_message('Scenario ID: %d' % self.ID)

        # startT = time.time()
        cmdStr = "%s %s %d %d %s %d %d" % (
            model_Exe, model_Workdir, threadsNum, layeringMethod, HOSTNAME, PORT, self.id)
        # print cmdStr
        process = Popen(cmdStr, shell=True, stdout=PIPE)
        while process.stdout.readline() != "":
            line = process.stdout.readline().split("\n")
            if line[0] != "" and len(line[0]) == 20:
                lineArr = line[0].split(' ')[0].split('-')
                if int(lineArr[2]) == 1:
                    sys.stdout.write(str(lineArr[0]) + "-" + str(lineArr[1]) + " ")
            continue
        process.wait()

        dataDir = model_Workdir + os.sep + "OUTPUT" + str(self.id)
        if process.returncode == 0:
            ## Outlet
            # polluteList = ['SED']
            # polluteWt = [1.]
            # for pp in range(len(polluteList)):
            #     simData = ReadSimfromTxt(timeStart, timeEnd, dataDir, polluteList[pp], subbasinID=0)
            #     self.benefit_env += sum(simData) / polluteWt[pp]

            ## soil loss in each cell
            while not os.path.isfile(dataDir + os.sep + "0_" + soilErosion):
                time.sleep(2)
            if os.path.isfile(dataDir + os.sep + "0_" + soilErosion):
                self.environment = getSoilErosion(dataDir, soilErosion, subbasinID=0)
            else:
                generationsInfoFile = model_Workdir + os.sep + "NSGAII_OUTPUT" + os.sep + "scenarios_info.txt"
                Sces_env = getScesInfo(generationsInfoFile)
                self.environment = numpy.max(Sces_env)
        else:
            # process.kill()
            ## If process is killed, benefit_env is replaced by average of exist benefit_env value
            generationsInfoFile = model_Workdir + os.sep + "NSGAII_OUTPUT" + os.sep + "scenarios_info.txt"
            Sces_env = getScesInfo(generationsInfoFile)
            self.environment = numpy.max(Sces_env)

        # Save scenario raster
        # createForld(dataDir)
        # writeSceRaste(dataDir, MODEL_DIR + os.sep + fieldRaster, self.gen_values, wRaster=False)

        (os.linesep + "economy: " + str(self.economy))
        ("benefit_env: " + str(self.environment))
        # endT = time.time()
        # print_message("SEIMS running time: %.2fs" % (endT - startT))

    def initialize(self):
        """Initialize a scenario.

        Returns:
            A list contains BMPs identifier of each gene location.
        """
        # Create configuration rate for each location randomly, 0.25 ~ 0.75
        cr = random.randint(25, 75) / 100.

        if self.rules:
            self.rule_based_config(cr)
        else:
            self.random_based_config(cr)
        if len(self.gen_values) == self.gen_num > 0:
            return self.gen_values
        else:
            raise RuntimeError('Initialize Scenario failed, please check the inherited scenario'
                               ' class, especially the overwritten rule_based_config and '
                               'random_based_config!')

    @staticmethod
    def initialize_scenario(cf):
        """Used for initial individual of population.

        Designed as static method, which should be instantiated in inherited class.
        """
        pass

    @staticmethod
    def scenario_effectiveness(cf, individual):
        """Used for evaluate the effectiveness of given individual.

        Designed as static method, which should be instantiated in inherited class.
        """
        pass
