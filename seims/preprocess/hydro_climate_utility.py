import math


class HydroClimateUtilClass(object):
    """Hydro-Climate utility functions."""

    def __init__(self):
        pass

    @staticmethod
    # earth-sun distance
    def dr(doy):
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
    def Rs(doy, n, lat):
        """solar radiation, n is sunshine duration"""
        lat = lat * math.pi / 180.
        a = 0.25
        b = 0.5
        d = HydroClimateUtilClass.dec(doy)
        w = HydroClimateUtilClass.ws(lat, d)
        N = 24. * w / math.pi
        # Extraterrestrial radiation for daily periods
        ra = (24. * 60. * 0.082 * HydroClimateUtilClass.dr(doy) / math.pi) * (
            w * math.sin(lat) * math.sin(d) + math.cos(lat) * math.cos(d) * math.sin(w))
        return (a + b * n / N) * ra
