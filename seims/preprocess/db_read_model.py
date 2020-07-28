"""Load data from MongoDB.

    @author   : Liangjun Zhu

    @changelog:
    - 18-01-02  - lj - separated from plot_timeseries.
    - 18-02-09  - lj - compatible with Python3.
    - 20-07-20  - lj - take MongoClient object as argument of ReadModelData class.
"""
from __future__ import absolute_import, unicode_literals
from future.utils import viewitems

import os
import sys
from datetime import datetime
from collections import OrderedDict

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from gridfs import GridFS
from pygeoc.utils import StringClass, is_string
from typing import Dict, List, Tuple, Union, AnyStr, Optional
from preprocess.db_mongodb import MongoClient, MongoQuery
from preprocess.text import DBTableNames, ModelCfgFields, FieldNames, SubbsnStatsName, \
    DataValueFields, DataType, StationFields


class ReadModelData(object):
    def __init__(self, conn, dbname):
        # type: (MongoClient, AnyStr) -> None
        """Initialization.

        Args:
            conn: `MongoClient` instance that can be created by ConnectMongoDB(host, port)
            dbname: Main spatial database name
        """
        self.maindb = conn[dbname]
        self.filein_tab = self.maindb[DBTableNames.main_filein]
        self.fileout_tab = self.maindb[DBTableNames.main_fileout]
        self._climdb_name = self.HydroClimateDBName
        self.climatedb = conn[self._climdb_name]
        self._scenariodb_name = self.ScenarioDBName
        self.scenariodb = conn[self._scenariodb_name]
        self._mode = ''
        self._interval = -1
        # UTCTIME
        self._stime = None
        self._etime = None
        self._outletid = -1
        self._subbasincount = -1
        # OUTPUT items
        self._output_ids = list()  # type: List[AnyStr]
        self._output_items = dict()  # type: Dict[AnyStr, Union[List[AnyStr]]]

    @property
    def HydroClimateDBName(self):
        # type: (...) -> AnyStr
        climtbl = self.maindb[DBTableNames.main_sitelist]
        allitems = climtbl.find()
        if not allitems.count():
            raise RuntimeError('%s Collection is not existed or empty!' %
                               DBTableNames.main_sitelist)
        for item in allitems:
            if FieldNames.db in item:
                self._climdb_name = item.get(FieldNames.db)
                break
        return self._climdb_name

    @property
    def ScenarioDBName(self):
        # type: (...) -> AnyStr
        scentbl = self.maindb[DBTableNames.main_scenario]
        allitems = scentbl.find()
        if not allitems.count():
            raise RuntimeError('%s Collection is not existed or empty!' %
                               DBTableNames.main_scenario)
        for item in allitems:
            if FieldNames.db in item:
                self._scenariodb_name = item.get(FieldNames.db)
                break
        return self._scenariodb_name

    @property
    def Mode(self):
        # type: (...) -> AnyStr
        """Get simulation mode."""
        if self._mode != '':
            return self._mode.upper()
        mode_dict = self.filein_tab.find_one({ModelCfgFields.tag: FieldNames.mode})
        self._mode = mode_dict[ModelCfgFields.value]
        if is_string(self._mode):
            self._mode = str(self._mode)
        return self._mode.upper()

    @property
    def Interval(self):
        # type: (...) -> int
        if self._interval > 0:
            return self._interval
        findinterval = self.filein_tab.find_one({ModelCfgFields.tag: ModelCfgFields.interval})
        self._interval = int(findinterval[ModelCfgFields.value])
        return self._interval

    @property
    def OutletID(self):
        # type: (...) -> int
        if self._outletid > 0:
            return self._outletid
        self._outletid = int(MongoQuery.get_init_parameter_value(self.maindb,
                                                                 SubbsnStatsName.outlet))
        return self._outletid

    @property
    def SubbasinCount(self):
        # type: (...) -> int
        if self._subbasincount > 0:
            return self._subbasincount
        self._subbasincount = int(MongoQuery.get_init_parameter_value(self.maindb,
                                                                      SubbsnStatsName.subbsn_num))
        return self._subbasincount

    @property
    def SimulationPeriod(self):
        # type: (...) -> (datetime, datetime)
        if self._stime is not None and self._etime is not None:
            return self._stime, self._etime
        st = self.filein_tab.find_one({ModelCfgFields.tag:
                                           ModelCfgFields.stime})[ModelCfgFields.value]
        et = self.filein_tab.find_one({ModelCfgFields.tag:
                                           ModelCfgFields.etime})[ModelCfgFields.value]
        st = StringClass.get_datetime(st)
        et = StringClass.get_datetime(et)
        if self._stime is None or st > self._stime:
            self._stime = st
        if self._etime is None or et < self._etime:
            self._etime = et
        if st > self._etime > self._stime:
            self._stime = st
            self._etime = et
        return self._stime, self._etime

    def OutputItems(self):
        # type: (...) -> (List[AnyStr], Dict[AnyStr, Optional[List[AnyStr]]])
        """Read output ID and items from database.

        Returns:
            _output_ids (list): OUTPUTID list
            _output_items (dict): key is core file name of output,
                                  value is None or list of aggregated types
        """
        if self._output_ids and self._output_items:
            return self._output_ids, self._output_items
        cursor = self.fileout_tab.find({'$or': [{ModelCfgFields.use: '1'},
                                                {ModelCfgFields.use: 1}]})
        if cursor is not None:
            for item in cursor:
                self._output_ids.append(item[ModelCfgFields.output_id])
                name = item[ModelCfgFields.filename]
                corename = StringClass.split_string(name, '.')[0]
                types = item[ModelCfgFields.type]
                if StringClass.string_match(types, 'NONE'):
                    self._output_items.setdefault(corename, None)
                else:
                    self._output_items.setdefault(corename, StringClass.split_string(types, '-'))
        return self._output_ids, self._output_items

    def Precipitation(self, subbsn_id, start_time, end_time):
        # type: (int, datetime, datetime) -> List[List[Union[datetime, float]]]
        """
        The precipitation is read according to the subbasin ID.
            Especially when plot a specific subbasin (such as ID 3).
            For the whole basin, the subbasin ID is 0.
        Returns:
            Precipitation data list with the first element as datetime.
            [[Datetime1, value1], [Datetime2, value2], ..., [Datetimen, valuen]]
        """
        pcp_date_value = list()
        sitelist_tab = self.maindb[DBTableNames.main_sitelist]
        findsites = sitelist_tab.find_one({FieldNames.subbasin_id: subbsn_id,
                                           FieldNames.mode: self.Mode})
        if findsites is not None:
            site_liststr = findsites[FieldNames.site_p]
        else:
            raise RuntimeError('Cannot find precipitation site for subbasin %d.' % subbsn_id)
        site_list = StringClass.extract_numeric_values_from_string(site_liststr)
        site_list = [int(v) for v in site_list]
        if len(site_list) == 0:
            raise RuntimeError('Cannot find precipitation site for subbasin %d.' % subbsn_id)

        pcp_dict = OrderedDict()

        for pdata in self.climatedb[DBTableNames.data_values].find(
            {DataValueFields.utc: {"$gte": start_time, '$lte': end_time},
             DataValueFields.type: DataType.p,
             DataValueFields.id: {"$in": site_list}}).sort([(DataValueFields.utc, 1)]):
            curt = pdata[DataValueFields.utc]
            curv = pdata[DataValueFields.value]
            if curt not in pcp_dict:
                pcp_dict[curt] = 0.
            pcp_dict[curt] += curv
        # average
        if len(site_list) > 1:
            for t in pcp_dict:
                pcp_dict[t] /= len(site_list)
        for t, v in pcp_dict.items():
            # print(str(t), v)
            pcp_date_value.append([t, v])
        print('Read precipitation from %s to %s done.' % (start_time.strftime('%c'),
                                                          end_time.strftime('%c')))
        return pcp_date_value

    def Observation(self, subbsn_id, vars, start_time, end_time):
        # type: (int, List[AnyStr], datetime, datetime) -> (List[AnyStr], Dict[datetime, List[float]])
        """Read observation data of given variables.

        Changelog:
          - 1. 2018-8-29 Use None when the observation of one variables is absent.

        Returns:
            1. Observed variable names, [var1, var2, ...]
            2. Observed data dict of selected plotted variables, with UTCDATETIME.
               {Datetime: [value_of_var1, value_of_var2, ...], ...}
        """
        vars_existed = list()
        data_dict = OrderedDict()

        coll_list = self.climatedb.collection_names()
        if DBTableNames.observes not in coll_list:
            return None, None
        isoutlet = 0
        if subbsn_id == self.OutletID:
            isoutlet = 1

        def get_observed_name(name):
            """To avoid the prefix of subbasin number."""
            if '_' in name:
                name = name.split('_')[1]
            return name

        def get_basename(name):
            """Get base variable name, e.g., SED for SED and SEDConc."""
            name = get_observed_name(name)
            if 'Conc' in name:
                name = name.split('Conc')[0]
            return name

        siteTbl = self.climatedb[DBTableNames.sites]
        obsTbl = self.climatedb[DBTableNames.observes]
        for i, param_name in enumerate(vars):
            site_items = siteTbl.find_one({StationFields.type: get_basename(param_name),
                                           StationFields.outlet: isoutlet,
                                           StationFields.subbsn: subbsn_id})

            if site_items is None:
                continue
            site_id = site_items.get(StationFields.id)
            for obs in obsTbl.find({DataValueFields.utc: {"$gte": start_time, '$lte': end_time},
                                    DataValueFields.type: get_observed_name(param_name),
                                    DataValueFields.id: site_id}).sort([(DataValueFields.utc, 1)]):

                if param_name not in vars_existed:
                    vars_existed.append(param_name)
                curt = obs[DataValueFields.utc]
                curv = obs[DataValueFields.value]
                if curt not in data_dict:
                    data_dict[curt] = [None] * len(vars)
                data_dict[curt][i] = curv
        if not vars_existed:
            return None, None

        # remove the redundant None in data_dict, in case of len(vars_existed) != len(vars)
        delidx = list()
        for i, vname in enumerate(vars):
            if vname not in vars_existed:
                delidx.append(i)
        delidx.reverse()
        for dt, adata in list(data_dict.items()):
            for i in delidx:
                del adata[i]

        print('Read observation data of %s from %s to %s done.' % (','.join(vars_existed),
                                                                   start_time.strftime('%c'),
                                                                   end_time.strftime('%c')))
        return vars_existed, data_dict

    def CleanOutputGridFs(self, scenario_id=-1, calibration_id=-1):
        # type: (int, int) -> None
        """Delete Output GridFS files in OUTPUT collection."""
        self.OutputItems()
        if self._output_items is None:  # No outputs
            return
        output_gfs = GridFS(self.maindb, DBTableNames.gridfs_output)
        for corename, types in viewitems(self._output_items):
            if types is None:
                continue
            # The format of filename of OUTPUT by SEIMS MPI version is:
            #   <SubbasinID>_CoreFileName_ScenarioID_CalibrationID
            #   If no ScenarioID or CalibrationID, i.e., with a value of -1, just left blank.
            #  e.g.,
            #    - 1_SED_OL_SUM_1_ means ScenarioID is 1 and Calibration ID is -1
            #    - 1_SED_OL_SUM__ means ScenarioID is -1 and Calibration ID is -1
            #    - 1_SED_OL_SUM_0_2 means ScenarioID is 0 and Calibration ID is 2
            regex_str = '(|\\d+_)%s(|_\\S+)' % corename
            regex_str += '_' if scenario_id < 0 else '_%d' % scenario_id
            regex_str += '_' if calibration_id < 0 else '_%d' % calibration_id
            for i in output_gfs.find({'filename': {'$regex': regex_str}}):
                output_gfs.delete(i._id)

    def CleanSpatialGridFs(self, scenario_id):
        # type: (int) -> None
        """Delete Spatial GridFS files in Main database."""
        spatial_gfs = GridFS(self.maindb, DBTableNames.gridfs_spatial)
        # If there are any GridFS in Sptial collection are generated during scenario analysis,
        #   the format of such GridFS file is: <SubbasinID>_<CoreFileName>_<ScenarioID>
        # e.g., SLPPOS_UNITS_12345678
        regex_str = '\\d+_\\S+_%d' % scenario_id
        for i in spatial_gfs.find({'filename': {'$regex': regex_str}}):
            spatial_gfs.delete(i._id)

    def CleanScenariosConfiguration(self, scenario_ids):
        # type: (Union[int, List[int], Tuple[int]]) -> None
        """Delete scenario data by IDs in MongoDB."""
        if not isinstance(scenario_ids, list) or not isinstance(scenario_ids, tuple):
            scenario_ids = [scenario_ids]
        collection = self.scenariodb[DBTableNames.scenarios]
        for _id in scenario_ids:
            collection.remove({'ID': _id})
            print('Delete scenario: %d in MongoDB completed!' % _id)


def main():
    """Functional tests."""
    import datetime
    from preprocess.db_mongodb import ConnectMongoDB

    host = '127.0.0.1'
    port = 27017
    dbname = 'youwuzhen10m_longterm_model'
    stime = datetime.datetime(2013, 1, 1, 0, 0)
    etime = datetime.datetime(2013, 12, 31, 0, 0)

    client = ConnectMongoDB(host, port).get_conn()

    rd = ReadModelData(client, dbname)
    print(rd.HydroClimateDBName)
    print(rd.Precipitation(4, stime, etime))
    print(rd.Observation(4, ['Q'], stime, etime))


if __name__ == "__main__":
    main()
