/*!
 * \brief ModelException class
 * \author Junzhi Liu
 * \version 1.1
 * \date Jun. 2010
 */
#ifndef MODEL_EXCEPTION
#define MODEL_EXCEPTION

#include <exception>
#include <string>

using namespace std;

/*!
 * \ingroup util
 * \class ModelException
 * \brief Print the exception message
 */
class ModelException : public exception {
public:
    /*!
     * \brief Constructor
     * \param[in] className, functionName, msg \a string
     */
    ModelException(string className, string functionName, string msg);

    //! Destructor (void)
    ~ModelException() throw() override {};

    /*!
     * \brief Construct error information (string version)
     * \return error information
     */
    string toString();

    /*!
     * \brief Overload function to construct error information
     * \return \a char* error information
     */
    const char *what() const throw() {
        string descri = "\n";
        descri = "Class:" + m_className + "\n";
        descri += "Function:" + m_functionName + "\n";
        descri += "Message:" + m_msg;
        const char *desc_char = descri.c_str();
        return desc_char;
    }

private:
    string m_className;
    string m_functionName;
    string m_msg;
};

#endif /* MODEL_EXCEPTION */
