"""Merge small subbasins to its downstream subbasin.

    @author   : Liangjun Zhu

    @changelog:
    - 17-06-30  lj - initial version modified from QSWAT 1.4
    - 18-02-08  lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals


class MergeSubbasins(object):
    """
    Merged selected subbasin to other subbasin.
    """

    @staticmethod
    def merge_to_downstream_qswat(cfg):
        """Algorithm modified from QSWAT->delineation.py"""
        print("TODO")


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    seims_cfg = parse_ini_configuration()
    MergeSubbasins.merge_to_downstream_qswat(seims_cfg)


if __name__ == "__main__":
    main()
