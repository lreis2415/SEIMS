#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""MongoDB utility
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-27  lj - reorganize as basic class other than Global variables
"""
import sys

from pymongo import MongoClient
from pymongo.errors import ConnectionFailure, InvalidOperation

from text import DBTableNames, ModelParamFields


class ConnectMongoDB(object):
    """Connect to MongoDB, and close when finished."""

    def __init__(self, ip, port, maxPoolSize=None):
        """initial mongodb client by IP address and port."""
        try:
            self.conn = MongoClient(ip, port, maxPoolSize=maxPoolSize)
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
    def get_init_parameter_value(db_model, param_name, field=ModelParamFields.value):
        """Query initial parameter value, raise exception if error occurs."""
        coll_param = db_model[DBTableNames.main_parameter]
        param_dict = coll_param.find_one({ModelParamFields.name: param_name})
        if param_dict is None or param_dict.get(field) is None:
            raise RuntimeError('%s item is not existed in MongoDB!' % param_name)
        return param_dict.get(field)


class MongoUtil(object):
    """Some utility functions."""

    @staticmethod
    def run_bulk(bulk, errmsg=''):
        """Execute bulk operations, do not raise exception."""
        try:
            bulk.execute()
        except InvalidOperation:
            print ('WARNING: %s' % errmsg)
