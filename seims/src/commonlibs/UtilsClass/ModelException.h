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
class ModelException :
    public exception {
public:
    /*!
     * \brief Constructor
     * \param[in] className, functionName, msg \a string
     */
    ModelException(string, string, string);

    //! Destructor (void)
    ~ModelException(void) throw();

    /*!
     * \brief Construct error information (string version)
     * \param[out] \a string error information
     */
    string toString();

    /*!
     * \brief Construct error information (char* version)
     * \param[out] \a char* error information
     */
    const char *what() const throw() {
        string descri = "\n";
        descri = "Class:" + m_className + "\n";
        descri += "Function:" + m_functionName + "\n";
        descri += "Message:" + m_msg;

        return descri.c_str();
    }

private:
    string m_className;
    string m_functionName;
    string m_msg;
};

#endif
