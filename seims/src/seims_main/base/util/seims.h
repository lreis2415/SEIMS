/*!
 * \brief The SEIMS related definitions and utilities header.
 * \author Liang-Jun Zhu
 * \date 2017-3-22
 */
#pragma once
#include "text.h"
#include "utilities.h"
/*!
 * \enum LayeringMethod
 * \brief Grid layering method for routing and parallel computing
 */
enum LayeringMethod {
    /// layering-from-source method
        UP_DOWN,
    /// layering-from-outlet method
        DOWN_UP
};
enum FlowDirectionMethod {
    TauDEM = 0,
    ArcGIS = 1
};
/*
 *\brief Whether diagonal counter clockwise from east
 *       the first element is set to 0, for indexing convenient.
 *          1  0  1
 *          0     0
 *          1  0  1
 *       e.g. the corresponding D8 flow direction of TauDEM rule:
 *          4  3  2
 *          5     1
 *          6  7  8
 */
const int DiagonalCCW[9] = {0, 0, 1, 0, 1, 0, 1, 0, 1};
/*
 *	Common used const.
 */
const float _23 = 0.6666666666666666f;
const float SQ2 = 1.4142135623730951f;
