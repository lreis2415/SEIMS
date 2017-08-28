#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Utility functions and classes for scenario analysis.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-11-08  hr - initial implementation.\n
                17-08-18  lj - reorganization.\n
"""

import os
import shutil
import uuid

import scoop
from seims.preprocess.db_mongodb import ConnectMongoDB
from seims.pygeoc.pygeoc.utils.utils import MathClass


def generate_uniqueid():
    """Generate unique integer ID for Scenario using uuid.

    Usage:
        uniqueid = generate_uniqueid().next()
    """
    uid = int(str(uuid.uuid4().fields[-1])[:9])
    while True:
        yield uid
        uid += 1


def print_message(msg):
    if os.name != 'nt':
        scoop.logger.warn(msg)
    else:
        print (msg)


def delete_scenarios_by_id(hostname, port, dbname, sid):
    """Delete scenario data by ID in MongoDB."""
    client = ConnectMongoDB(hostname, port).get_conn()
    db = client[dbname]
    collection = db['BMP_SCENARIOS']
    collection.remove({'ID': sid})
    print ('Delete scenario: %d in MongoDB completed!' % sid)
    client.close()


def delete_model_outputs(model_workdir, hostname, port, dbname):
    """Delete model outputs and scenario in MongoDB."""
    f_list = os.listdir(model_workdir)
    # print f_list
    for f in f_list:
        outfilename = model_workdir + os.sep + f
        if os.path.isdir(outfilename):
            if len(f) > 9:
                if MathClass.isnumerical(f[-9:]):
                    shutil.rmtree(outfilename)
                    sid = int(f[-9:])
                    delete_scenarios_by_id(hostname, port, dbname, sid)
