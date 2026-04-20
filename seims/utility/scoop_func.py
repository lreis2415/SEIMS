"""Utility functions of DEAP package.

    @author   : Liangjun Zhu

    @changelog:
    - 18-10-29 - lj - Extract from other packages.
"""
from __future__ import absolute_import, unicode_literals

import os
# OLD: import scoop
# NEW: Make scoop optional for Docker/non-distributed environments (2026-03-19)
try:
    import scoop
    HAS_SCOOP = True
except ImportError:
    HAS_SCOOP = False


def scoop_log(msg):
    if HAS_SCOOP and os.name != 'nt':
        scoop.logger.warn(msg)
    else:
        print(msg)
