/*!
 * \brief A simple text read class
 * \author Junzhi Liu
 * \version 1.0
 * \date June 2010
 */
#ifndef SEIMS_SIMPLE_TEXT_H
#define SEIMS_SIMPLE_TEXT_H
#include "utilities.h"

using namespace std;

/*!
 * \ingroup data
 * \class clsSimpleTxtData
 *
 * \brief read string line from text file
 *
 *
 *
 */
class clsSimpleTxtData {
public:
    //! Constructor, from text file read lines data
    clsSimpleTxtData(string fileName);

    //! Destructor
    ~clsSimpleTxtData(void);

    //! Get line number and data
    void getData(int *nRow, float **data);

    //! Output lines data to \a ostream
    void dump(ostream *fs);

private:
    //! line number
    int m_row;
    //! lines data
    float *m_data;
};

#endif /* SEIMS_SIMPLE_TEXT_H */