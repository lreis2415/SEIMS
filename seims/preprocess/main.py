#! /usr/bin/env python
# coding=utf-8
# @Main function entrance for preprocessing
# @Author: Liang-Jun Zhu
#
import os
import sys

# MongoDB modules
from build_db import BuildMongoDB
# HydroClimate modules
from hydroclimate_sites import ImportHydroClimateSitesInfo
from import_measurement import ImportMeasurementData
from MeteorologicalDaily import ImportDailyMeteoData
from parameters_extraction import ExtractParameters
from PrecipitationDaily import ImportDailyPrecData
# Spatial modules
from subbasin_delineation import SubbasinDelineation
# Intermediate SQLite database
from txt2db3 import reConstructSQLiteDB
# Load configuration file
from util import GetINIfile, LoadConfiguration

if __name__ == "__main__":
    # Load Configuration file
    LoadConfiguration(GetINIfile())
    # Update SQLite Parameters.db3 database
    reConstructSQLiteDB()
    # Climate Data
    SitesMList, SitesPList = ImportHydroClimateSitesInfo()
    ImportDailyMeteoData(SitesMList)
    ImportDailyPrecData(SitesPList)
    # Spatial Data derived from DEM
    SubbasinDelineation()
    # Measurement Data, such as discharge, sediment yield.
    ImportMeasurementData()
    # Extract parameters from landuse, soil properties etc.
    ExtractParameters()
    # Import to MongoDB database
    BuildMongoDB()
