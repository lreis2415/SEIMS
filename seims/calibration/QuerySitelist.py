import errno
import os
import sys
from pathlib import Path

import pandas as pd

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))
from preprocess.text import DataValueFields


def copy_to_data_values(
    model_path,
    start_time_utc,
    end_time_utc,
):
    original_dir = os.path.join(model_path, "DataValuesAll")
    out_dir = os.path.join(model_path, "DataValues")
    try:
        os.makedirs(out_dir)
    except OSError as exc:
        if exc.errno != errno.EEXIST:
            raise
        pass
    # iterate all csv using os.walk
    # for forcing_file in original_dir.glob("*.csv"):
    for root, dirs, files in os.walk(original_dir):
        for forcing_file in files:
            forcing_file = Path(root) / forcing_file
            if not forcing_file.name.endswith('.csv'):
                continue
            # df = pd.read_csv(forcing_file)
            df = pd.read_csv(forcing_file)
            # columns: date_utc, value1, value2
            # parse utc date
            df[DataValueFields.utc] = pd.to_datetime(df[DataValueFields.utc])
            # select date
            df = df[(df[DataValueFields.utc] >= start_time_utc) & (df[DataValueFields.utc] <= end_time_utc)]
            # remove the utc column
            df = df.drop(columns=[DataValueFields.utc])
            # write to new csv
            # out_path = out_dir.joinpath(forcing_file.name)
            out_path = os.path.join(out_dir, forcing_file.name)
            # if out_path.exists():
            #     out_path.unlink()
            if os.path.exists(out_path):
                os.remove(out_path)
            df.to_csv(out_path, index=False)
