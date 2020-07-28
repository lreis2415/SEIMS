"""@package autofuzslppos
Python for Automatic Fuzzy Slope Positions including data preparation, running model, etc.

 Currently, the five basic slope position types are supported, i.e., ridge,
     shoulder slope, back slope, foot slope and valley.
 TODO, 11 slope positions considering the concavity and convexity along both
     the contour and profile directions will be considered in the future.

 [1] Qin, C.-Z., Zhu, A.-X., Shi, X., Li, B.-L., Pei, T., Zhou, C.-H., 2009.
        Quantification of spatial gradation of slope positions.
        Geomorphology 110, 152–161.
        doi:10.1016/j.geomorph.2009.04.003

 [2] Zhu, L.-J., Zhu, A.-X., Qin, C.-Z., Liu, J.-Z., 2018.
        Automatic approach to deriving fuzzy slope positions.
        Geomorphology 304, 173-183.

                              -------------------
        author               : Liangjun Zhu, Chengzhi Qin
        copyright            : (C) 2015 - 2019 Lreis, IGSNRR, CAS
        email                : zlj@lreis.ac.cn
 ******************************************************************************
 *                                                                            *
 *   AutoFuzSlpPos is distributed for Research and/or Education only, any     *
 *   commercial purpose will be FORBIDDEN. SEIMS is an open-source project,   *
 *   but without ANY WARRANTY, WITHOUT even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS for A PARTICULAR PURPOSE.                     *
 *   See the GNU General Public License for more details.                     *
 *                                                                            *
 ******************************************************************************/
"""
__author__ = 'Liangjun Zhu'
__email__ = 'zlj@lreis.ac.cn'
__version__ = '1.2.1'
