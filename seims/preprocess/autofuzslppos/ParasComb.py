"""Combine parameter results for text files for further analysis.

    @author   : Liangjun Zhu

    @changelog:
    - 15-03-20  - lj - initial implementation.
    - 17-07-30  - lj - reorganize and incorporate with pygeoc.
"""
from __future__ import absolute_import, unicode_literals
from io import open
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import StringClass

from autofuzslppos.Config import get_input_cfgs


def read_ext_conf(ext_file):
    """Read extract typical location configuration file."""
    with open(ext_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    ext_conf_data = list()
    # Read the number of records
    rec_num = int(lines[1].split('\n')[0].split('\t')[1])
    ext_conf_data.append(rec_num)
    for i in range(0, rec_num):
        temp_conf = lines[i + 2].split('\n')[0].split('\t')
        if StringClass.string_in_list(temp_conf[1], ['profc', 'horizc', 'planc']):
            min_v = 1000. * float(temp_conf[3])
            max_v = 1000. * float(temp_conf[4])
        else:
            min_v = float(temp_conf[3])
            max_v = float(temp_conf[4])
        min_s = str(round(min_v, 2))
        max_s = str(round(max_v, 2))
        ext_conf_data.append([temp_conf[1], min_s, max_s])
    return ext_conf_data


def combine_ext_conf_parameters(slppostypes, extconf_dict, combinedconf):
    """Combine extraction configuration parameters to a single file."""
    ext_conf_lines = list()
    ext_conf_lines.append([' '])
    for slppos in slppostypes:
        temp_ext_data = read_ext_conf(extconf_dict[slppos].extconfig)
        temp_ext_conf_line = [slppos]
        temp_ext_conf_line += [''] * temp_ext_data[0]
        for i in range(temp_ext_data[0]):
            if temp_ext_data[i + 1][0] not in ext_conf_lines[0]:
                ext_conf_lines[0].append(temp_ext_data[i + 1][0])
            idx = ext_conf_lines[0].index(temp_ext_data[i + 1][0])
            if idx >= len(temp_ext_conf_line):
                temp_ext_conf_line += [''] * (idx - len(temp_ext_conf_line) + 1)
            temp_ext_conf_line[idx] = '[%s, %s]' % (repr(temp_ext_data[i + 1][1]),
                                                    repr(temp_ext_data[i + 1][2]))
        ext_conf_lines.append(temp_ext_conf_line)
    # print(extConfLines)
    with open(combinedconf, 'w', encoding='utf-8') as f:
        for line in ext_conf_lines:
            for elem in line:
                f.write('%s\t' % elem)
            f.write('\n')


def read_inf_conf(ext_file):
    """Read fuzzy inference configuration file."""
    f = open(ext_file)
    lines = f.readlines()
    f.close()
    inf_conf_data = []
    # Read the number of records
    rec_num = int(lines[2].split('\n')[0].split('\t')[1])
    inf_conf_data.append(rec_num)
    for i in range(0, rec_num):
        temp_conf = lines[i + 3].split('\n')[0].split('\t')
        if StringClass.string_in_list(temp_conf[1], ['profc', 'horizc', 'planc']):
            w1_v = 1000. * float(temp_conf[4])
            w2_v = 1000. * float(temp_conf[7])
        else:
            w1_v = float(temp_conf[4])
            w2_v = float(temp_conf[7])
        w1S = str(round(w1_v, 2))
        w2S = str(round(w2_v, 2))
        inf_conf_data.append([temp_conf[1], temp_conf[3], w1S, w2S])
    return inf_conf_data


def combine_inf_conf_parameters(slppostypes, infconf_dict, combinedconf):
    """Combine fuzzy inference configuration parameters to a single file."""
    inf_conf_lines = list()
    inf_conf_lines.append([' '])
    for slppos in slppostypes:
        temp_inf_data = read_inf_conf(infconf_dict[slppos].infconfig)
        temp_inf_conf_line = [slppos]
        temp_inf_conf_line += [''] * temp_inf_data[0]
        for i in range(temp_inf_data[0]):
            if temp_inf_data[i + 1][0] not in inf_conf_lines[0]:
                inf_conf_lines[0].append(temp_inf_data[i + 1][0])
            idx = inf_conf_lines[0].index(temp_inf_data[i + 1][0])
            if idx >= len(temp_inf_conf_line):
                temp_inf_conf_line += [''] * (idx - len(temp_inf_conf_line) + 1)
            if temp_inf_data[i + 1][1] == 'B':
                if temp_inf_data[i + 1][2] == temp_inf_data[i + 1][3]:
                    temp_inf_conf_line[idx] = temp_inf_data[i + 1][1] + ': w1 = w2 = ' + \
                                              temp_inf_data[i + 1][2]
                else:
                    temp_inf_conf_line[idx] = temp_inf_data[i + 1][1] + ': w1 = ' + \
                                              temp_inf_data[i + 1][2] + ', w2 = ' + \
                                              temp_inf_data[i + 1][3]
            elif temp_inf_data[i + 1][1] == 'S':
                temp_inf_conf_line[idx] = temp_inf_data[i + 1][1] + ': w1 = ' + \
                                          temp_inf_data[i + 1][2]
            else:
                temp_inf_conf_line[idx] = temp_inf_data[i + 1][1] + ': w2 = ' + \
                                          temp_inf_data[i + 1][3]
        inf_conf_lines.append(temp_inf_conf_line)
    # print(infConfLines)
    with open(combinedconf, 'w', encoding='utf-8') as f:
        for line in inf_conf_lines:
            for elem in line:
                f.write('%s\t' % elem)
            f.write('\n')


def main():
    """TEST CODE."""
    fuzslppos_cfg = get_input_cfgs()
    types = fuzslppos_cfg.slppostype
    slpposconf = fuzslppos_cfg.singleslpposconf
    combinedextconf = fuzslppos_cfg.slpposresult.extconfig
    combinedinfconf = fuzslppos_cfg.slpposresult.infconfig

    combine_ext_conf_parameters(types, slpposconf, combinedextconf)
    combine_inf_conf_parameters(types, slpposconf, combinedinfconf)


if __name__ == '__main__':
    main()
