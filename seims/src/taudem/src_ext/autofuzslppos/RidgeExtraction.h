/*!
 * \date date 2016/11/21
 *
 * \author Liangjun Zhu
 * Contact: zlj@lreis.ac.cn
 *
 * \brief
 *
*/
#ifndef RIDGE_EXTRACTION_H
#define RIDGE_EXTRACTION_H

#include "commonLib.h"
#include <fstream>
#include <vector>

using std::vector;

/*!
 * \struct RdgSrc
 * \brief Ridge source with some attribute at a location (col, row)
 *
 */
struct RdgSrc
{
    struct node Coor;
    float elev;
};
/* Get the coordinates of downstream cells
 * D8 and D-inf flow model are supported.
 */
vector<node> downstream_coors(float &dirv, int &row, int &col);
/* Extract ridge sources according to flow direction and elevation.
 *
 *
 */
int ExtractRidges(char *dirsfile, char *felfile, float threshold, char *rdgsrcfile);

#endif /* RIDGE_EXTRACTION_H */
