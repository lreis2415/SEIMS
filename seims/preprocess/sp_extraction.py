#! /usr/bin/env python
# coding=utf-8
# @Extract spatial parameters for soil, landuse, and terrain related.
#

from pygeoc.utils.parseConfig import getconfigfile

from sp_soil import soil_parameters_extraction
from sp_landuse import landuse_parameters_extraction
from sp_terrain import terrain_parameters_extration
from utility import LoadConfiguration


def ExtractSpatialParameters():
    """Main entrance for spatial parameters extraction."""
    # 1. Soil related
    soil_parameters_extraction()
    # 2. Landuse/Landcover related
    landuse_parameters_extraction()
    # 3. Terrain related and other spatial parameters
    terrain_parameters_extration()


if __name__ == "__main__":
    LoadConfiguration(getconfigfile())
    ExtractSpatialParameters()
