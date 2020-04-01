# -*- coding: utf-8 -*-
"""
Created on Fri Mar  6 16:20:03 2020

@author: Administrator
"""
import csv
import os
import subprocess

cmd_pre = ['python', 'db_import_model_parameters.py', '-ini',
           'D:/Programs/SEIMS/data/youwuzhen/workspace/preprocess.ini']
cmd_run_model = ['python', 'run_seims.py', '-ini',
                 'D:/Programs/SEIMS/data/youwuzhen/workspace/runmodel.ini']
cmd_post = ['python', 'main.py', '-ini',
            'D:/Programs/SEIMS/data/youwuzhen/workspace/postprocess.ini']

target_param = 'spcon'
possible_values = ['34.1','33.1','32.1','31.1','30.1']

model_ywz = 'D:/Programs/SEIMS/data/youwuzhen/exp_youwuzhen10m_longterm_model/'
param_file = 'param.cali'
target_file = 'param.cali'  # 'test.cali'

for val in possible_values:
    print('-'*50)
    params = []
    # 修改参数文件
    with open(model_ywz + param_file, newline='') as fp:
        rd = csv.reader(fp)
        for row in rd:
            params.append(row)

    for row in params:
        if row[0] == target_param:  # 假设事先是有这个参数存在的
            row[1] = val

    with open(model_ywz + target_file, 'w', newline='') as fp:
        wr = csv.writer(fp)
        wr.writerows(params)

    print(params)
    # 运行SEIMS
    print('Starting: ', os.getcwd())
    print('Preprocess...')
    os.chdir('preprocess')
    print(os.getcwd())
    subprocess.run(cmd_pre)
    os.chdir(os.pardir)
    print('preprocess finished!')

    print(os.getcwd())
    print('Run model...')
    subprocess.run(cmd_run_model)
    print('run model finished!')

    print('Postprocess...')
    os.chdir('postprocess')
    print(os.getcwd())
    subprocess.run(cmd_post)
    os.chdir(os.pardir)
    print('Postprocess finished!')
