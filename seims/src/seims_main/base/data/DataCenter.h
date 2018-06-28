/*!
 * \brief Data center for running SEIMS
 *        including configuration, input data, output data, etc.
 *        All interaction with database should be implemented here.
 * \author Liangjun Zhu
 * \date May 2017
 * \revision  2018-03-01  lj - refactor the constructor and move SetData from \sa ModuleFactory class
 */
#ifndef SEIMS_DATA_CENTER_H
#define SEIMS_DATA_CENTER_H

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
#include "clsInterpolationWeightData.h"

/*!
 * \ingroup data
 * \class DataCenter
 * \brief Base class of Data center for SEIMS
 * \version 1.2
 */
class DataCenter: Interface {
public:
    /*!
     * \brief Constructor
     * \param[in] input_args Input arguments of SEIMS, \sa InputArgs
     * \param[in] factory SEIMS modules factory, \sa ModuleFactory
     * \param[in] subbsn_id Subbasin ID, 0 is the default for entire watershed
     */
    DataCenter(InputArgs* input_args, ModuleFactory* factory, int subbsn_id = 0);

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
     */
    virtual bool ReadParametersInDB() = 0;
    /*!
     * \brief Output calibrated parameters to txt file
     */
    void DumpCaliParametersInDB();
    /*!
     * \brief Read raster data, both 1D and 2D, and insert to m_rsMap
     * \param[in] remote_filename Raster file name.
     */
    virtual FloatRaster* ReadRasterData(const string& remote_filename) = 0;
    /*!
     * \brief Read interpolated weight data and insert to m_weightDataMap
     * \param[in] remote_filename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void ReadItpWeightData(const string& remote_filename, int& num, float*& data) = 0;
    /*!
     * \brief Read 1D array data
     * \param[in] param_name \string parameter name
     * \param[in] remote_filename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void Read1DArrayData(const string& param_name, const string& remote_filename,
                                 int& num, float*& data) = 0;
    /*!
     * \brief Read 2D array data and insert to m_2DArrayMap
     * The matrix format is as follows:
     *                           5  (Row number)
     *          RowIdx\ColIdx    0  1  2  3  4
     *                  0        1  9.
     *                  1        2  8. 1.
     *                  2        2  5. 2.
     *                  3        1  2.
     *                  4        4  2. 5. 1. 8.
     * i.e., the first element in each row is the valid number of the current row.
     * \param[in] remote_filename \string data file name
     * \param[out] rows \int&, first dimension of the 2D Array, i.e., Rows
     * \param[out] cols \int&, second dimension of the 2D Array, i.e., Cols. If each col are different, set cols to 1.
     * \param[out] data \float**&, returned data
     */
    virtual void Read2DArrayData(const string& remote_filename, int& rows, int& cols, float**& data) = 0;
    /*!
     * \brief Read IUH data and insert to m_2DArrayMap
     * \param[in] remote_filename \string data file name
     * \param[out] n \int&, valid cell number
     * \param[out] data \float*&, returned data
     */
    virtual void ReadIuhData(const string& remote_filename, int& n, float**& data) = 0;
    /*!
     * \brief Make lapse 2D array data and insert to m_2DArrayMap
     * \param[in] remote_filename \string data file name
     * \param[out] rows \int&, first dimension of the 2D Array, i.e., Rows
     * \param[out] cols \int&, second dimension of the 2D Array, i.e., Cols
     * \param[out] data \float**&, returned data
     */
    virtual void SetLapseData(const string& remote_filename, int& rows, int& cols, float**& data);
    /*!
     * \brief Set Raster data for Scenario data
     * \return True if set successfully, otherwise false.
     */
    virtual bool SetRasterForScenario() = 0;

public:
    /**** Load or update data ****/
    //! Load data for each module, return time span
    double LoadDataForModules(vector<SimulationModule *>& modules);

    //! Set data for modules, include all datatype
    void SetData(SEIMSModuleSetting* setting, ParamInfo* param,
                 SimulationModule* p_module, bool vertital_itp);

    //! Set single Value
    void SetValue(ParamInfo* param, SimulationModule* p_module);

    //! Set 1D Data
    void Set1DData(const string& para_name, const string& remote_filename,
                   SimulationModule* p_module, bool vertital_itp);

    //! Set 2D Data
    void Set2DData(const string& para_name, const string& remote_filename, SimulationModule* p_module);

    //! Set raster data
    void SetRaster(const string& para_name, const string& remote_filename, SimulationModule* p_module);

    //! Set BMPs Scenario data
    void SetScenario(SimulationModule* p_module);

    //! Set Reaches information
    void SetReaches(SimulationModule* p_module);

    //! Set Subbasins information
    void SetSubbasins(SimulationModule* p_module);

    //! Update inputs, such climate data.
    void UpdateInput(vector<SimulationModule *>& modules, time_t t);

    /*!
    *\brief Update model parameters (value, 1D raster, and 2D raster, etc.) by Scenario, e.g., areal BMPs.
    * \sa BMPArealStructFactory, and \sa BMPArealStruct
    */
    void UpdateParametersByScenario(int subbsn_id);

    /**** Accessors: Set and Get *****/

    string GetModelName() const { return model_name_; }
    string GetProjectPath() const { return model_path_; }
    string GetFileInFullPath() const { return file_in_file_; }
    string GetFileOutFullPath() const { return file_out_file_; }
    string GetFileCfgFullPath() const { return file_cfg_file_; }
    LayeringMethod GetLayeringMethod() const { return lyr_method_; }
    int GetSubbasinID() const { return subbasin_id_; }
    int GetScenarioID() const { return scenario_id_; }
    int GetCalibrationID() const { return calibration_id_; }
    int GetThreadNumber() const { return thread_num_; }
    bool UseScenario() const { return use_scenario_; }
    string GetOutputSceneName() const { return output_scene_; }
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
    FloatRaster* GetMaskData() { return mask_raster_; }
    map<string, FloatRaster *>& GetRasterDataMap() { return rs_map_; }
    map<string, ParamInfo *>& GetInitParameters() { return init_params_; }
    map<string, float *>& Get1DArrayMap() { return array1d_map_; }
    map<string, int>& Get1DArrayLenMap() { return array1d_len_map_; }
    map<string, float **>& Get2DArrayMap() { return array2d_map_; }
    map<string, int>& Get2DArrayRowsMap() { return array2d_rows_map_; }
    map<string, int>& Get2DArrayColsMap() { return array2d_cols_map_; }
    map<string, ItpWeightData *>& GetItpWeightDataMap() { return weight_data_map_; }
    /*!
    * \brief Get file.in configuration
    */
    virtual bool GetFileInStringVector();
    /*!
    * \brief Get file.out configuration
    */
    virtual bool GetFileOutVector() = 0;
    /*!
    * \brief Get subbasin number and outlet ID
    */
    virtual bool GetSubbasinNumberAndOutletID() = 0;

protected:
    string model_name_;                    ///< Model name, e.g., model_dianbu30m_longterm
    const string model_path_;              ///< Model path
    string file_in_file_;                  ///< file.in full path
    string file_out_file_;                 ///< file.out full path
    string file_cfg_file_;                 ///< config.fig full path
    const LayeringMethod lyr_method_;      ///< Layering method
    const int subbasin_id_;                ///< Subbasin ID
    const int scenario_id_;                ///< Scenario ID
    const int calibration_id_;             ///< Calibration ID
    const int thread_num_;                 ///< Thread number for OpenMP
    bool use_scenario_;                    ///< Model Scenario
    string output_scene_;                  ///< Output scenario identifier, e.g. output1 means scenario 1
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
    FloatRaster* mask_raster_;             ///< Mask data
    map<string, FloatRaster *> rs_map_;    ///< Map of spatial data, both 1D and 2D
    map<string, ParamInfo *> init_params_; ///< Store parameters from Database (PARAMETERS collection)
    map<string, float *> array1d_map_;     ///< 1D array data map, e.g. FLOWOUT_INDEX_D8
    map<string, int> array1d_len_map_;     ///< 1D array data length map
    map<string, float **> array2d_map_;    ///< 2D array data map, e.g. ROUTING_LAYERS
    map<string, int> array2d_rows_map_;    ///< Row number of 2D array data map
    map<string, int> array2d_cols_map_;    ///< Col number of 2D array data map
                                                   ///<   CAUTION that nCols may not same for all rows

    map<string, ItpWeightData *> weight_data_map_; ///< Interpolation weight data map
};

#endif /* SEIMS_DATA_CENTER_H */
