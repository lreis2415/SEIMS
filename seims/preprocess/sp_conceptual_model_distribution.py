from __future__ import absolute_import, unicode_literals

import os
import sys

from preprocess.config import PreprocessConfig

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import pandas as pd


def strip_df_columns(df: pd.DataFrame):
    df.columns = df.columns.str.strip()


def get_conceptual_subbasin_list(cfg: PreprocessConfig):
    fn = cfg.conceptual_structure
    if fn is None:
        return None
    structures = pd.read_csv(fn, header=0, sep='|', comment='#')
    strip_df_columns(structures)
    subbasin_list = structures['SUBBASINID']
    is_conceptual_list = structures['IS_CONCEPTUAL']
    conceptual_subbasin_list = []
    for i in range(len(structures)):
        subbasin = subbasin_list[i]
        is_conceptual = is_conceptual_list[i]
        # sbbasin 0 means the default config,
        #   other subbasins will use the config of subbasin0 if not specified
        if subbasin == 0 and is_conceptual == 1:
            conceptual_subbasin_list = [0]
            break
        elif subbasin != 0 and is_conceptual == 1:
            conceptual_subbasin_list.append(subbasin)

    return conceptual_subbasin_list
