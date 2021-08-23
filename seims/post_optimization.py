# coding:utf-8
import os
import zipfile
import time

MODEL_PATH = 'D:/Programs/SEIMS/data/youwuzhen/ss_youwuzhen10m_longterm_model'
OPTI_GEN = 50
OPTI_POP = 72
OPTI_FOLDER = 'SA_NSGA2_SLPPOS_HILLSLP_Gen_{}_Pop_{}'.format(OPTI_GEN,OPTI_POP)


def zipdir(path, ziph):
    # ziph is zipfile handle
    for root, dirs, files in os.walk(path):
        for file in files:
            ziph.write(os.path.join(root, file),
                       os.path.relpath(os.path.join(root, file),
                                       os.path.join(path, '..')))


def zipit(dir_list, zip_name):
    zipf = zipfile.ZipFile(zip_name, 'w', zipfile.ZIP_DEFLATED)
    for dir in dir_list:
        zipdir(dir, zipf)
    zipf.close()


if __name__ == '__main__':
    # read runtime.log
    log_filename = MODEL_PATH + '/' + OPTI_FOLDER + '/' + 'runtime.log'
    last_gen_str = 'Generation: {}'.format(OPTI_GEN)
    zip_dirs = []
    zip_dirs.append(MODEL_PATH + '/' + OPTI_FOLDER)# add SA* folder
    with open(log_filename) as logfp:
        for line in logfp:
            if last_gen_str not in line:
                continue
            next(logfp)# title line
            for target_line in logfp:
                items = target_line.split('\t')
                sce_folder = '{}/OUTPUT{}'.format(MODEL_PATH, items[1])
                # add to zip list
                zip_dirs.append(sce_folder)

    zip_filename = '{}-{}.zip'.format(OPTI_FOLDER,time.strftime('%Y%m%d%H%M%S'))
    zipit(zip_dirs, zip_filename)


