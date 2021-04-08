"""Create a connection to MongoDB as a global module used in SCOOP-based parallel applications

    Note that, this is a temporary and not elegant solution.
      Before running such applications such as scenarios_analysis/spatialunits/main_nsga2.py,
      users MUST update the host and port manually.

    References:
        Explicit access to module level variables by accessing them explicitly on the module.
          https://stackoverflow.com/a/35904211/4837280

    @author   : Liangjun Zhu

    @changelog:
    - 20-07-21  lj - separated from preprocess.db_mongodb.py to make it more likely a global module
"""
from __future__ import absolute_import, unicode_literals

import sys

from preprocess.db_mongodb import ConnectMongoDB

# this is a pointer to the module object instance itself
this = sys.modules[__name__]

# user specific parameters for their MongoDB server
this.host = '127.0.0.1'
this.port = 27017

# this client will be created once for each process in the entire application
this.client = ConnectMongoDB(ip=this.host, port=this.port).get_conn()
