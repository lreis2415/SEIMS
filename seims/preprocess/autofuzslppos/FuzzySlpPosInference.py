"""Prepare configure file for fuzzy slope position inference program.

    @author   : Liangjun Zhu

    @changelog:
    - 15-09-08  - lj - initial implementation.
    - 17-07-30  - lj - reorganize and incorporate with pygeoc.
"""
from __future__ import absolute_import, unicode_literals
import time
from io import open
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import StringClass

from autofuzslppos.Config import get_input_cfgs
from autofuzslppos.ParasComb import combine_inf_conf_parameters
from autofuzslppos.TauDEMExtension import TauDEMExtension


def read_inf_param_from_file(conf):
    """Read fuzzy inference parameters from file."""
    params_list = list()
    with open(conf, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            eles = line.split('\n')[0].split('\t')
            params = StringClass.extract_numeric_values_from_string(line.split('\n')[0])
            if StringClass.string_match(eles[0], 'Parameters') and len(params) >= 6:
                params_list.append([eles[1]] + [eles[3]] + params[-6:])
    return params_list


def fuzzy_inference(cfg):
    """Fuzzy slope position inference."""
    if not cfg.flag_fuzzyinference:
        return 0
    start_t = time.time()
    simif = list()  # similarity file path of each slope position types
    for i, slppos in enumerate(cfg.slppostype):
        if slppos not in cfg.inferparam:
            cfg.inferparam[slppos] = dict()
        simif.append(cfg.singleslpposconf[slppos].fuzslppos)
        if cfg.flag_auto_inferenceparams:  # use automatically recommended parameters
            params_list = read_inf_param_from_file(cfg.singleslpposconf[slppos].infrecommend)
            for p in params_list:
                cfg.inferparam[slppos][p[0]] = p[1:]
        else:  # keep cfg.inferparam[slppos] as it is, and read infconfig for update
            params_list = read_inf_param_from_file(cfg.singleslpposconf[slppos].infconfig)
            for p in params_list:
                # for update
                cfg.inferparam[slppos][p[0]] = p[1:]
                #cfg.selectedtopo[p[0]] = p[1]
                # # for supplement
                # if not p[0] in cfg.inferparam[slppos]:
                #     cfg.inferparam[slppos][p[0]] = p[1:]
                # if not p[0] in cfg.selectedtopo:
                #     cfg.selectedtopo[p[0]] = p[1]

    # In current version, all regional attribute named as 'rpi'
    # Set fuzzy inference parameters for 'rpi' if it does not existed.
    regional_attr_range_dict = dict()
    for slppos in cfg.slppostype:
        # TODO, add some value check
        regional_attr_range_dict[slppos] = cfg.extractrange[slppos]['rpi']
    for i, typ in enumerate(cfg.slppostype):
        cur_rng = regional_attr_range_dict[typ]
        if i == 0:  # for Ridge, S: w1 = Rdg.max-Shd.max
            next_rng = regional_attr_range_dict[cfg.slppostype[i + 1]]
            tempw1 = cur_rng[1] - next_rng[1]
            cur_param = ['S', tempw1, 2, 0.5, 1, 0, 1]
        elif i == len(cfg.slppostype) - 1:  # for Valley, Z: w2 = Fts.max-Vly.max
            before_rng = regional_attr_range_dict[cfg.slppostype[i - 1]]
            tempw2 = before_rng[1] - cur_rng[1]
            cur_param = ['Z', 1, 0, 1, tempw2, 2, 0.5]
        else:
            # for other slope positions, B: w1 = w2 = min(cur.min-next.max, before.min-cur.max)
            next_rng = regional_attr_range_dict[cfg.slppostype[i + 1]]
            before_rng = regional_attr_range_dict[cfg.slppostype[i - 1]]
            tempw = min(cur_rng[0] - next_rng[1], before_rng[0] - cur_rng[1])
            cur_param = ['B', tempw, 2, 0.5, tempw, 2, 0.5]
        if 'rpi' not in cfg.inferparam[typ]:
            cfg.inferparam[typ]['rpi'] = cur_param[:]

    # write fuzzy inference parameters to configuration file and run 'fuzzyslpposinference'
    for i, slppos in enumerate(cfg.slppostype):
        config_info = open(cfg.singleslpposconf[slppos].infconfig, 'w', encoding='utf-8')
        config_info.write('PrototypeGRID\t%s\n' % cfg.singleslpposconf[slppos].typloc)
        config_info.write('ProtoTag\t%d\n' % cfg.slppostag[i])
        config_info.write('ParametersNUM\t%d\n' % len(cfg.inferparam[slppos]))
        for name, param in list(cfg.inferparam[slppos].items()):
            config_info.write('Parameters\t%s\t%s\t%s\t%f\t%f\t%f\t%f\t%f\t%f\n' % (
                name, cfg.selectedtopo[name], param[0], param[1], param[2],
                param[3], param[4], param[5], param[6]))
        config_info.write('DistanceExponentForIDW\t%d\n' % cfg.dist_exp)
        config_info.write('OUTPUT\t%s\n' % cfg.singleslpposconf[slppos].fuzslppos)
        config_info.close()

        TauDEMExtension.fuzzyslpposinference(cfg.proc,
                                             cfg.singleslpposconf[slppos].infconfig,
                                             cfg.ws.output_dir,cfg.mpi_dir, cfg.bin_dir,
                                             cfg.log.all, cfg.log.runtime, cfg.hostfile)

    TauDEMExtension.hardenslppos(cfg.proc, simif, cfg.slppostag,
                                 cfg.slpposresult.harden_slppos,
                                 cfg.slpposresult.max_similarity,
                                 cfg.slpposresult.secharden_slppos,
                                 cfg.slpposresult.secmax_similarity, None, None,
                                 cfg.ws.output_dir, cfg.mpi_dir, cfg.bin_dir,
                                 cfg.log.all, cfg.log.runtime, cfg.hostfile)
    print('Fuzzy slope position calculated done!')
    # Combine fuzzy inference parameters.
    combine_inf_conf_parameters(cfg.slppostype, cfg.singleslpposconf, cfg.slpposresult.infconfig)
    end_t = time.time()
    cost = (end_t - start_t) / 60.
    with open(cfg.log.runtime, 'a', encoding='utf-8') as logf:
        logf.write('Fuzzy Slope Position Inference Time-consuming: %s\n' % repr(cost))
    return cost


def main():
    """TEST CODE"""
    fuzslppos_cfg = get_input_cfgs()
    fuzzy_inference(fuzslppos_cfg)


if __name__ == '__main__':
    main()
