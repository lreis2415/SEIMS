/*!
 * \brief Functions for climate related intermediate parameters, 
 *  e.g., saturation vapor pressure, max solar radiation.
 * \author LiangJun Zhu
 * \date April 2016
 *
 */
#ifdef MSVC
#pragma once
#endif

#include "ClimateParams.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include "ModelException.h"
#include "util.h"

using namespace std;

//// FUNCTIONS IMPLEMENTATION ////
float MeanBarometricPressure(float &elev)
{
    return 101.3f - elev * (0.01152f - 0.544e-6f * elev);
}

float LatentHeatVapor(float &tmean)
{
    return 2.501f - 0.002361f * tmean;
}

int GetYear(time_t &date)
{
    struct tm dateInfo;
    LocalTime(date, &dateInfo);
    return dateInfo.tm_year;
}

int GetMonth(time_t &date)
{
    struct tm dateInfo;
    LocalTime(date, &dateInfo);
    return dateInfo.tm_mon;
}

int GetDay(time_t &date)
{
    struct tm dateInfo;
    LocalTime(date, &dateInfo);
    return dateInfo.tm_mday;
}

int JulianDay(time_t &date)
{
    struct tm dateInfo;
    LocalTime(date, &dateInfo);
    return dateInfo.tm_yday + 1;
}

void MaxSolarRadiation(int &day, float &lat, float &dayL, float &maxSR)
{
    lat = lat * PI / 180.f;
    //Calculate Daylength
    //calculate solar declination: equation 1:1.1.2 in SWAT Theory 2009, p31
    float sd = asin(0.4f * sin((day - 82.0f) / 58.09f));  /// 365/2pi = 58.09

    //calculate the relative distance of the earth from the sun
    //also called the eccentricity correction factor of the orbit, Duffie and Beckman(1980)
    //equation 1:1.1.1 in SWAT Theory 2009, p30
    float dd = 1.0f + 0.033f * cos(day / 58.09f);
    //daylength = 2 * Acos(-Tan(sd) * Tan(lat)) / omega
    //where the angular velocity of the earth's rotation, omega, is equal
    //to 15 deg/hr or 0.2618 rad/hr and 2/0.2618 = 7.6374
    //equation 2.1.6 in SWAT manual

    float h = 0.0f;
    /// equation 1:1.1.4 in SWAT Theory 2009, p32
    float ch = -sin(lat) * tan(sd) / cos(lat);
    if (ch > 1.f) //ch will be >= 1. if latitude exceeds +/- 66.5 deg in winter
        h = 0.0f;
    else if (ch >= -1.0f)
        h = acos(ch);
    else
        h = PI; //latitude exceeds +/- 66.5 deg in summer
    dayL = 7.6394f * h;
    //Calculate Potential (maximum) Radiation !!
    /// equation 1:1.1.3 in SWAT Theory 2009, p31
    float ys = sin(lat) * sin(sd);
    float yc = cos(lat) * cos(sd);
    /// equation 1:1.1.7 in SWAT Theory 2009, p34
    maxSR = 30.f * dd * (h * ys + yc * sin(h));
}

float PsychrometricConst(float &tmean, float &elev)
{
    float pb = MeanBarometricPressure(elev);
    float xl = LatentHeatVapor(tmean);
    return 1.013e-3f * pb / (0.622f * xl);
}

float SaturationVaporPressure(float &t)
{
    /// Calculate saturation vapor pressure, equation 1:2.3.2 in SWAT Theory 2009, p54
    /// Tetens (1930) and Murray (1967), ee.f in SWAT src.
    if (!FloatEqual(t + 237.3f, UTIL_ZERO))
    {
        float ea = (16.78f * t - 116.9f) / (t + 237.3f);
        return exp(ea);
    }
    return 0.0f;
}
