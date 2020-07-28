"""  Parameters sensitivity analysis of SEIMS-based watershed model.

The invoking format is:

  `python demo_parameters_sensitivity.py -name <demo_watershed_name>`

    @author   : Liangjun Zhu

    @changelog:
    - 19-01-07 - lj - initial implementation.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
import time

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import UtilClass

from parameters_sensitivity.sensitivity import Sensitivity
from test.demo_config import ModelPaths, write_sensitivity_config_file
from test.demo_config import DEMO_MODELS, get_watershed_name


def main():
    wtsd_name = get_watershed_name('Specify watershed name to run parameters sensitivity analysis.')
    if wtsd_name not in list(DEMO_MODELS.keys()):
        print('%s is not one of the available demo watershed: %s' %
              (wtsd_name, ','.join(list(DEMO_MODELS.keys()))))
        exit(-1)

    cur_path = UtilClass.current_path(lambda: 0)
    SEIMS_path = os.path.abspath(cur_path + '../../..')
    model_paths = ModelPaths(SEIMS_path, wtsd_name, DEMO_MODELS[wtsd_name])

    psa_cfg = write_sensitivity_config_file(model_paths, 'sensitivity_analysis.ini')
    print('### START TO PARAMETERS SENSITIVITY ANALYSIS ###')
    start_t = time.time()
    saobj = Sensitivity(psa_cfg)
    saobj.run()
    saobj.plot()
    print('### END OF PARAMETERS SENSITIVITY ANALYSIS ###')
    print('Running time: %.2fs' % (time.time() - start_t))


if __name__ == "__main__":
    main()
