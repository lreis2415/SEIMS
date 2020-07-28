"""Main function entrance for preprocessing

    @author   : Liangjun Zhu

    @changelog:
    - 16-12-07  lj - rewrite for version 2.0
    - 17-06-29  lj - reformat according to pylint and google style
    - 18-02-08  lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import time
import sys
import os
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

# Load configuration file
from preprocess.config import parse_ini_configuration
# MongoDB modules
from preprocess.db_build_mongodb import ImportMongodbClass
# Spatial delineation
from preprocess.sd_delineation import SpatialDelineation


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
    print('SEIMS preprocess done, time-consuming: %.2f seconds.' % (end_time - start_time))


if __name__ == "__main__":
    workflow()
