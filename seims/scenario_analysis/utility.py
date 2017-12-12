#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Utility functions and classes for scenario analysis.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-11-08  hr - initial implementation.\n
                17-08-18  lj - reorganization.\n
"""

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))
import shutil
import uuid

import scoop
from preprocess.db_mongodb import ConnectMongoDB
from pygeoc.utils import MathClass


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


def delete_scenarios_by_ids(hostname, port, dbname, sids):
    """Delete scenario data by ID in MongoDB."""
    client = ConnectMongoDB(hostname, port)
    conn = client.get_conn()
    db = conn[dbname]
    collection = db['BMP_SCENARIOS']
    for _id in sids:
        collection.remove({'ID': _id})
        print ('Delete scenario: %d in MongoDB completed!' % _id)
    client.close()


def delete_model_outputs(model_workdir, hostname, port, dbname):
    """Delete model outputs and scenario in MongoDB."""
    f_list = os.listdir(model_workdir)
    # print f_list
    sids = list()
    for f in f_list:
        outfilename = model_workdir + os.sep + f
        if os.path.isdir(outfilename):
            if len(f) > 9:
                if MathClass.isnumerical(f[-9:]):
                    shutil.rmtree(outfilename)
                    sid = int(f[-9:])
                    sids.append(sid)
    if len(sids) > 0:
        delete_scenarios_by_ids(hostname, port, dbname, sids)
