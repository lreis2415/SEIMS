/*!
 * \brief Functions for climate related intermediate parameters,
 *          e.g., saturation vapor pressure, max solar radiation.
 * \ingroup data
 * \author LiangJun Zhu
 * \date April 2016
 *
 */
#ifndef SEIMS_CLIMATE_PARAMS_H
#define SEIMS_CLIMATE_PARAMS_H

#include <ctime>

using std::time_t;

/*!
 * \brief Get the year
 * \return int year
 */
int GetYear(time_t date);

/*!
 * \brief Get the month
 * \return int month
 */
int GetMonth(time_t date);

/*!
 * \brief Get the day
 * \return int day
 */
int GetDay(time_t date);

/*!
 * \brief Get the Julian day of one day
 * \return int Julian day
 */
int JulianDay(time_t date);

/*!
 * \brief Calculate latent heat of vaporization(MJ/kg)
 * \param[in] tmean Mean temperature
 * \return Latent heat of vaporization
 */
float LatentHeatVapor(float tmean);

/*!
 * \brief Calculate the max solar radiation for a station of one day
 *
 *
 * \param[in] day Julian day
 * \param[in] lat Latitude (degree) of the station
 * \param[out] day_l day length (hr)
 * \param[out] max_sr The max solar radiation
 */
void MaxSolarRadiation(int day, float lat, float& day_l, float& max_sr);

/*!
 * \brief Calculate mean barometric pressure
 * \param[in] elev elevation of current cell or site
 * \return mean atmospheric pressure (kPa)
 */
float MeanBarometricPressure(float elev);

/*!
 * \brief Calculate psychrometric constant
 * \param[in] elev elevation of current cell or site
 * \param[in] tmean Mean temperature
 * \sa MeanBarometricPressure(), LatentHeatVapor()
 * \return Psychrometric constant
 */
float PsychrometricConst(float tmean, float elev);

/*!
 * \brief Calculates saturation vapor pressure at a given air temperature.
 * \param[in] mean_tmp Mean air temperature(deg C)
 * \return saturation vapor pressure(kPa)
*/
float SaturationVaporPressure(float mean_tmp);

#endif /* SEIMS_CLIMATE_PARAMS_H */
