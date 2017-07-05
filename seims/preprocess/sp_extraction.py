#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Extract spatial parameters for soil, landuse, and terrain related.
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize according to pylint and google style
"""
from seims.preprocess.config import parse_ini_configuration
from seims.preprocess.sp_landuse import LanduseUtilClass
from seims.preprocess.sp_soil import SoilUtilClass
from seims.preprocess.sp_terrain import TerrainUtilClass


def extract_spatial_parameters(cfg):
    """Main entrance for spatial parameters extraction."""
    # 1. Soil related
    SoilUtilClass.parameters_extraction(cfg)
    # 2. Landuse/Landcover related
    LanduseUtilClass.parameters_extraction(cfg)
    # 3. Terrain related and other spatial parameters
    TerrainUtilClass.parameters_extration(cfg)


def main():
    """TEST CODE"""
    seims_cfg = parse_ini_configuration()
    extract_spatial_parameters(seims_cfg)


if __name__ == "__main__":
    main()
