/*!
 * \file base_VariableData.h
 * \brief Base class used to store input and output variable datasets
 * \author Junzhi Liu
 * \version 1.1
 * \date Jun. 2010
 */

#pragma once

#include <string>
#include "MetadataInfoConst.h"

using namespace std;

/*!
 * \ingroup data
 * \class base_VariableData
 *
 * \brief Base class for variable data
 */
class base_VariableData
{
protected:
    ///< dimension type for the dataset, \sa dimensionTypes
    dimensionTypes m_Dimension;

public:
    //! Constructor (by default, m_Dimention is Unknown)
    base_VariableData(void);

    //! Copy Constructor
    base_VariableData(const base_VariableData &obj);

    //! Destructor
    virtual ~base_VariableData(void);

    //! Assignment operator overload
    virtual base_VariableData &operator=(const base_VariableData &obj);

    //! Returns the dimension type for the object instance
    virtual dimensionTypes Dimension(void);
};

