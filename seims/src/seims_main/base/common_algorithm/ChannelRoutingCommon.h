/*!
 * \brief Define some common used function in channel routing related modules, e.g., MUSK_CH.
 * \author Liang-Jun Zhu
 * \date 2018-8-7
 */
#ifndef SEIMS_CHANNEL_ROUTING_COMMON_H
#define SEIMS_CHANNEL_ROUTING_COMMON_H

/*!
 * \brief Calculates flow rate or flow velocity using Manning's
 *        equation. If x1 is set to 1, the velocity is calculated. If x1 is set to
 *        cross-sectional area of flow, the flow rate is calculated.
 *        Refers code Qman.f in SWAT.
 * \param[in] x1 cross-sectional flow area or 1, m^2 or none.
 * \param[in] x2 hydraulic radius, m.
 * \param[in] x3 Manning's "n" value for channel.
 * \param[in] x4 average slope of channel, m/m.
 * \return flow rate or flow velocity, m^3/s or m/s.
 */
float manningQ(float x1, float x2, float x3, float x4);



#endif /* SEIMS_CHANNEL_ROUTING_COMMON_H */
