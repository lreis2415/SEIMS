#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Hydro-Climate utility class.
    @author   : Junzhi Liu, Liangjun Zhu
    @changelog: 13-01-10  jz - initial implementation
                17-06-23  lj - reformat according to pylint and google style
"""
import math

from seims.preprocess.text import DBTableNames, StationFields


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
        # print x
        return 0.5 * math.pi - math.atan(-math.tan(lat) * math.tan(dec) / math.sqrt(x))

    @staticmethod
    def rs(doy, n, lat):
        """solar radiation, n is sunshine duration"""
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
        from seims.preprocess.db_import_sites import SiteInfo
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


def main():
    """TEST CODE"""
    from seims.preprocess.config import parse_ini_configuration
    from seims.preprocess.db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    hydroclim_db = conn[seims_cfg.climate_db]

    site_m = HydroClimateUtilClass.query_climate_sites(hydroclim_db, 'M')
    site_p = HydroClimateUtilClass.query_climate_sites(hydroclim_db, 'P')

    client.close()


if __name__ == "__main__":
    main()
