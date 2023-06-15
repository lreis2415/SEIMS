import os
import shutil

from pygeoc.utils import UtilClass

from preprocess.db_import_model_parameters import ImportParam2Mongo
from test_conceptual.demo_config import *
from run_seims import MainSEIMS
from postprocess.plot_timeseries import TimeSeriesPlots
import pandas as pd
from pathlib import Path

outdir = Path('C:\src\SEIMS\data\youwuzhen\demo_youwuzhen30m_conceptual_model\OUTPUT_D8_DOWN_UP-0-')
pngdir = outdir / 'png'

out_pngdir = outdir.parent/'out-png'
out_pngdir.mkdir(exist_ok=True)
# clean all files in outdir, including its subfolders
for f in outdir.glob('*'):
    if f.is_file():
        f.unlink()
    else:
        shutil.rmtree(f)


wtsd_name = 'youwuzhen'

cur_path = UtilClass.current_path(lambda: 0)
SEIMS_path = os.path.abspath(cur_path + '../../..')
model_paths = ModelPaths(SEIMS_path, wtsd_name, DEMO_MODELS[wtsd_name])
seims_cfg = write_preprocess_config_file(model_paths, 'preprocess.ini')

ImportParam2Mongo.workflow(seims_cfg)

runmodel_cfg = write_runmodel_config_file(model_paths, 'runmodel.ini')

seims_obj = MainSEIMS(args_dict=runmodel_cfg.ConfigDict)
seims_obj.run()
for l in seims_obj.runlogs:
    print(l)

params = pd.read_csv('C:\src\SEIMS\data\youwuzhen\demo_youwuzhen30m_conceptual_model\param.cali', index_col=0 ,header=None)
params_list = [
    params.loc['SOILTHICK'].values[0],
    params.loc['GR4J_X2'].values[0],
    params.loc['GR4J_X3'].values[0],
    params.loc['GR4J_X4'].values[0],
]
params_str = '-'.join([str(p) for p in params_list])

scenario_id = 0
calibration_id = -1
post_cfg = write_postprocess_config_file(model_paths, 'postprocess.ini',
                                         scenario_id, calibration_id)
plt = TimeSeriesPlots(post_cfg).generate_plots(params_str)
plt.show()

# copy png files in pngdir out from outdir, overwrite if exists
for f in pngdir.glob('*.png'):
    shutil.copy(f, out_pngdir)

