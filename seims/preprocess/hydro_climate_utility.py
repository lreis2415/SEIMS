"""Hydro-Climate utility class.

    @author   : Junzhi Liu, Liangjun Zhu

    @changelog:
    - 13-01-10  - jz - initial implementation
    - 17-06-23  - lj - reformat according to pylint and google style
    - 18-02-08  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import math
import time
from datetime import datetime, timedelta
from io import open
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import StringClass, MathClass

from preprocess.text import DBTableNames, StationFields, DataValueFields


class HydroClimateUtilClass(object):
    """Hydro-Climate utility functions."""

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def dr(doy):
        """earth-sun distance"""
        return 1. + 0.033 * math.cos(2. * math.pi * doy / 365.)

    @staticmethod
    def dec(doy):
        """Declination."""
        return 0.409 * math.sin(2. * math.pi * doy / 365. - 1.39)

    @staticmethod
    def ws(lat, dec):
        """sunset hour angle"""
        x = 1. - math.pow(math.tan(lat), 2.) * math.pow(math.tan(dec), 2.)
        if x < 0:
            x = 0.00001
        return 0.5 * math.pi - math.atan(-math.tan(lat) * math.tan(dec) / math.sqrt(x))

    @staticmethod
    def rs(doy, n, lat):
        """solar radiation (MJ/m2), n is sunshine duration (hour)"""
        lat = lat * math.pi / 180.
        a = 0.25
        b = 0.5
        d = HydroClimateUtilClass.dec(doy)
        w = HydroClimateUtilClass.ws(lat, d)
        nn = 24. * w / math.pi
        # Extraterrestrial radiation for daily periods
        ra = (24. * 60. * 0.082 * HydroClimateUtilClass.dr(doy) / math.pi) * \
             (w * math.sin(lat) * math.sin(d) + math.cos(lat) * math.cos(d) * math.sin(w))
        return (a + b * n / nn) * ra

    @staticmethod
    def query_climate_sites(clim_db, site_type):
        """Query climate sites information, return a dict with stationID as key."""
        from .db_import_sites import SiteInfo
        sites_loc = dict()
        sites_coll = clim_db[DBTableNames.sites]
        find_results = sites_coll.find({StationFields.type: site_type})
        for dic in find_results:
            sites_loc[dic[StationFields.id]] = SiteInfo(dic[StationFields.id],
                                                        dic[StationFields.name],
                                                        dic[StationFields.lat],
                                                        dic[StationFields.lon],
                                                        dic[StationFields.x],
                                                        dic[StationFields.y],
                                                        dic[StationFields.elev])
        return sites_loc

    @staticmethod
    def get_time_system_from_data_file(in_file):
        # type: (str) -> (str, int)
        """Get the time system from the data file. The basic format is:
           #<time_system> [<time_zone>], e.g., #LOCALTIME 8, #LOCALTIME -2, #UTCTIME

        Returns:
            time_sys: 'UTCTIME' or 'LOCALTIME'
            time_zone(int): Positive for West time zone, and negative for East.
        """
        time_sys = 'LOCALTIME'
        time_zone = time.timezone // 3600
        with open(in_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        for line in lines:
            str_line = line.strip()
            # for LF in LFs:
            #     if LF in line:
            #         str_line = line.split(LF)[0]
            #         break
            if str_line[0] != '#':
                break
            if str_line.lower().find('utc') >= 0:
                time_sys = 'UTCTIME'
                time_zone = 0
                break
            if str_line.lower().find('local') >= 0:
                line_list = StringClass.split_string(str_line, [' ', ','])
                if len(line_list) == 2 and MathClass.isnumerical(line_list[1]):
                    time_zone = -1 * int(line_list[1])
                break
        return time_sys, time_zone

    @staticmethod
    def get_utcdatetime_from_field_values(flds, values, tsys, tzone=None):
        """Get datetime from field-value lists.

        Returns:
            utctime
        """
        cur_y = 0
        cur_m = 0
        cur_d = 0
        cur_hh = 0
        cur_mm = 0
        cur_ss = 0
        dt = None
        for i, fld in enumerate(flds):
            if StringClass.string_match(fld, DataValueFields.dt):
                dt = StringClass.get_datetime(values[i])
            elif StringClass.string_match(fld, DataValueFields.y):
                cur_y = int(values[i])
            elif StringClass.string_match(fld, DataValueFields.m):
                cur_m = int(values[i])
            elif StringClass.string_match(fld, DataValueFields.d):
                cur_d = int(values[i])
            elif StringClass.string_match(fld, DataValueFields.hour):
                cur_hh = int(values[i])
            elif StringClass.string_match(fld, DataValueFields.minute):
                cur_mm = int(values[i])
            elif StringClass.string_match(fld, DataValueFields.second):
                cur_ss = int(values[i])
        # Get datetime and utc/local transformation
        if dt is None:  # 'DATETIME' is not existed
            if cur_y < 1900 or cur_m <= 0 and cur_d <= 0:
                raise ValueError("Can not find TIME information from "
                                 "fields: %s" % ' '.join(fld for fld in flds))
            else:
                dt = datetime(cur_y, cur_m, cur_d, cur_hh, cur_mm, cur_ss)
        if not StringClass.string_match(tsys, 'UTCTIME'):
            if tzone is None:
                tzone = time.timezone // 3600  # positive value for WEST
            dt += timedelta(minutes=tzone * 60)
        return dt


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    from preprocess.db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    hydroclim_db = conn[seims_cfg.climate_db]

    site_m = HydroClimateUtilClass.query_climate_sites(hydroclim_db, 'M')
    site_p = HydroClimateUtilClass.query_climate_sites(hydroclim_db, 'P')

    client.close()


if __name__ == "__main__":
    main()
