#coding:utf-8
"""Read and output functions related to SEIMS-based model's database.

    @author   : Liangjun Zhu

    @changelog:
    -25-09-11 - lj - Extract from other packages.
"""
from __future__ import absolute_import, unicode_literals

from pymongo import UpdateOne

from preprocess.text import ModelCfgFields, ModelParamFields

# def write_param_values_to_mongodb(coll, param_defs, param_values):
#     coll.update_many({}, {'$unset': {'CALI_VALUES': ''}})
#     for idx, pname in enumerate(param_defs['names']):
#         v2str = ','.join(str(v) for v in param_values[:, idx])
#         coll.find_one_and_update({'NAME': pname}, {'$set': {'CALI_VALUES': v2str}})

def write_cali_param_values_to_mongodb(coll, param_defs, param_values, cfg_name, task_name):
    """Write calibrate parameters into PARAMETERS_SPEC collection in MongoDB.
    Please be aware that for specific SUB_MODEL and TASK, there may exist three
    types of parameter's item:
    1. parameter with IMPACT
    2. parameter with CALI_VALUES
    3. parameter with both IMPACT and CALI_VALUES

    The principle of SEIMS main program when reading calibrated parameters is:
    if calibration_id >= 0, then use CALI_VALUES[calibration_id], ignore IMPACT;
    if calibration_id < 0 and IMPACT exists, then use the IMPACT value.
    """
    if not param_defs:
        return
    if param_values is None or len(param_values) == 0:
        return
    flt = {ModelCfgFields.configname: cfg_name,
           ModelCfgFields.taskname: task_name}
    # Tidy-up: 1) delete existing parameters only with 'CALI_VALUES' (i.e., without IMPACT)
    r1 = coll.delete_many({**flt, ModelParamFields.cali_values: {'$exists': True},
                           ModelParamFields.impact: {'$exists': False}})
    # Tidy-up: 2) unset values in CALI_VALUES if the parameter has both IMPACT and CALI_VALUES
    r2 = coll.update_many({**flt, ModelParamFields.impact: {'$exists': True},
                           ModelParamFields.cali_values: {'$exists': True}},
                          {"$unset": {ModelParamFields.cali_values: ''}})
    print('Deleted specific parameters in PARAMETER_SPEC: %d, '
          'unset CALI_VALUES: %d' % (r1.deleted_count, r2.modified_count))
    # Write
    ops = list()
    for idx, pname in enumerate(param_defs['names']):
        v2str = ",".join(str(v) for v in param_values[:, idx])
        ops.append(UpdateOne({**flt, ModelParamFields.name: pname},
                             {"$set": {ModelParamFields.cali_values: v2str}},
                             upsert=True))
    if ops:
        coll.bulk_write(ops, ordered=False)
