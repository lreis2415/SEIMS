"""Prototype-based fuzzy slope positions.

    @author   : Liangjun Zhu

    @changelog:
    - 15-03-20  - lj - initial implementation.
    - 17-07-30  - lj - reorganize and incorporate with pygeoc.
"""
from __future__ import absolute_import, unicode_literals
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from autofuzslppos.Config import get_input_cfgs
from autofuzslppos.FuzzySlpPosInference import fuzzy_inference
from autofuzslppos.PreProcessing import pre_processing
from autofuzslppos.SelectTypLoc import extract_typical_location


def main():
    """Main workflow."""
    fuzslppos_cfg = get_input_cfgs()

    pre_processing(fuzslppos_cfg)
    extract_typical_location(fuzslppos_cfg)
    fuzzy_inference(fuzslppos_cfg)


if __name__ == '__main__':
    main()
