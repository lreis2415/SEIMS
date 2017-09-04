#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Export scenario to tiff.
    @author   : Liangjun Zhu
    @changelog:
"""
from seims.pygeoc.pygeoc.utils.utils import get_config_parser
from seims.scenario_analysis.slpposunits.config import SASPUConfig
from seims.scenario_analysis.slpposunits.scenario import SPScenario


def export_scenario_demo():
    """Export scenario as raster data."""
    cf = get_config_parser()
    cfg = SASPUConfig(cf)
    gene_array = [0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0,
                  1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
                  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                  0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 2.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0,
                  0.0, 2.0, 0.0, 2.0, 0.0, 2.0, 0.0, 2.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                  0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0,
                  0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    out_raster = r'C:\z_data\ChangTing\seims_models\NSGA2_Output\0830_method3\rule_mth3_3rd\gen100_158736296.tif'
    sce = SPScenario(cfg)
    setattr(sce, 'gene_values', gene_array)
    sce.export_scenario_to_gtiff(out_raster)


if __name__ == '__main__':
    export_scenario_demo()
