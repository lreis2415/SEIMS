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
# cmd_pre = ['python', 'main.py', '-ini',
#            'D:/Programs/SEIMS/data/youwuzhen/workspace/preprocess.ini']
cmd_run_model = ['python', 'run_seims.py', '-ini',
                 'D:/Programs/SEIMS/data/youwuzhen/workspace/runmodel.ini']
cmd_post = ['python', 'main.py', '-ini',
            'D:/Programs/SEIMS/data/youwuzhen/workspace/postprocess.ini']

# cmd_pre = ['python', 'db_import_model_parameters.py', '-ini',
#            'D:/Programs/SEIMS/data/youwuzhen_demo/workspace/preprocess.ini']
# cmd_pre = ['python', 'main.py', '-ini',
#            'D:/Programs/SEIMS/data/youwuzhen_demo/workspace/preprocess.ini']
# cmd_run_model = ['python', 'run_seims.py', '-ini',
#                  'D:/Programs/SEIMS/data/youwuzhen_demo/workspace/runmodel.ini']
# cmd_post = ['python', 'main.py', '-ini',
#             'D:/Programs/SEIMS/data/youwuzhen_demo/workspace/postprocess.ini']

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
