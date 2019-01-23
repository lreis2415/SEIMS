/*!
 * \file SelectTypLocSlpPos.h
 * \date 2015/04/09 14:12
 *
 * \author Liangjun Zhu
 * Contact: zlj@lreis.ac.cn
 *
 * \brief Select typical location of slope position according to RPI value range.
 *
*/
#ifndef SELECT_TYPSLPPOSLOC_H
#define SELECT_TYPSLPPOSLOC_H

#include "commonLib.h"
#include <vector>

#define FREQUENCY_GROUP 100  //!< used to calculate frequency of cell values

using std::vector;

/*!
 * \struct paramExtGrid
 * \brief Constructs parameters for finding typical locations and fuzzy inference
 *
 */
struct paramExtGRID
{
    char name[10];
    char path[MAXLN];
    float minTyp;
    float maxTyp;
    char shape;
    float w1;
    float r1;
    float k1;
    float w2;
    float r2;
    float k2;
};
/*!
 * \struct ExtInfo
 * \brief Constructs frequency distribution array, and store other statistics values
 *
 */
struct ExtInfo
{
    int num; //!< number of terrain attributes values
    float maxValue; //!< maximum value
    float minValue; //!< minimum value
    float interval; //!< (maxValue - minValue) / FREQUENCY_GROUP
    float x[FREQUENCY_GROUP];  //!< x[i] = minValue + interval * (i + 0.5)
    float y[FREQUENCY_GROUP];  //!< y[i] = value count that fall in [XRange[i],XRange[i+1]]
    float XRange[FREQUENCY_GROUP + 1]; //!< XRange[i] = minValue + interval * i
};
/*!
 * \struct DefaultFuzInf
 * \brief  prior expert knowledge of curve shape for fuzzy inference model for a specific topographic attribute
 *
 */
struct DefaultFuzInf
{
    char param[10]; //!< name of topographic attribute
    char shape[4];  //!< prior expert knowledge of curve shape for fuzzy inference models
};

int SelectTypLocSlpPos(char *inconfigfile, int prototag, int paramsNum, paramExtGRID *paramsgrd, int addparamsNum,
                       paramExtGRID *addparamgrd, vector<DefaultFuzInf> fuzinf, float *baseInputParameters,
                       char *typlocfile, char *outconffile, bool writelog, char *logfile);

void dropParam(paramExtGRID &paramgrd);

int SetFuzFuncShape(paramExtGRID &paramgrd, ExtInfo &paramExt, char shape, float fittedCenter, float *allvalues,
                    float MIN_TYPLOC_NUM_PECENT, float MAX_TYPLOC_NUM_PECENT, int SELECTION_MODE,
                    float DEFAULT_SIGMA_MULTIPLIER);

#endif /* SELECT_TYPSLPPOSLOC_H */
