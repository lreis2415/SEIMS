#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""MongoDB utility
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-27  lj - reorganize as basic class other than Global variables
"""
import sys

from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

from seims.preprocess.text import DBTableNames, ModelParamFields, SubbsnStatsName


class ConnectMongoDB(object):
    """Connect to MongoDB, and close when finished."""

    def __init__(self, ip, port):
        """initial mongodb client by IP address and port."""
        try:
            self.conn = MongoClient(ip, port)
        except ConnectionFailure:
            sys.stderr.write('Could not connect to MongoDB: %s' % ConnectionFailure.message)
            sys.exit(1)

    def get_conn(self):
        """get MongoDB connection."""
        return self.conn

    def close(self):
        """Close collection."""
        self.conn.close()


class MongoQuery(object):
    """
    Query data from MongoDB
    """

    @staticmethod
    def get_subbasin_num(db_model):
        """Query subbasin number, raise exception if error occurs."""
        coll_param = db_model[DBTableNames.main_parameter]
        subbsn_num_dict = coll_param.find_one({ModelParamFields.name: SubbsnStatsName.subbsn_num})
        if subbsn_num_dict is None or subbsn_num_dict.get(ModelParamFields.value) is None:
            raise RuntimeError('Subbasin number item is not existed in MongoDB!')
        return subbsn_num_dict.get(ModelParamFields.value)
