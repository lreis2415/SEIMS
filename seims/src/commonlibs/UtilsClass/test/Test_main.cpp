/*!
 * @brief The main function should only be present once in the test.
 *
 * @version 1.0
 * @authors Liangjun Zhu (zlj@lreis.ac.cn)
 * @revised 12/02/2017 lj Initial version.
 *
 */
#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#pragma warning(disable : 4996) // 'function': was declared deprecated
#pragma warning(disable : 4127) // Use exception for catching assert
#endif
#include "gtest/gtest.h"
#include "utilities.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Create a text file in the directory of executable file.
    string apppath = GetAppPath();
    string txtfile = apppath + "txtfile.txt";
    fstream fs(txtfile.c_str(), ios::app);
    if (fs.is_open()) {
        fs << "File for test!";
        fs.close();
    }

#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
    // Get a checkpoint of the memory after Google Test has been initialized. (not finished yet! by lj)
    // Solving the memory leak problem caused by Google test in an imperfect way in MSVC with VLD.
    // refers: [1] https://stackoverflow.com/questions/12704543/memory-leak-when-using-google-test-on-windows
    //         [2] https://github.com/google/googletest/issues/624 (still unsolved by Google Test)
    //         [3] https://github.com/Tencent/rapidjson/blob/master/test/unittest/unittest.cpp
    _CrtMemState memoryState = { 0 };
    (void)memoryState;
    _CrtMemCheckpoint(&memoryState);
#endif /* Run Visual Leak Detector during Debug */

    int retval = RUN_ALL_TESTS();

#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
    // Check for leaks after tests have run
    // Current Google test constantly leak 2 blocks at exit
    _CrtMemDumpAllObjectsSince(&memoryState);
#endif /* Run Visual Leak Detector during Debug */
    return retval;
}

