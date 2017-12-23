#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Main entrance for parameters sensitivity analysis.
    @author   : Liangjun Zhu
    @changelog: 17-12-23  lj - initial implementation.\n
"""
from pygeoc.utils import get_config_parser

from sensitivity import Sensitivity
from config import PSAConfig


def main():
    """MAIN FUNCTION."""
    cf = get_config_parser()
    cfg = PSAConfig(cf)
    saobj = Sensitivity(cfg)
    saobj.read_param_ranges()
    saobj.generate_samples()
    saobj.write_param_values_to_mongodb()
    saobj.evaluate()
    saobj.calc_elementary_effects()


if __name__ == '__main__':
    main()
