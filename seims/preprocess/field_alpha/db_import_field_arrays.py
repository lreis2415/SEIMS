#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Import spatial parameters corresponding to fields as GridFS to MongoDB
    @author   : Liangjun Zhu
    @changelog: 18-06-08  lj - first implementation version.\n
"""
from __future__ import absolute_import

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '../..')))

from preprocess.db_mongodb import ConnectMongoDB


def main():
    from preprocess.config import parse_ini_configuration

    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    print('Success')


if __name__ == "__main__":
    main()
