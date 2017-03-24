#! /usr/bin/env python
# coding=utf-8
# @Main function entrance for preprocessing
# @Author: Liang-Jun Zhu
#

# pygeoc
from pygeoc.utils.parseConfig import getconfigfile
# Load configuration file
from utility import LoadConfiguration

# Spatial delineation
from sd_delineation import SubbasinDelineation
# Spatial parameters extraction
from sp_extraction import ExtractSpatialParameters
# MongoDB modules
from db_build_mongodb import BuildMongoDB

if __name__ == "__main__":
    # Load Configuration file
    LoadConfiguration(getconfigfile())
    # Spatial delineation by TauDEM
    SubbasinDelineation()
    # Extract spatial parameters for reaches, landuse, soil, etc.
    ExtractSpatialParameters()
    # Import to MongoDB database
    BuildMongoDB()
