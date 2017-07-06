#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Main function entrance for preprocessing
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-29  lj - reformat according to pylint and google style
"""
# Load configuration file
from seims.preprocess.config import parse_ini_configuration
# MongoDB modules
from seims.preprocess.db_build_mongodb import ImportMongodbClass
# Spatial delineation
from seims.preprocess.sd_delineation import SpatialDelineation
# Spatial parameters extraction
from seims.preprocess.sp_extraction import extract_spatial_parameters


def main():
    """Main entrance for the whole preprocessing workflow"""
    # Parse Configuration file
    seims_cfg = parse_ini_configuration()
    # Spatial delineation by TauDEM
    SpatialDelineation.workflow(seims_cfg)
    # Extract spatial parameters for reaches, landuse, soil, etc.
    extract_spatial_parameters(seims_cfg)
    # Import to MongoDB database
    ImportMongodbClass.workflow(seims_cfg)


if __name__ == "__main__":
    main()
