/*!
 * \file DataCenter.h
 * \brief Data center for running SEIMS
 *        including configuration, input data, output data, etc.
 *        All interaction with database should be implemented here.
 *
 * Changelog:
 *   - 1. 2018-03-01 - lj - Refactor the constructor and move SetData from ModuleFactory class.
 *   - 2. 2018-09-19 - lj - Separate load data from SetData. Compatible with optional parameters.
 *   - 3. 2021-04-06 - lj - Add fdir_method_ to handle different flow direction algorithms.
 *   - 4. 2022-08-20 - lj - Change float to FLTPT.
 *
 * \author Liangjun Zhu
 */
#ifndef SEIMS_DATA_CENTER_H
#define SEIMS_DATA_CENTER_H

#include <unordered_set>

#include "db_mongoc.h"

#include "seims.h"
#include "ModuleFactory.h"
#include "invoke.h"
#include "InputStation.h"
#include "SettingsInput.h"
#include "SettingsOutput.h"
#include "clsReach.h"
#include "clsSubbasin.h"
#include "Scenario.h"

#include "MetadataInfo.h"
#include "clsInterpolationWeightData.h"

/*!
 * \ingroup data
 * \class DataCenter
 * \brief Base class of Data center for SEIMS
 * \version 1.3
 */
class DataCenter: Interface {
public:
    /*!
     * \brief Constructor
     * \param[in] input_args Input arguments of SEIMS
     * \param[in] factory SEIMS modules factory
     * \param[in] subbasin_id Subbasin ID, 0 is the default for entire watershed
     */
    DataCenter(InputArgs* input_args, ModuleFactory* factory, int subbasin_id = 0);

    //! Destructor
    ~DataCenter();

    /**** virtual functions dependent on database IO *****/

    /*!
     * \brief Make sure all the required data are presented
     */
    virtual bool CheckModelPreparedData() = 0;
    /*!
     * \brief Read climate site data
     */
    virtual void ReadClimateSiteList() = 0;
    /*!
     * \brief Read initial and calibrated parameters
     * \todo Should initial parameters in DB separate integer or floating point number?
     */
    virtual bool ReadParametersInDB() = 0;
    /*!
    * \brief Get subbasin number and outlet ID
    */
    virtual int ReadIntParameterInDB(const char* param_name) = 0;
    /*!
     * \brief Output calibrated parameters to txt file
     */
    void DumpCaliParametersInDB();
    /*!
     * \brief Read raster data, both 1D and 2D, and insert to m_rsMap
     * \param[in] remote_filename Raster file name.
     * \param[in] flt_rst Float raster data
     */
    virtual bool ReadRasterData(const string& remote_filename, FloatRaster*& flt_rst,
        STRING_MAP* opts = nullptr) = 0;
    /*!
     * \brief Read raster data, both 1D and 2D, and insert to m_rsMap
     * \param[in] remote_filename Raster file name.
     * \param[in] int_rst Integer raster data
     */
    virtual bool ReadRasterData(const string& remote_filename, IntRaster*& int_rst,
        STRING_MAP* opts = nullptr) = 0;
    /*!
     * \brief Read interpolated weight data and insert to m_weightDataMap
     * \param[in] remote_filename Data file name
     * \param[out] num Data length
     * \param[out] stations Number of stations
     * \param[out] data returned data
     */
    virtual void ReadItpWeightData(const string& remote_filename, int& num, int& stations, FLTPT**& data,
        STRING_MAP* opts = nullptr) = 0;
    /*!
     * \brief Read 1D array data
     * \param[in] remote_filename Data file name
     * \param[out] num Data length
     * \param[out] data returned data
     */
    virtual void Read1DArrayData(const string& remote_filename, int& num, FLTPT*& data,
        STRING_MAP* opts = nullptr) = 0;
    /*!
     * \brief Read 1D integer array data
     * \param[in] remote_filename Data file name
     * \param[out] num Data length
     * \param[out] data returned integer data
     */
    virtual void Read1DArrayData(const string& remote_filename, int& num, int*& data,
        STRING_MAP* opts = nullptr) = 0;
    /*!
     * \brief Read 2D array data and insert to array2d_map_
     *
     * The matrix format is as follows:\n
     *                           5  (Row number)        \n
     *               RowIdx      0  1  2  3  4 (ColIdx) \n
     *                  0        1  9.                  \n
     *                  1        2  8. 1.               \n
     *                  2        2  5. 2.               \n
     *                  3        1  2.                  \n
     *                  4        4  2. 5. 1. 8.         \n
     *
     * i.e., the first element in each row is the valid number of the current row.
     *
     * \param[in] remote_filename data file name
     * \param[out] rows first dimension of the 2D Array, i.e., Rows
     * \param[out] cols second dimension of the 2D Array, i.e., Cols. If each col are different, set cols to 1.
     * \param[out] data returned data
     */
    virtual void Read2DArrayData(const string& remote_filename, int& rows, int& cols, FLTPT**& data,
        STRING_MAP* opts = nullptr) = 0;
    // Read 2D integer array data and insert to array2d_int_map_
    virtual void Read2DArrayData(const string& remote_filename, int& rows, int& cols, int**& data,
        STRING_MAP* opts = nullptr) = 0;
    /*!
     * \brief Read IUH data and insert to m_2DArrayMap
     * \param[in] remote_filename data file name
     * \param[out] n valid cell number
     * \param[out] data returned data
     */
    virtual void ReadIuhData(const string& remote_filename, int& n, FLTPT**& data,
        STRING_MAP* opts = nullptr) = 0;
    /*!
     * \brief Make lapse 2D array data and insert to m_2DArrayMap
     * \param[in] remote_filename data file name
     * \param[out] rows first dimension of the 2D Array, i.e., Rows
     * \param[out] cols second dimension of the 2D Array, i.e., Cols
     * \param[out] data returned data
     */
    virtual void SetLapseData(const string& remote_filename, int& rows, int& cols, FLTPT**& data);
    /*!
     * \brief Set Raster data for Scenario data
     * \return True if set successfully, otherwise false.
     */
    virtual bool SetRasterForScenario() = 0;

public:
    /**** Load or update data ****/
    /*!
     * \brief Check out whether the adjustment is needed.
     * \param[in] para_name Parameter name which may match one of the parameters in `init_params_`.
     */
    bool CheckAdjustment(const string& para_name);

    bool CheckAdjustmentInt(const string& para_name);

    /*!
     * \brief Read and adjust (if necessary) 1D/2D raster data from Database.
     * \param[in] para_name Parameter name, e.g., Slope
     * \param[in] remote_filename Actual file/data name stored in Database, e.g., 0_SLOPE
     * \param[in] is_optional Optional parameters won't raise exception when loaded failed
     */
    void LoadAdjustRasterData(const string& para_name, const string& remote_filename,
                              bool is_optional = false, STRING_MAP* opts = nullptr);

    /*!
     * \brief Read and adjust (if necessary) 1D/2D integer raster data from Database.
     * \param[in] para_name Parameter name, e.g., Landuse
     * \param[in] remote_filename Actual file/data name stored in Database, e.g., 0_LANDUSE
     * \param[in] is_optional Optional parameters won't raise exception when loaded failed
     */
    void LoadAdjustIntRasterData(const string& para_name, const string& remote_filename,
                                 bool is_optional = false, STRING_MAP* opts = nullptr);

    /*!
     * \brief Read and adjust (if necessary) 1D array data from Database.
     *        Currently, there may no parameters are allowed to be adjusted.
     * \param[in] para_name Parameter name
     * \param[in] remote_filename Actual file/data name stored in Database
     * \param[in] is_optional Optional parameters won't raise exception when loaded failed
     */
    void LoadAdjust1DArrayData(const string& para_name, const string& remote_filename,
                               bool is_optional = false, STRING_MAP* opts = nullptr);

    void LoadAdjustInt1DArrayData(const string& para_name, const string& remote_filename,
                                  bool is_optional = false, STRING_MAP* opts = nullptr);

    /*!
     * \brief Read and adjust (if necessary) 2D array data from Database.
     *        Currently, there may no parameters are allowed to be adjusted.
     * \param[in] para_name Parameter name
     * \param[in] remote_filename Actual file/data name stored in Database
     */
    void LoadAdjust2DArrayData(const string& para_name, const string& remote_filename, STRING_MAP* opts = nullptr);

    void LoadAdjustInt2DArrayData(const string& para_name, const string& remote_filename, STRING_MAP* opts = nullptr);

    //! Load data for each module, return time span
    double LoadParametersForModules(vector<SimulationModule *>& modules);

    //! Set data for modules, include all datatype
    void SetData(SEIMSModuleSetting* setting, ParamInfo<FLTPT>* param,
                 SimulationModule* p_module, STRING_MAP* opts = nullptr);

    //! Set integer data for modules, include all datatype
    void SetData(SEIMSModuleSetting* setting, ParamInfo<int>* param,
                 SimulationModule* p_module, STRING_MAP* opts = nullptr);

    //! Set single Value
    void SetValue(ParamInfo<FLTPT>* param, SimulationModule* p_module);

    //! Set single integer Value
    void SetValue(ParamInfo<int>* param, SimulationModule* p_module);

    //! Set 1D Data
    void Set1DData(const string& para_name, const string& remote_filename,
                   SimulationModule* p_module, bool is_optional = false, STRING_MAP* opts = nullptr);

    void Set1DDataInt(const string& para_name, const string& remote_filename,
                      SimulationModule* p_module, bool is_optional = false, STRING_MAP* opts = nullptr);

    //! Set 2D Data
    void Set2DData(const string& para_name, const string& remote_filename,
                   SimulationModule* p_module, bool is_optional = false, STRING_MAP* opts = nullptr);

    void Set2DDataInt(const string& para_name, const string& remote_filename,
                      SimulationModule* p_module, bool is_optional = false, STRING_MAP* opts = nullptr);

    //! Set raster data
    void SetRaster(const string& para_name, const string& remote_filename,
                   SimulationModule* p_module, bool is_optional = false, STRING_MAP* opts = nullptr);

    void SetRasterInt(const string& para_name, const string& remote_filename,
                      SimulationModule* p_module, bool is_optional = false, STRING_MAP* opts = nullptr);

    //! Set BMPs Scenario data
    void SetScenario(SimulationModule* p_module, bool is_optional = false);

    //! Set Reaches information
    void SetReaches(SimulationModule* p_module);

    //! Set Subbasins information
    void SetSubbasins(SimulationModule* p_module);

	//void SetReachDepthData(SimulationModule* p_module);

    //! Update inputs, such climate data.
    void UpdateInput(vector<SimulationModule *>& modules, time_t t);

    /*!
    * \brief Update model parameters (value, 1D raster, and 2D raster, etc.) by Scenario, e.g., areal BMPs.
    *
    * changelog:
    *   - 1. Added by Huiran GAO, Feb. 2017
    *   - 2. Redesigned by Liangjun Zhu, 08/16/17
	*   - 3. Add time parameter by Shen Shen, Feb. 2021
    *
    * \sa BMPArealStructFactory
    * \sa BMPArealStruct
    */
    void UpdateScenarioParametersStable(int subbsn_id);

    bool UpdateScenarioParametersDynamic(int subbsn_id, time_t t);

    /**** Accessors: Set and Get *****/

    string GetModelName() const { return model_name_; }
    string GetProjectPath() const { return model_path_; }
    string GetFileInFullPath() const { return file_in_file_; }
    string GetFileOutFullPath() const { return file_out_file_; }
    string GetFileCfgFullPath() const { return file_cfg_file_; }
    LayeringMethod GetLayeringMethod() const { return lyr_method_; }
    FlowDirMethod GetFlowDirectionMethod() const { return fdir_method_; }
    int GetSubbasinID() const { return subbasin_id_; }
    int GetScenarioID() const { return scenario_id_; }
    int GetCalibrationID() const { return calibration_id_; }
    int GetThreadNumber() const { return thread_num_; }
    bool UseScenario() const { return use_scenario_; }
    string GetOutputScenePath() const { return output_path_; }
    string GetModelMode() const { return model_mode_; }
    int GetSubbasinsCount() const { return n_subbasins_; }
    int GetOutletID() const { return outlet_id_; }
    SettingsInput* GetSettingInput() { return input_; }
    SettingsOutput* GetSettingOutput() { return output_; }
    InputStation* GetClimateStation() { return clim_station_; }
    clsSubbasins* GetSubbasinData() { return subbasins_; }
    clsReaches* GetReachesData() { return reaches_; }
    Scenario* GetScenarioData() { return use_scenario_ ? scenario_ : nullptr; }
    IntRaster* GetMaskData() { return mask_raster_; }
    map<string, FloatRaster *>& GetRasterDataMap() { return rs_map_; }
    map<string, ParamInfo<FLTPT> *>& GetInitParameters() { return init_params_; }
    map<string, FLTPT*>& Get1DArrayMap() { return array1d_map_; }
    map<string, int>& Get1DArrayLenMap() { return array1d_len_map_; }
    map<string, FLTPT**>& Get2DArrayMap() { return array2d_map_; }
    map<string, int>& Get2DArrayRowsMap() { return array2d_rows_map_; }
    map<string, int>& Get2DArrayColsMap() { return array2d_cols_map_; }
    /*!
    * \brief Get file.in configuration
    */
    virtual bool GetFileInStringVector();
    /*!
    * \brief Get file.out configuration
    */
    virtual bool GetFileOutVector() = 0;
    /*!
     * \brief Check date of output settings
     */
    void UpdateOutputDate(time_t start_time, time_t end_time);

protected:
    string model_name_;                    ///< Model name, e.g., model_dianbu30m_longterm
    const string model_path_;              ///< Model path
    string file_in_file_;                  ///< file.in full path
    string file_out_file_;                 ///< file.out full path
    string file_cfg_file_;                 ///< config.fig full path
    const LayeringMethod lyr_method_;      ///< Layering method
    const FlowDirMethod fdir_method_;      ///< Flow direction method
    const int subbasin_id_;                ///< Subbasin ID
    const int scenario_id_;                ///< Scenario ID
    const int calibration_id_;             ///< Calibration ID
    const int mpi_rank_;                   ///< Rank ID for MPI, starts from 0 to mpi_size_ - 1
    const int mpi_size_;                   ///< Rank size for MPI
    const int thread_num_;                 ///< Thread number for OpenMP
    bool use_scenario_;                    ///< Model Scenario
    string output_path_;                   ///< Output path (with / in the end) according to m_outputScene
    vector<string> file_in_strs_;          ///< file.in configuration
    vector<OrgOutItem> origin_out_items_;  ///< file.out configuration
    string model_mode_;                    ///< Storm or Longterm model
    int n_subbasins_;                      ///< Number of subbasins
    int outlet_id_;                        ///< Outlet subbasin ID
    ModuleFactory* factory_;               ///< Module factory
    SettingsInput* input_;                 ///< The basic input settings
    SettingsOutput* output_;               ///< The user-defined outputs, Q, SED, etc
    InputStation* clim_station_;           ///< data of input HydroClimate stations
    Scenario* scenario_;                   ///< BMPs Scenario data
    clsReaches* reaches_;                  ///< Reaches information
    clsSubbasins* subbasins_;              ///< Subbasins information
    bool is_subbasin_conceptual;
    bool is_lumped;
    IntRaster* mask_raster_;               ///< Mask data
    map<string, FloatRaster *> rs_map_;    ///< Map of spatial data, both 1D and 2D
    map<string, IntRaster*> rs_int_map_;   ///< Map of spatial data with integer, both 1D and 2D
	FloatRaster* ch_depth_;                /// reach depth data,every cell has a depth
    map<string, ParamInfo<FLTPT>*> init_params_; ///< Store parameters from Database (PARAMETERS collection)
    map<string, ParamInfo<int>*> init_params_int_; ///< Store integer parameters from Database (PARAMETERS collection)
    map<string, FLTPT*> array1d_map_;      ///< 1D array data map
    map<string, int> array1d_len_map_;     ///< 1D array data length map
    map<string, FLTPT**> array2d_map_;     ///< 2D array data map
    map<string, int> array2d_rows_map_;    ///< Row number of 2D array data map
    map<string, int> array2d_cols_map_;    ///< Col number of 2D array data map
                                           ///<   CAUTION that nCols may not same for all rows
    map<string, int*> array1d_int_map_;    ///< 1D integer array data map
    map<string, int> array1d_int_len_map_; ///< 1D integer array data length map
    map<string, int**> array2d_int_map_;   ///< 2D integer array data map, e.g. FLOWIN_INDEX, FLOWOUT_INDEX, ROUTING_LAYERS
    map<string, int> array2d_int_rows_map_; ///< Row number of 2D array data map
    map<string, int> array2d_int_cols_map_; ///< Col number of 2D array data map
                                            ///<   CAUTION that nCols may not same for all rows
};

#endif /* SEIMS_DATA_CENTER_H */
