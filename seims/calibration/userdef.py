#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""User defined functions.
    @author   : Liangjun Zhu
    @changelog: 18-1-22  lj - initial implementation.\n
"""
from preprocess.db_mongodb import ConnectMongoDB


def write_param_values_to_mongodb(hostname, port, spatial_db, param_defs, param_values):
    # update Parameters collection in MongoDB
    client = ConnectMongoDB(hostname, port)
    conn = client.get_conn()
    db = conn[spatial_db]
    collection = db['PARAMETERS']
    for idx, pname in enumerate(param_defs['names']):
        v2str = ','.join(str(v) for v in param_values[:, idx])
        collection.find_one_and_update({'NAME': pname}, {'$set': {'CALI_VALUES': v2str}})
    client.close()
