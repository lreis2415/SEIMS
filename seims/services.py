# coding: utf-8

from flask import Flask,jsonify,request
import subprocess
import os
import time
from shutil import move,rmtree
from configparser import ConfigParser

from scenario_analysis.spatialunits.scenario import extra_process_for_last_generation

app = Flask(__name__)

SEIMS_ROOT = "D:/Programs/SEIMS/"
SEIMS_MODEL_PATH = SEIMS_ROOT+"data/youwuzhen30/demo_youwuzhen30m_longterm_model/"
# BMPS_ORDER_OPT_CONF_FILE = SEIMS_ROOT+"data/youwuzhen30/workspace/scenario_analysis_bmps_order.ini"
BMPS_ORDER_OPT_CONF_FILE_ONLINE = SEIMS_ROOT+"data/youwuzhen30/workspace/scenario_analysis_bmps_order_online.ini"
SEIMS_WORKING_DIR = SEIMS_ROOT + "seims/scenario_analysis/spatialunits/"
SEIMS_SA_SCRIPT_FILE = SEIMS_ROOT + "seims/scenario_analysis/spatialunits/bmps_order_nsga2.py"
SEIMS_SCENARIO_SCRIPT_FILE = SEIMS_ROOT + "seims/scenario_analysis/spatialunits/scenario.py"

@app.route('/')
def test():
    return jsonify(test=True)

@app.route('/onlineOptimization',methods=['GET'])
def simpleOptimization():
    if request.method == 'GET':
        group_id = request.args.get('groupID')
        opt_id = request.args.get('optID')
        # use online optimization config file
        cmd_opt = ['python', SEIMS_SA_SCRIPT_FILE, '-ini',BMPS_ORDER_OPT_CONF_FILE_ONLINE]
        print(cmd_opt)

        # devNull = open(os.devnull, 'w')
        print('Start optimization ...')
        # subprocess.call(cmd_opt, stdout=devNull)
        start = time.time()
        rtn_code = subprocess.call(cmd_opt) # a long time
        time_span = time.time()-start
        print('Finish optimization!', time_span)

        result_dir = SEIMS_MODEL_PATH + 'SA_NSGA2_SLPPOS_HILLSLP_Gen_2_Pop_4/'
        target_path = '{0}group{1}_opt{2}/'.format(SEIMS_MODEL_PATH,group_id,opt_id)
        out_scenarios_path = target_path + 'Scenarios/'
        if os.path.exists(target_path): 
            rmtree(target_path)
        os.mkdir(target_path)
        os.mkdir(out_scenarios_path)
        print('moving files')
        move(result_dir, target_path)
        
        cf = ConfigParser()
        cf.read(BMPS_ORDER_OPT_CONF_FILE_ONLINE)
        extra_process_for_last_generation(cf, target_path+'SA_NSGA2_SLPPOS_HILLSLP_Gen_2_Pop_4/' + 'runtime.log', 2, out_scenarios_path)

        return jsonify(resultCode=1,message='Optimize finished!')


if __name__ == '__main__':
    app.run()
    # app.run(debug=True)
