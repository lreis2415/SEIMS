/*!
 * \file clsSimpleTxtData.h
 * \brief A simple text read class
 * \author Junzhi Liu, Liangjun Zhu
 * \version 1.0
 * \date June 2010
 */
#ifndef SEIMS_SIMPLE_TEXT_H
#define SEIMS_SIMPLE_TEXT_H
#include "basic.h"

using namespace ccgl;

/*!
 * \ingroup data
 * \class clsSimpleTxtData
 * \brief read string line from text file
 *
 */
class clsSimpleTxtData: Interface {
public:
    //! Constructor, from text file read lines data
    explicit clsSimpleTxtData(const string& filename);

    //! Destructor
    ~clsSimpleTxtData();

    //! Get line number and data
    void GetData(int* n_row, float** data);

    //! Output lines data to \a ostream
    void Dump(std::ostream* fs);

private:
    //! line number
    int row_;
    //! lines data
    float* data_;
};

#endif /* SEIMS_SIMPLE_TEXT_H */
