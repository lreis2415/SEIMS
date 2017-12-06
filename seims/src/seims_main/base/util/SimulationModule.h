/*!
 * \brief Parent class for all modules in SEIMS
 *
 * \author Junzhi Liu
 * \version 1.1
 * \date Jul. 2010
 * \revised Liang-Jun Zhu
 * \data Jun. 2016
 */
#ifndef SIMULATION_MOUDULE_BASE
#define SIMULATION_MOUDULE_BASE

#include "ModelException.h"
#include "Scenario.h"  /// added by LJ. 2016-6-14
#include "clsReach.h"
#include "clsSubbasin.h"

#include <string>
#include <fstream>
#include <ostream>
#include <ctime>
#include <cmath>
#include <map>
#include <vector>
#include <iomanip>
#ifdef SUPPORT_OMP
#include <omp.h>
#endif

using namespace std;
using namespace MainBMP;
/*!
 * \enum TimeStepType
 */
enum TimeStepType {
    TIMESTEP_HILLSLOPE,   /**< Hillslope scale */
    TIMESTEP_CHANNEL,     /**< Channel scale */
    TIMESTEP_ECOLOGY,     /**< Ecology scale, currently not necessary? */
    TIMESTEP_SIMULATION   /**< Whole simulation scale */
};

/*!
 * \ingroup Util
 * \class SimulationModule
 * \brief Base module for all simulation modules in SEIMS
 */
class SimulationModule {
public:
    //! Constructor
    SimulationModule();

    //! Destructor
    virtual ~SimulationModule() = default;

    //! Execute the simulation
    virtual int Execute() { return -1; };

    //! Set date time, as well as the sequence number of the entire simulation. Added by LJ for statistics convenient.
    virtual void SetDate(time_t t, int yearIdx) {
        m_date = t;
        m_yearIdx = yearIdx;
    };

    //! Set thread number for OpenMP
    virtual void SetTheadNumber(int threadNum) {
#ifdef SUPPORT_OMP
        omp_set_num_threads(threadNum);
#endif
    };

    //! Set climate data type, P, M, PET etc.
    virtual void SetClimateDataType(float value) {
    };

    //! Set data, DT_Single
    virtual void SetValue(const char *key, float data) {
        throw ModelException("SimulationModule", "SetValue",
                             "Set function of parameter " + string(key) + " is not implemented.");
    };

    //! Set 1D data, by default, DT_Raster1D
    virtual void Set1DData(const char *key, int n, float *data) {
        throw ModelException("SimulationModule", "Set1DData",
                             "Set function of parameter " + string(key) + " is not implemented.");
    };

    //! Set 2D data, by default, DT_Raster2D
    virtual void Set2DData(const char *key, int nRows, int nCols, float **data) {
        throw ModelException("SimulationModule", "Set2DData",
                             "Set function of parameter " + string(key) + " is not implemented.");
    };

    //! Set 1D array data, DT_Array1D
    virtual void Set1DArrayData(const char *key, int n, float *data) {
        throw ModelException("SimulationModule", "Set1DArrayData",
                             "Set function of parameter " + string(key) + " is not implemented.");
    };

    //! Set 2D array data, by default, DT_Array2D
    virtual void Set2DArrayData(const char *key, int nRows, int nCols, float **data) {
        throw ModelException("SimulationModule", "Set2DArrayData",
                             "Set function of parameter " + string(key) + " is not implemented.");
    };

    //! Get value, DT_Single
    virtual void GetValue(const char *key, float *value) {
        throw ModelException("SimulationModule", "GetValue",
                             "Get function of parameter " + string(key) + " is not implemented.");
    };

    //! Get 1D data, by default, DT_Raster1D
    virtual void Get1DData(const char *key, int *n, float **data) {
        throw ModelException("SimulationModule", "Get1DData",
                             "Get function of parameter " + string(key) + " is not implemented.");
    };

    //! Get 2D data, by default, DT_Raster2D
    virtual void Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
        throw ModelException("SimulationModule", "Get2DData",
                             "Get function of parameter " + string(key) + " is not implemented.");
    };

    //! Get 1D Array data, by default, DT_Array1D
    virtual void Get1DArrayData(const char *key, int *n, float **data) {
        throw ModelException("SimulationModule", "Get1DArrayData",
                             "Get function of parameter " + string(key) + " is not implemented.");
    };

    //! Get 2D Array data, by default, DT_Array2D
    virtual void Get2DArrayData(const char *key, int *nRows, int *nCols, float ***data) {
        throw ModelException("SimulationModule", "Get2DArrayData",
                             "Get function of parameter " + string(key) + " is not implemented.");
    };

    //! Set pointer of Scenario class which contains all BMP information. Added by LJ, 2016-6-14
    virtual void SetScenario(Scenario *) {
        throw ModelException("SimulationModule", "SetScenario", "Set scenario function is not implemented.");
    }

    //! Set pointer of clsReaches class which contains all reaches information. Added by LJ, 2016-7-2
    virtual void SetReaches(clsReaches *) {
        throw ModelException("SimulationModule", "SetReaches", "Set reaches function is not implemented.");
    }

    //! Set pointer of clsSubbasins class which contains all subbasins information. Added by LJ, 2016-7-28
    virtual void SetSubbasins(clsSubbasins *) {
        throw ModelException("SimulationModule", "SetSubbasins", "Set subbasins function is not implemented.");
    }

    //! Get time step type
    virtual TimeStepType GetTimeStepType() {
        return TIMESTEP_HILLSLOPE;
    };

    //! Reset subtime step
    virtual void ResetSubTimeStep() {
        m_tsCounter = 1;
    };

    //! Whether the inputs parameters (i.e., parameters derived from other modules) have been set.
    bool IsInputsSetDone() { return m_inputsSetDone; }

    //! Change the status of setting inputs parameters
    void SetInputsDone(bool setDone) { m_inputsSetDone = setDone; }

protected:
    /// date time
    time_t m_date;
    /// index of current year of simulation, e.g., the simulation period from 2010 to 2015,  m_yearIdx is 2 when simulate 2012.
    int m_yearIdx;
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
#define CHECK_DATA(moduleID, expression, param, desc) if ((expression)) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" has not been set: ") + string(desc))
//! CHECK_POINTER is used for 1D or 2D raster and other pointer of data
#define CHECK_POINTER(moduleID, param, desc) if (nullptr == (param)) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" has not been set: ") + string(desc))
//! CHECK_POSITIVE is used for single value that must be positive
#define CHECK_POSITIVE(moduleID, param, desc) if ((param) <= 0) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" has not been set: ") + string(desc))
//! CHECK_NEGATIVE is used for single value that must be negative
#define CHECK_NEGATIVE(moduleID, param, desc) if ((param) >= 0) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" has not been set: ") + string(desc))
//! CHECK_ZERO is used for single value that must not be ZERO
#define CHECK_ZERO(moduleID, param, desc) if ((param) == 0 || FloatEqual(float(param), 0.f)) \
                   throw ModelException(moduleID, "CheckInputData", string(#param) + string(" has not been set: ") + string(desc))

#endif /* SIMULATION_MOUDULE_BASE */
