
from __future__ import absolute_import, unicode_literals

import os
import sys

from preprocess.config import PreprocessConfig

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import pandas as pd

def get_conceptual_model_distribution(cfg:PreprocessConfig):
    fn = cfg.conceptual_structure
    if fn is None:
        return None
    structures = pd.read_csv(fn, header=0, sep='|', comment='#')
    for subbasin, is_conceptual in structures.iterrows():
        is_conceptual = int(is_conceptual)
        if is_conceptual == 1:
            print(subbasin, 'is conceptual')
        else:
            print(subbasin, 'is not conceptual')
    return structures
