#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Configuration of Scenario Analysis for SEIMS.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-12-30  hr - initial implementation.\n
                17-08-18  lj - reorganize as basic class.\n
"""
try:
    from ConfigParser import ConfigParser  # py2
except ImportError:
    from configparser import ConfigParser  # py3
from seims.pygeoc.pygeoc.utils.utils import FileClass, StringClass, get_config_file

class SAConfig(object):
    """Parse scenario analysis configuration of SEIMS project."""
    def __init__(self, cf):
        """Initialization."""






    # 1. Text files directories
    MODEL_DIR = None
    # print cf.sections()
    if 'PATH' in cf.sections():
        MODEL_DIR = cf.get('PATH', 'MODEL_DIR'.lower())
        fieldFile = cf.get('PATH', 'fieldFile'.lower())
        pointFile = cf.get('PATH', 'pointFile'.lower())
        pointBMPsFile = cf.get('PATH', 'pointBMPsFile'.lower())
        scenariosInfo = cf.get('PATH', 'scenariosInfo'.lower())
        fieldRaster = cf.get('PATH', 'fieldRaster'.lower())
        soilErosion = cf.get('PATH', 'soilErosion'.lower())
    else:
        raise ValueError("[PATH] section MUST be existed in *.ini file.")
    if not isPathExists(MODEL_DIR):
        raise IOError("Please Check Directories defined in [PATH]")

    # 2. NSGA-II
    if 'NSGAII' in cf.sections():
        GenerationsNum = int(cf.get('NSGAII', 'GenerationsNum'))
        PopulationSize = int(cf.getint('NSGAII', 'PopulationSize'))
        CrossoverRate = float(cf.get('NSGAII', 'CrossoverRate'))
        MutateRate = float(cf.get('NSGAII', 'MutateRate'))
        SelectRate = float(cf.get('NSGAII', 'SelectRate'))
    else:
        raise ValueError("[NSGAII] section MUST be existed in *.ini file.")

    # 3. MongoDB
    if 'MONGODB' in cf.sections():
        HOSTNAME = cf.get('MONGODB', 'HOSTNAME')
        PORT = int(cf.getint('MONGODB', 'PORT'))
        BMPScenarioDBName = cf.get('MONGODB', 'BMPScenarioDBName'.lower())
    else:
        raise ValueError("[MONGODB] section MUST be existed in *.ini file.")
    if not isIPValid(HOSTNAME):
        raise ValueError("HOSTNAME illegal defined in [MONGODB]!")

    # 3. SEIMS_Model
    if 'SEIMS_Model' in cf.sections():
        model_Exe = cf.get('SEIMS_Model', 'model_Exe'.lower())
        model_Workdir = cf.get('SEIMS_Model', 'model_Workdir'.lower())
        threadsNum = int(cf.get('SEIMS_Model', 'threadsNum'))
        layeringMethod = int(cf.get('SEIMS_Model', 'layeringMethod'))
        timeStart = cf.get('SEIMS_Model', 'timeStart')
        timeEnd = cf.get('SEIMS_Model', 'timeEnd')
    else:
        raise ValueError("[SEIMS_Model] section MUST be existed in *.ini file.")

    # 4. BMPs
    # bmps_farm = []
    # bmps_cattle = []
    # bmps_pig = []
    # bmps_sewage = []
    # bmps_farm_cost = []
    # bmps_cattle_cost = []
    # bmps_pig_cost = []
    # bmps_sewage_cost = []
    bmps_areal_struct = []
    bmps_areal_struct_cost = []
    if 'BMPs' in cf.sections():
        # bmps_farm_STR = cf.get('BMPs', 'bmps_farm'.lower())
        # bmps_cattle_STR = cf.get('BMPs', 'bmps_cattle'.lower())
        # bmps_pig_STR = cf.get('BMPs', 'bmps_pig'.lower())
        # bmps_sewage_STR_1 = cf.get('BMPs', 'bmps_sewage_1'.lower())
        # bmps_sewage_STR_2 = cf.get('BMPs', 'bmps_sewage_2'.lower())
        # bmps_sewage_STR_3 = cf.get('BMPs', 'bmps_sewage_3'.lower())
        # bmps_sewage_STR_4 = cf.get('BMPs', 'bmps_sewage_4'.lower())
        # bmps_farm_cost_STR = cf.get('BMPs', 'bmps_farm_cost'.lower())
        # # bmps_cattle_cost_STR = cf.get('BMPs', 'bmps_cattle_cost'.lower())
        # # bmps_pig_cost_STR = cf.get('BMPs', 'bmps_pig_cost'.lower())
        # # bmps_sewage_cost_STR = cf.get('BMPs', 'bmps_sewage_cost'.lower())
        # bmps_farm = StrtoIntArr(SplitStr(StripStr(bmps_farm_STR)))
        # bmps_cattle = StrtoIntArr(SplitStr(StripStr(bmps_cattle_STR)))
        # bmps_pig = StrtoIntArr(SplitStr(StripStr(bmps_pig_STR)))
        # bmps_sewage_1 = StrtoIntArr(SplitStr(StripStr(bmps_sewage_STR_1)))
        # bmps_sewage_2 = StrtoIntArr(SplitStr(StripStr(bmps_sewage_STR_2)))
        # bmps_sewage_3 = StrtoIntArr(SplitStr(StripStr(bmps_sewage_STR_3)))
        # bmps_sewage_4 = StrtoIntArr(SplitStr(StripStr(bmps_sewage_STR_4)))
        # bmps_sewage.append(bmps_sewage_1)
        # bmps_sewage.append(bmps_sewage_2)
        # bmps_sewage.append(bmps_sewage_3)
        # bmps_sewage.append(bmps_sewage_4)
        # farm_area = float(cf.get('BMPs', 'farm_area'.lower()))
        # bmps_farm_cost = StrtoFltArr(SplitStr(StripStr(bmps_farm_cost_STR)))
        # bmps_cattle_cost = StrtoFltArr(SplitStr(StripStr(bmps_cattle_cost_STR)))
        # bmps_pig_cost = StrtoFltArr(SplitStr(StripStr(bmps_pig_cost_STR)))
        # bmps_sewage_cost = StrtoFltArr(SplitStr(StripStr(bmps_sewage_cost_STR)))
        bmps_areal_struct_STR = cf.get('BMPs', 'bmp_areal_struct'.lower())
        bmps_areal_struct_cost_STR = cf.get('BMPs', 'bmp_areal_struct_cost'.lower())
        bmps_areal_struct = StrtoIntArr(SplitStr(StripStr(bmps_areal_struct_STR)))
        bmps_areal_struct_cost = StrtoFltArr(SplitStr(StripStr(bmps_areal_struct_cost_STR)))


    else:
        raise ValueError("[BMPs] section MUST be existed in *.ini file.")
    # 5. Switch
    if 'SWITCH' in cf.sections():
        BMPs_rule = int(cf.get('SWITCH', 'BMPs_rule'.lower()))
    else:
        raise ValueError("[SEIMS_Model] section MUST be existed in *.ini file.")

    # Scenario
    # fieldInfo = getFieldInfo(fieldFile)
    # pointSource = getPointSource(pointFile)
    # bmpsInfo = getBMPsInfo(pointBMPsFile, pointFile)
    #
    # field_farm = fieldInfo[1]
    # field_lu = fieldInfo[2]
    # farm_area = fieldInfo[3]
    # point_cattle = pointSource[0]
    # point_pig = pointSource[1]
    # point_sewage = pointSource[2]
    #
    # farm_Num = len(field_farm)
    # # farm_Num = 1
    # point_cattle_Num = len(point_cattle)
    # point_pig_Num = len(point_pig)
    # point_sewage_Num = len(point_sewage)
    #
    # # BMP cost
    # bmps_cattle_cost = bmpsInfo[4]
    # bmps_pig_cost = bmpsInfo[5]
    # bmps_sewage_cost = bmpsInfo[6]
    #
    # # livestock farm size
    # point_cattle_size = pointSource[3]
    # point_pig_size = pointSource[4]

    field_Num = getFieldNum(MODEL_DIR + os.sep + fieldRaster)
    field_Area = getFieldArea(MODEL_DIR + os.sep + fieldRaster)


def parse_ini_configuration():
    """Load model configuration from *.ini file"""
    cf = ConfigParser()
    ini_file = get_config_file()
    cf.read(ini_file)
    return SAConfig(cf)


if __name__ == '__main__':
    cfg = parse_ini_configuration()
