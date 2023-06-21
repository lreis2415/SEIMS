"""Extract spatial parameters for soil, landuse, and terrain related.

    @author   : Liangjun Zhu

    @changelog:
    - 16-12-07  lj - rewrite for version 2.0
    - 17-06-23  lj - reorganize according to pylint and google style
    - 18-02-08  lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys

from preprocess.sp_soil_conceptual import SoilPropertyConceptual

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from preprocess.sp_landuse import LanduseUtilClass
from preprocess.sp_soil_base import SoilUtilClass
from preprocess.sp_soil_physical import SoilPropertyPhysical
from preprocess.sp_soil_conceptual import SoilPropertyConceptual
from preprocess.sp_terrain import TerrainUtilClass


def extract_spatial_parameters(cfg):
    """Main entrance for spatial parameters extraction."""
    # 1. Soil related
    if cfg.soil_property_conceptual:
        SoilUtilClass.parameters_extraction(cfg, SoilPropertyConceptual)
    if cfg.soil_property_physical:
        SoilUtilClass.parameters_extraction(cfg, SoilPropertyPhysical)

    # 2. Landuse/Landcover related
    LanduseUtilClass.parameters_extraction(cfg)
    # 3. Terrain related and other spatial parameters
    TerrainUtilClass.parameters_extraction(cfg)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()

    extract_spatial_parameters(seims_cfg)


if __name__ == "__main__":
    main()
