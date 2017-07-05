#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Import parameters and lookup tables to SQLite database
    @author   : Liangjun Zhu, Fang Shen
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-07-05  lj - reorganize according to pylint and gogle style
"""
import os
import sqlite3

from seims.preprocess.config import parse_ini_configuration
from seims.preprocess.text import ModelParamFields
from seims.preprocess.utility import read_data_items_from_txt, DEFAULT_NODATA
from seims.pygeoc.pygeoc.utils.utils import UtilClass


def txt_to_sqlite(cfg, data_files, db_file):
    """Read data from text file."""
    data_import = dict()  # format: {tabName:[fieldName, Units, dataRows]}
    for data_file in data_files:
        # print data_file
        data_items = read_data_items_from_txt(data_file)
        # print data_items
        if data_file[0] == cfg.sqlitecfgs.Tag_Params:
            field_names = data_items[0][1:]
            units = data_items[1][1:]
            change_idx = field_names.index(ModelParamFields.change) + 1
            impact_idx = field_names.index(ModelParamFields.impact) + 1
            max_idx = field_names.index(ModelParamFields.max) + 1
            min_idx = field_names.index(ModelParamFields.min) + 1
            for i in range(2, len(data_items)):
                cur_data_item = data_items[i]
                # print cur_data_item
                if cur_data_item[change_idx] == ModelParamFields.change_nc\
                        or cur_data_item[change_idx] == '':
                    cur_data_item[impact_idx] = 0
                if cur_data_item[change_idx] == ModelParamFields.change_rc\
                        and cur_data_item[impact_idx] == '':
                    cur_data_item[impact_idx] = 1
                if cur_data_item[change_idx] == ModelParamFields.change_ac\
                        and cur_data_item[impact_idx] == '':
                    cur_data_item[impact_idx] = 0
                if cur_data_item[max_idx] == '':
                    cur_data_item[max_idx] = DEFAULT_NODATA
                if cur_data_item[min_idx] == '':
                    cur_data_item[min_idx] = DEFAULT_NODATA
                if cur_data_item[0] in data_import.keys():
                    data_import[cur_data_item[0]][2].append(cur_data_item[1:])
                else:
                    data_import[cur_data_item[0]] = [
                        field_names, units, [cur_data_item[1:]]]

        else:
            field_names = data_items[0]
            units = data_items[1]
            if data_file[1] not in data_import:
                data_import[data_file[1]] = [field_names, units, []]
            for i in range(2, len(data_items)):
                data_import[data_file[1]][2].append(data_items[i])
    # print data_import
    import_data_to_sqlite(data_import, db_file)


def import_data_to_sqlite(data_import, db_file):
    """Import text to SQLite database."""
    conn = sqlite3.connect(db_file)
    cur = conn.cursor()
    for tab_name, v in data_import.items():
        flds = v[0]
        unit_types = v[1]
        data_row = v[2]
        field_name_str = ''
        for i in range(len(flds)):
            field_name_str += flds[i] + ' ' + unit_types[i] + ' DEFAULT NULL,'
        create_table_sql = '''CREATE TABLE IF NOT EXISTS %s (%s)''' % (tab_name,
                                                                       field_name_str[:-1])
        # print create_table_sql
        cur.execute(create_table_sql)
        # construct a string like '?,?,?,...'
        tmp_arg = ','.join(['?' for i in range(0, len(flds))])
        load_sql = '''insert into %(table)s values (%(arg)s)''' % {'table': tab_name,
                                                                   'arg': tmp_arg}
        # print load_sql
        for singledatarow in data_row:
            cur.execute(load_sql, singledatarow)
    conn.commit()
    cur.close()
    conn.close()


def reconstruct_sqlite_db_file(cfg):
    """ReConstruct SQLite database file."""
    # If the database file existed, DELETE it.
    db_dir = cfg.dirs.import2db
    UtilClass.mkdir(db_dir)
    sql_path = cfg.sqlitecfgs.sqlite_file
    if os.path.exists(sql_path):
        os.remove(sql_path)
    data_files = [[cfg.sqlitecfgs.Tag_Params, cfg.sqlitecfgs.init_params_file]]
    for df in cfg.sqlitecfgs.lookup_tabs:
        data_files.append([cfg.sqlitecfgs.Tag_Lookup, cfg.txt_db_dir + os.sep + df + '.txt'])
    txt_to_sqlite(cfg, data_files, sql_path)


def main():
    """TEST CODE"""
    seims_cfg = parse_ini_configuration()
    reconstruct_sqlite_db_file(seims_cfg)


if __name__ == "__main__":
    main()
