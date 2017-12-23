#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base class of parameters sensitivity analysis.
    @author   : Liangjun Zhu
    @changelog: 17-12-22  lj - initial implementation.\n
"""

from pygeoc.utils import get_config_parser
from SALib.sample.morris import sample as morris_spl
from SALib.analyze.morris import analyze as morris_alz

from preprocess.db_mongodb import ConnectMongoDB
from config import PSAConfig
from preprocess.utility import read_data_items_from_txt
from evaluate import evaluate_model_response, build_seims_model


class Sensitivity(object):
    """Base class of Sensitivity Analysis."""

    def __init__(self, psa_cfg):
        """
        Initialization.
        Args:
            psa_cfg: PSAConfig object.
        """
        self.cfg = psa_cfg
        self.param_defs = dict()
        self.param_values = None
        self.run_count = 0
        self.output_nameIndex = dict()
        self.output_values = list()
        self.morris_si = dict()

    def read_param_ranges(self):
        """Read param_rng.def file

           name,lower_bound,upper_bound,group,dist
           (group and dist are optional)

            e.g.,
             Param1,0,1[,Group1][,dist1]
             Param2,0,1[,Group2][,dist2]
             Param3,0,1[,Group3][,dist3]

        Returns:
            a dictionary containing:
            - names - the names of the parameters
            - bounds - a list of lists of lower and upper bounds
            - num_vars - a scalar indicating the number of variables
                         (the length of names)
            - groups - a list of group names (strings) for each variable
            - dists - a list of distributions for the problem,
                        None if not specified or all uniform
        """
        client = ConnectMongoDB(self.cfg.hostname, self.cfg.port)
        conn = client.get_conn()
        db = conn[self.cfg.spatial_db]
        collection = db['PARAMETERS']

        names = list()
        bounds = list()
        groups = list()
        dists = list()
        num_vars = 0
        items = read_data_items_from_txt(self.cfg.param_range_def)
        for item in items:
            if len(item) < 3:
                continue
            # find parameter name, print warning message if not existed
            cursor = collection.find({'NAME': item[0]}, no_cursor_timeout=True)
            if not cursor.count():
                print ('WARNING: parameter %s is not existed!' % item[0])
                continue
            num_vars += 1
            names.append(item[0])
            bounds.append([float(item[1]), float(item[2])])
            # If the fourth column does not contain a group name, use
            # the parameter name
            if len(item) >= 4:
                groups.append(item[3])
            else:
                groups.append(item[0])
            if len(item) >= 5:
                dists.append(item[4])
            else:
                dists.append('unif')
        if groups == names:
            groups = None
        elif len(set(groups)) == 1:
            raise ValueError('''Only one group defined, results will not be
                    meaningful''')

        # setting dists to none if all are uniform
        # because non-uniform scaling is not needed
        if all([d == 'unif' for d in dists]):
            dists = None

        self.param_defs = {'names': names, 'bounds': bounds,
                           'num_vars': num_vars, 'groups': groups, 'dists': dists}

    def generate_samples(self):
        """Sampling and write to a single file and MongoDB 'PARAMETERS' collection"""
        self.param_values = morris_spl(self.param_defs, self.cfg.N,
                                       self.cfg.num_levels, self.cfg.grid_jump,
                                       optimal_trajectories=self.cfg.optimal_t,
                                       local_optimization=self.cfg.local_opt)
        self.run_count = len(self.param_values)

    def write_param_values_to_mongodb(self):
        # update Parameters collection in MongoDB
        client = ConnectMongoDB(self.cfg.hostname, self.cfg.port)
        conn = client.get_conn()
        db = conn[self.cfg.spatial_db]
        collection = db['PARAMETERS']
        for idx, pname in enumerate(self.param_defs['names']):
            v2str = ','.join(str(v) for v in self.param_values[:, idx])
            collection.find_one_and_update({'NAME': pname}, {'$set': {'CALI_VALUES': v2str}})
        client.close()

    def evaluate(self):
        """Run SEIMS for objective output variables, and write out.
        """
        # TODO, need to think about a elegant way to define and calculate ouput variables.
        self.output_nameIndex = {'Q': 0, 'SED': 1}

        cali_seqs = range(self.run_count)
        model_cfg_dict = {'bin_dir': self.cfg.seims_bin, 'model_dir': self.cfg.model_dir,
                          'nthread': self.cfg.seims_nthread, 'lyrmethod': self.cfg.seims_lyrmethod,
                          'hostname': self.cfg.hostname, 'port': self.cfg.port,
                          'scenario_id': 0}
        cali_models = map(build_seims_model, [model_cfg_dict] * self.run_count, cali_seqs)
        try:
            # parallel on multiprocesor or clusters using SCOOP
            from scoop import futures
            self.output_values = futures.map(evaluate_model_response, cali_models)
        except ImportError or ImportWarning:
            # serial
            self.output_values = map(evaluate_model_response, cali_models)
        print (self.output_values)

    def calc_elementary_effects(self):
        """Calculate Morris elementary effects.
           It is worth to be noticed that evaluate() allows to return several output variables,
           hence we should calculate each of them separately.
        """
        for k, v in self.output_nameIndex:
            print (k)
            tmp_Si = morris_alz(self.param_defs,
                                self.param_values,
                                self.output_values[:, v],
                                conf_level=0.95, print_to_console=True,
                                num_levels=self.cfg.num_levels,
                                grid_jump=self.cfg.grid_jump)
            self.morris_si[k] = tmp_Si
        print (self.morris_si)


if __name__ == '__main__':
    cf = get_config_parser()
    cfg = PSAConfig(cf)
    saobj = Sensitivity(cfg)
    saobj.read_param_ranges()
    saobj.generate_samples()

    print (saobj.param_defs)
    print (len(saobj.param_values))
