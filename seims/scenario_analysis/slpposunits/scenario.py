#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Scenario for optimizing BMPs based on slope position units.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-10-29  hr - initial implementation.\n
                17-08-18  lj - redesign and rewrite.\n
"""
from seims.scenario_analysis.scenario import Scenario


class SPScenario(Scenario):
    """Scenario analysis based on slope position units."""

    def __init__(self, cf):
        """Initialization."""
        Scenario.__init__(self, cf)

    def rule_based_config(self, conf_rate):
        pass

    def random_based_config(self, conf_rate):
        pass

    def decoding(self):

        # scenario section
        if len(self.gen_values) == 0:
            raise Exception("'gen_values' cannot be Null!")
        if self.ID is None:
            raise Exception("'id' cannot be None!")

        # field_index = self.field_Num
        locations = getBMPsLocations(self.gen_values, bmps_areal_struct)
        sinfo = str(self.id) + "\tS_" + str(self.id) + "\t17\t"
        for bmp in bmps_areal_struct:
            scenario_Row = ""
            scenario_Row += sinfo
            scenario_Row += str(bmp) + "\tRASTER|MGT_FIELDS\tAREAL_STRUCT_MANAGEMENT\t"
            locationStr = ""
            for loc in locations[bmp - 1]:
                locationStr += str(loc) + ","
            locationStr = locationStr[:-1]
            scenario_Row += locationStr
            self.sce_list.append(scenario_Row)

    def import_from_mongodb(self, sid):
        pass

    def import_from_txt(self, sid):
        pass

    def calculate_economy(self):
        if len(self.gen_values) == 0:
            raise Exception("'gen_values' cannot be Null!")

        for f in range(field_Num):
            bmpPosArr = numpy.where(numpy.array(bmps_areal_struct) == self.gen_values[f])
            if len(bmpPosArr[0]) > 0:
                self.economy += bmps_areal_struct_cost[bmpPosArr[0][0]] * field_Area[f]

    @staticmethod
    def initialize_scenario(cf):
        sce = SPScenario(cf)
        return sce.initialize()

    @staticmethod
    def scenario_effectiveness(cf, individual):
        # 1. instantiate the inherited Scenario class.
        sce = SPScenario(cf)
        sce.set_unique_id()
        setattr(sce, 'gen_values', individual)
        # 2. decoding gene values to BMP items and exporting to MongoDB.
        sce.decoding()
        sce.export_to_mongodb()
        # 3. evaluate_environment and calculate_economy
        sce.calculate_economy()
        sce.evaluate_environment()
        # 4. Save scenarios information in text file
        sce.export_to_txt()

        return sce.economy, sce.environment
