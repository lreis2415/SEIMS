# coding:utf-8
import os, sys
import argparse
import json
from configparser import ConfigParser
from pygeoc.utils import get_config_parser

from scenario_analysis.spatialunits.config import SASlpPosConfig
from scenario_analysis.spatialunits.scenario import SUScenario


def statsSelectedScenario(ini_file):
    cf = ConfigParser()
    cf.read(ini_file)
    cfg = SASlpPosConfig(cf)
    cfg.construct_indexes_units_gene()
    sce = SUScenario(cfg)
    sceid = 10511
    sce.set_unique_id(sceid)
    gene_values = [0.0, 2001.0, 2001.0, 2001.0, 2001.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2001.0, 0.0,
                   0.0, 2001.0, 2001.0, 0.0, 2001.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2001.0, 2001.0, 0.0, 0.0,
                   2001.0, 0.0, 1001.0, 1001.0, 0.0, 2001.0, 2001.0, 0.0, 2001.0, 2001.0, 0.0, 0.0, 2001.0,
                   0.0, 2001.0, 2001.0, 0.0, 2001.0, 0.0, 0.0, 1001.0, 3001.0, 2001.0, 0.0, 2001.0, 0.0,
                   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1001.0, 2001.0, 2001.0, 0.0, 2001.0, 2001.0, 1001.0, 0.0,
                   0.0, 2001.0, 2001.0, 0.0, 1001.0, 2001.0, 0.0, 0.0, 0.0, 0.0, 2001.0, 0.0, 0.0, 0.0,
                   0.0, 0.0, 2001.0, 2001.0, 2001.0, 0.0, 0.0, 0.0, 1001.0, 3001.0, 4001.0, 1001.0, 3001.0,
                   0.0, 1001.0, 3001.0, 0.0, 2001.0, 2001.0, 0.0, 0.0, 0.0, 0.0]
    # gene_values = [0.0, 2002.0, 2001.0, 2005.0, 2001.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2005.0,
    #                0.0, 0.0, 2005.0, 2002.0, 0.0, 2005.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2005.0, 2002.0, 0.0, 0.0,
    #                2001.0, 0.0, 1002.0, 1002.0, 0.0, 2002.0, 2001.0, 0.0, 2004.0, 2001.0, 0.0, 0.0, 2002.0, 0.0, 2004.0,
    #                2002.0, 0.0, 2005.0, 0.0, 0.0, 1001.0, 3002.0, 2005.0, 0.0, 2004.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    #                0.0, 1001.0, 2001.0, 2002.0, 0.0, 2005.0, 2004.0, 1004.0, 0.0, 0.0, 2002.0, 2001.0,
    #                0.0, 1001.0, 2004.0, 0.0, 0.0, 0.0, 0.0, 2003.0, 0.0, 0.0, 0.0, 0.0,
    #                0.0, 2005.0, 2001.0, 2005.0, 0.0, 0.0, 0.0, 1004.0, 3002.0, 4002.0, 1002.0, 3001.0,
    #                0.0, 1001.0, 3001.0, 0.0, 2003.0, 2003.0, 0.0, 0.0, 0.0, 0.0]
    sce.initialize(input_genes=gene_values)
    sce.decoding_with_bmps_order()
    results = sce.statistics_by_bmp()
    sum_capex = 0.
    sum_income = 0.
    sum_opex = 0.
    sum_area = 0.
    for k, v in results['BMPS'].iteritems():
        sum_capex += v['CAPEX']
        sum_income += v['INCOME']
        sum_opex += v['OPEX']
        sum_area += v['AREA']
    results['SUMMARY'] = {
        'CAPEX': sum_capex,
        'INCOME': sum_income,
        'OPEX': sum_opex,
        'AREA': sum_area
    }
    results['sceid'] = sceid
    results['economy'] = 162.98
    results['environment'] = 5.48
    results['gene_values'] = gene_values
    # print(results)
    fp = open('Scenario_Selected.json', 'w')
    json.dump(results, fp)
    return results


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Exposed APIs in Scenario Analysis')
    parser.add_argument('--func', help='which function to choose')
    parser.add_argument('-ini', help="Full path of configuration file")
    args = parser.parse_args()
    if args.func == 'statsSelectedScenario':
        statsSelectedScenario(args.ini)
    else:
        pass
