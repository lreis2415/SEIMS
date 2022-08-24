/*!
 * \file ccgl.h
 * \brief All namespaces in CCGL.
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * \remarks
 *   - 1. 2018-05-02 - lj - Initially implementation.
 *   - 2. 2018-08-21 - lj - Doxygen comment style check.
 *   - 2. 2021-11-30 - lj - Version 2.0 released!
 *
 * \author Liangjun Zhu, zlj(at)lreis.ac.cn
 * \version 2.0
 */
#ifndef CCGL_H
#define CCGL_H

#include "basic.h"
#include "utils_string.h"
#include "utils_array.h"
#include "utils_math.h"
#include "utils_time.h"
#include "utils_filesystem.h"
#include "db_mongoc.h"
#include "data_raster.hpp"

using namespace ccgl;
using namespace utils_string;
using namespace utils_array;
using namespace utils_math;
using namespace utils_time;
using namespace utils_filesystem;
#ifdef USE_MONGODB
using namespace db_mongoc;
#endif
using namespace data_raster;

#endif /* CCGL_H */
