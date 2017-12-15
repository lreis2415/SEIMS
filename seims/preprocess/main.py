#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Main function entrance for preprocessing
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-29  lj - reformat according to pylint and google style
"""
import time
import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.getcwd(), '..')))

# Load configuration file
from config import parse_ini_configuration
# MongoDB modules
from db_build_mongodb import ImportMongodbClass
# Spatial delineation
from sd_delineation import SpatialDelineation


def workflow():
    """Main entrance for the whole preprocessing workflow"""
    start_time = time.time()
    # Parse Configuration file
    seims_cfg = parse_ini_configuration()
    # Spatial delineation by TauDEM
    SpatialDelineation.workflow(seims_cfg)
    # Import to MongoDB database
    ImportMongodbClass.workflow(seims_cfg)

    end_time = time.time()
    print ("SEIMS preprocess done, time-consuming: %.2f seconds." % (end_time - start_time))


if __name__ == "__main__":
    workflow()
