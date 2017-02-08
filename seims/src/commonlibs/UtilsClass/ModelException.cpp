/*!
 * \brief Implement of ModelException class
 * \author Junzhi Liu
 * \version 1.1
 * \date Jun. 2010
 *
 * 
 */
#include "ModelException.h"

ModelException::ModelException(string className, string functionName, string msg)
{
    exception();
    m_className = className;
    m_functionName = functionName;
    m_msg = msg;
}

ModelException::~ModelException(void) throw()
{
}


string ModelException::toString()
{
    string descri = "";
    descri = "Class:" + m_className + "\n";
    descri += "Function:" + m_functionName + "\n";
    descri += "Message:" + m_msg;

    return descri;
}
