#ifndef SEIMS_HEADER
#define SEIMS_HEADER
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
int DiagonalCCW[9] = {0, 0, 1, 0, 1, 0, 1, 0, 1};
#endif
