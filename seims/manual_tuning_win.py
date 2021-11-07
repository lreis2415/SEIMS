# -*- coding: utf-8 -*-
"""
Created on Fri Mar  6 16:20:03 2020
@author: Administrator
"""
import csv
import os
import subprocess

SEIMS_ROOT = 'D:/Programs/SEIMS/'
SEIMS_CODE = SEIMS_ROOT + 'seims/'
SEIMS_DATA = SEIMS_ROOT + 'data/'

cmd_pre = ['python', SEIMS_CODE + 'preprocess/db_import_model_parameters.py', '-ini',
           SEIMS_DATA + 'youwuzhen/workspace/preprocess.ini']
cmd_run_model = ['python', SEIMS_CODE + 'run_seims.py', '-ini',
                 SEIMS_DATA + 'youwuzhen/workspace/runmodel.ini']
cmd_post = ['python', SEIMS_CODE + 'postprocess/main.py', '-ini',
            SEIMS_DATA + 'youwuzhen/workspace/postprocess.ini']

target_param = 'MSK_X'
possible_values = ['-0.1', '-0.05', '0', '0.05', '0.1']

model_ywz = SEIMS_DATA + 'youwuzhen/ss_youwuzhen10m_longterm_model/'
param_file = 'param.cali'
target_file = 'param.cali'  # 'test.cali'

for val in possible_values:
    print('-' * 50)
    print(target_param + ': ' + str(val))
    params = []
    # 修改参数文件
    with open(model_ywz + param_file) as fp:
        rd = csv.reader(fp)
        for row in rd:
            params.append(row)

    for row in params:
        if row[0] == target_param:  # Assuming this parameter exists in advance
            row[1] = val

    with open(model_ywz + target_file, 'w') as fp:
        for param in params:
            fp.writelines(','.join(param) + '\n')

    # print(params)
    # import params , run, and postprocess
    devNull = open(os.devnull, 'w')
    print('Preprocess...')
    subprocess.call(cmd_pre, stdout=devNull)
    print('preprocess finished!')

    print('Run model...')
    subprocess.call(cmd_run_model, stdout=devNull)
    print('run model finished!')

    print('Postprocess...')
    subprocess.call(cmd_post)
    print('Postprocess finished!')
