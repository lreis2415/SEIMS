#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Extract spatial parameters for soil, landuse, and terrain related.
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize according to pylint and google style
"""
from sp_landuse import LanduseUtilClass
from sp_soil import SoilUtilClass
from sp_terrain import TerrainUtilClass


def extract_spatial_parameters(cfg, maindb):
    """Main entrance for spatial parameters extraction."""
    # 1. Soil related
    SoilUtilClass.parameters_extraction(cfg)
    # 2. Landuse/Landcover related
    LanduseUtilClass.parameters_extraction(cfg, maindb)
    # 3. Terrain related and other spatial parameters
    TerrainUtilClass.parameters_extration(cfg, maindb)


def main():
    """TEST CODE"""
    from config import parse_ini_configuration
    from db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    main_db = conn[seims_cfg.spatial_db]

    extract_spatial_parameters(seims_cfg, main_db)


if __name__ == "__main__":
    main()
