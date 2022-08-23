/*!
 * \file FuzzySlpPosInference.h
 * \date 2015/04/09 14:12
 *
 * \author Liangjun Zhu
 * Contact: zlj@lreis.ac.cn
 *
 * \brief Fuzzy slope position inference function header
 *
*/
#ifndef FUZZYSLPPOS_INFERENCE_H
#define FUZZYSLPPOS_INFERENCE_H
#include "commonLib.h"

/*!
 * \struct paramInfGRID
 * \brief Parameters for fuzzy inference of terrain attribute
 *
 */
typedef struct paramInfGRID
{
    char path[MAXLN]; //!< Grid file path
    char shape[2]; //!< fuzzy membership function shape, B, S, or Z
    float w1;
    float r1;
    float k1;
    float w2;
    float r2;
    float k2;
} paramInfGRID;
/*!
 * \struct TypLocAttr
 * \brief Terrain attribute values at the typical location (col, row)
 *
 */
typedef struct TypLocAttr
{
    int col, row;
    float *Value;
} TypLocAttr;

int FuzzySlpPosInf(char *protofile, int prototag, int paramsnum, paramInfGRID *paramsgrd, float exponent,
                   char *simfile);

#endif /* FUZZYSLPPOS_INFERENCE_H */
