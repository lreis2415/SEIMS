"""Import BMP Scenario related parameters to MongoDB

    @author   : Liangjun Zhu

    @changelog:
    - 16-06-16  lj - first implementation version.
    - 17-06-22  lj - improve according to pylint and google style.
    - 18-02-08  lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import logging
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.raster import RasterUtilClass
from pygeoc.utils import MathClass, FileClass, StringClass

from preprocess.text import DBTableNames
from preprocess.config import PreprocessConfig
from utility import read_data_items_from_txt


class ImportScenario2Mongo(object):
    """Import scenario data to MongoDB
    """
    _FLD_DB = 'DB'
    _LocalX = 'LocalX'
    _LocalY = 'LocalY'
    _DISTDOWN = 'DIST2REACH'
    _SUBBASINID = 'SUBBASINID'

    @staticmethod
    def scenario_from_texts(cfg):  # type: (PreprocessConfig) -> bool
        """Import BMPs Scenario data to MongoDB
        Args:
            cfg: SEIMS configuration object
        Returns:
            False if failed, otherwise True.
        """
        if not cfg.use_scenario:
            return False
        logging.info('Import BMP Scenario Data... ')
        bmp_files = FileClass.get_filename_by_suffixes(cfg.scenario_dir, ['.txt', '.csv'])
        bmp_tabs = list()
        bmp_tabs_path = list()
        for f in bmp_files:
            bmp_tabs.append(f.split('.')[0])
            bmp_tabs_path.append(cfg.scenario_dir + os.path.sep + f)

        # initialize if collection not existed
        if cfg.scenariodb is None:
            raise(AttributeError, "Scenario database MUST be initialized first!")
        c_list = cfg.scenariodb.list_collection_names()
        for item in bmp_tabs:
            if not StringClass.string_in_list(item.upper(), c_list):
                cfg.scenariodb.create_collection(item.upper())
            else:
                cfg.scenariodb.drop_collection(item.upper())
        # Read subbasin.tif and dist2Stream.tif
        subbasin_r = RasterUtilClass.read_raster(cfg.spatials.subbsn)
        dist2stream_r = RasterUtilClass.read_raster(cfg.spatials.dist2stream_dinf)
        # End reading
        for j, bmp_txt in enumerate(bmp_tabs_path):
            bmp_tab_name = bmp_tabs[j]
            data_array = read_data_items_from_txt(bmp_txt)
            field_array = data_array[0]
            data_array = data_array[1:]
            for item in data_array:
                dic = dict()
                for i, field_name in enumerate(field_array):
                    if MathClass.isnumerical(item[i]):
                        v = float(item[i])
                        if v % 1. == 0.:
                            v = int(v)
                        dic[field_name.upper()] = v
                    else:
                        dic[field_name.upper()] = str(item[i]).upper()
                if StringClass.string_in_list(ImportScenario2Mongo._LocalX, list(dic.keys())) and \
                        StringClass.string_in_list(ImportScenario2Mongo._LocalY, list(dic.keys())):
                    subbsn_id = subbasin_r.get_value_by_xy(
                            dic[ImportScenario2Mongo._LocalX.upper()],
                            dic[ImportScenario2Mongo._LocalY.upper()])
                    distance = dist2stream_r.get_value_by_xy(
                            dic[ImportScenario2Mongo._LocalX.upper()],
                            dic[ImportScenario2Mongo._LocalY.upper()])
                    if subbsn_id is not None and distance is not None:
                        dic[ImportScenario2Mongo._SUBBASINID] = int(subbsn_id)
                        dic[ImportScenario2Mongo._DISTDOWN] = float(distance)
                        cfg.scenariodb[bmp_tab_name.upper()].find_one_and_replace(dic, dic,
                                                                                  upsert=True)
                else:
                    cfg.scenariodb[bmp_tab_name.upper()].find_one_and_replace(dic, dic,
                                                                              upsert=True)
        # print('BMP tables are imported.')
        # Write BMP database name into Model workflow database
        c_list = cfg.maindb.list_collection_names()
        if not StringClass.string_in_list(DBTableNames.main_scenario, c_list):
            cfg.maindb.create_collection(DBTableNames.main_scenario)

        bmp_info_dic = dict()
        bmp_info_dic[ImportScenario2Mongo._FLD_DB] = cfg.scenario_db
        cfg.maindb[DBTableNames.main_scenario].find_one_and_replace(bmp_info_dic, bmp_info_dic,
                                                                    upsert=True)
        return True


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    ImportScenario2Mongo.scenario_from_texts(seims_cfg)


if __name__ == "__main__":
    main()
