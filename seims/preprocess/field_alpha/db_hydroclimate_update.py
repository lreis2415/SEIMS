"""Append or update some data items for field version SEIMS.

    @author   : Liangjun Zhu

    @changelog:
    - 18-06-09  - lj - first implementation version.
"""
from __future__ import absolute_import

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '../..')))

from copy import deepcopy

from preprocess.db_mongodb import ConnectMongoDB
from preprocess.text import DBTableNames, FieldNames



def main():
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    db_model = conn[seims_cfg.spatial_db]
    sitelist_coll = db_model[DBTableNames.main_sitelist]

    # Add an item in SITELIST collection, in which the SUBBASINID is 9999
    bsn_item = sitelist_coll.find_one({FieldNames.subbasin_id: 0})
    field_bsn_item = deepcopy(bsn_item)
    del field_bsn_item['_id']
    field_bsn_item[FieldNames.subbasin_id] = 9999
    sitelist_coll.insert_one(field_bsn_item)


if __name__ == "__main__":
    main()
