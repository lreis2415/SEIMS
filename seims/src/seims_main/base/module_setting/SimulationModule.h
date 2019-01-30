/*!
 * \file SimulationModule.h
 * \brief Parent class for all modules in SEIMS
 *
 * Changelog:
 *   - 1. 2010-07-31 - jz - Initial implementation.
 *   - 2. 2016-06-14 - lj - Add SetScenario etc. functions.
 *   - 3. 2018-03-03 - lj - Add CHECK_XXX series macros for data checking.
 *
 * \author Junzhi Liu, Liangjun Zhu
 */
#ifndef SIMULATION_MOUDULE_BASE
#define SIMULATION_MOUDULE_BASE

#include "basic.h"
#include "utils_time.h"
#include "Scenario.h"
#include "clsReach.h"
#include "clsSubbasin.h"

#include <string>
#include <ctime>

using namespace ccgl;
using namespace utils_time;
using namespace bmps;

/*!
 * \enum TimeStepType
 * \ingroup module_setting
 * \brief Time step types.
 */
enum TimeStepType {
    TIMESTEP_HILLSLOPE, ///< Hillslope scale
    TIMESTEP_CHANNEL,   ///< Channel scale
    TIMESTEP_SIMULATION ///< Whole simulation scale
};

/*!
 * \ingroup module_setting
 * \class SimulationModule
 * \brief Base module for all simulation modules in SEIMS
 */
class SimulationModule: Interface {
public:
    //! Constructor
    SimulationModule(): m_date(-1), m_yearIdx(-1), m_year(1900), m_month(-1), m_day(-1),
                        m_tsCounter(1), m_inputsSetDone(false) {
    }

    //! Execute the simulation. Return 0 for success.
    virtual int Execute() { return -1; }

    //! Set date time, as well as the sequence number of the entire simulation. Added by LJ for statistics convenient.
    virtual void SetDate(const time_t t, const int year_idx) {
        m_date = t;
        m_yearIdx = year_idx;
        struct tm* date_info = new tm();
        LocalTime(m_date, date_info);
        m_year = date_info->tm_year + 1900;
        m_month = date_info->tm_mon + 1;
        m_day = date_info->tm_mday;
        m_dayOfYear = date_info->tm_yday + 1;
        delete date_info;
    }

    //! Set thread number for OpenMP
    virtual void SetTheadNumber(const int thread_num) {
        SetOpenMPThread(thread_num);
    }

    //! Set climate data type, P, M, PET etc.
    virtual void SetClimateDataType(float value) {
    }

    //! Set data, DT_Single
    virtual void SetValue(const char* key, float value) {
        throw ModelException("SimulationModule", "SetValue",
                             "Set function of parameter " + string(key) + " is not implemented.");
    }

    //! Set single value to array1D by index, used in MPI version for passing values of subbasins
    virtual void SetValueByIndex(const char* key, int index, float value) {
        throw ModelException("SimulationModule", "SetValueByIndex",
                             "Set function of parameter " + string(key) + " is not implemented.");
    }

    //! Set 1D data, by default, DT_Raster1D
    virtual void Set1DData(const char* key, int n, float* data) {
        throw ModelException("SimulationModule", "Set1DData",
                             "Set function of parameter " + string(key) + " is not implemented.");
    }

    //! Set 2D data, by default, DT_Raster2D
    virtual void Set2DData(const char* key, int nrows, int ncols, float** data) {
        throw ModelException("SimulationModule", "Set2DData",
                             "Set function of parameter " + string(key) + " is not implemented.");
    }

    //! Get value, DT_Single
    virtual void GetValue(const char* key, float* value) {
        throw ModelException("SimulationModule", "GetValue",
                             "Get function of parameter " + string(key) + " is not implemented.");
    }

    //! Get 1D data, by default, DT_Raster1D
    virtual void Get1DData(const char* key, int* n, float** data) {
        throw ModelException("SimulationModule", "Get1DData",
                             "Get function of parameter " + string(key) + " is not implemented.");
    }

    //! Get 2D data, by default, DT_Raster2D
    virtual void Get2DData(const char* key, int* nrows, int* ncols, float*** data) {
        throw ModelException("SimulationModule", "Get2DData",
                             "Get function of parameter " + string(key) + " is not implemented.");
    }

    //! Set pointer of Scenario class which contains all BMP information. Added by LJ, 2016-6-14
    virtual void SetScenario(Scenario* sce) {
        throw ModelException("SimulationModule", "SetScenario", "Set scenario function is not implemented.");
    }

    //! Set pointer of clsReaches class which contains all reaches information. Added by LJ, 2016-7-2
    virtual void SetReaches(clsReaches* rches) {
        throw ModelException("SimulationModule", "SetReaches", "Set reaches function is not implemented.");
    }

    //! Set pointer of clsSubbasins class which contains all subbasins information. Added by LJ, 2016-7-28
    virtual void SetSubbasins(clsSubbasins* subbsns) {
        throw ModelException("SimulationModule", "SetSubbasins", "Set subbasins function is not implemented.");
    }

    /*!
     * \brief Check the input data. Make sure all the input data is available.
     *
     *        This function is optional to be overridden.
     *
     * \return bool The validity of the input data.
     */
    virtual bool CheckInputData() { return true; }

    /*!
     * \brief Initialize output variables.
     *
     *        This function is optional to be overridden.
     */
    virtual void InitialOutputs() {}

    /*!
     * \brief Get time step type, default is hillslope process.
     *
     *        Remember to OVERRIDE this function to return other time step type for
     *        routing modules and others if necessary.
     */
    virtual TimeStepType GetTimeStepType() {
        return TIMESTEP_HILLSLOPE;
    }

    //! Reset subtime step
    virtual void ResetSubTimeStep() {
        m_tsCounter = 1;
    }

    //! Whether the inputs parameters (i.e., parameters derived from other modules) have been set.
    bool IsInputsSetDone() { return m_inputsSetDone; }

    //! Change the status of setting inputs parameters
    void SetInputsDone(const bool set_done) { m_inputsSetDone = set_done; }

protected:
    /// date time
    time_t m_date;
    /// index of current year of simulation, e.g., the simulation period from 2010 to 2015,  m_yearIdx is 2 when simulate 2012.
    int m_yearIdx;
    /// year
    int m_year;
    /// month since January - [1,12]
    int m_month;
    /// day of the month - [1,31]
    int m_day;
    /// day of year - [1, 366]
    int m_dayOfYear;
    /// sub-timestep counter
    int m_tsCounter;
    /// Whether the inputs parameters (i.e., parameters derived from other modules) have been set.
    bool m_inputsSetDone;
};

/*!
 * Macros for CheckInputData function
 * BE REMEMBER OF SEMICOLON!
 */
//! CHECK_DATA is used for the unforeseen situation
#define CHECK_DATA(moduleID, expression, desc) if ((expression)) \
                   throw ModelException(moduleID, "CheckInputData", string(desc))
//! CHECK_POINTER is used for 1D or 2D raster and other pointer of data
#define CHECK_POINTER(moduleID, param) if (nullptr == (param)) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" MUST NOT be NULL!"))
//! CHECK_POSITIVE is used for single value that must be positive
#define CHECK_POSITIVE(moduleID, param) if ((param) <= 0) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" MUST be positive!"))
//! CHECK_NONNEGATIVE is used for single value that must be greater or equal than zero
#define CHECK_NONNEGATIVE(moduleID, param) if ((param) < 0) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" MUST be greater or equal than zero!"))
//! CHECK_NEGATIVE is used for single value that must be negative
#define CHECK_NEGATIVE(moduleID, param) if ((param) >= 0) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" MUST be negative!"))
//! CHECK_ZERO is used for single value that must not be ZERO
#define CHECK_ZERO(moduleID, param) if ((param) == 0 || FloatEqual(CVT_FLT(param), 0.f)) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" MUST NOT be zero!"))
//! CHECK_NODATA is used for single value that must not be NODATA_VALUE
#define CHECK_NODATA(moduleID, param) if ((param) == NODATA_VALUE || FloatEqual(CVT_FLT(param), NODATA_VALUE)) \
                     throw ModelException(moduleID, "CheckInputData", string(#param) + string(" MUST NOT be NODATA_VALUE!"))
#endif /* SIMULATION_MOUDULE_BASE */
