/*!
 * \file clsSubbasin.h
 * \brief Class for managing subbasin data
 * \author Junzhi Liu, Liang-Jun Zhu
 * \date May 2017
 */
#ifndef SEIMS_SUBBASIN_CLS_H
#define SEIMS_SUBBASIN_CLS_H

#include "basic.h"
#include "db_mongoc.h"
#include "data_raster.h"

#include "seims.h"

using namespace ccgl;

/*!
 * \class Subbasin
 * \ingroup data
 * \brief Subbasin related parameters and methods.
 *
 * Changelog:
 *   - 1. Remove isOutput, since the output is handled in printInfo class
 *   - 2. Add soil water balance related. 2016-7-28
 */
class Subbasin: Interface {
public:
    /*!
     * \brief Constructor
     * \param [in] id Subbasin ID
     */
    explicit Subbasin(int id);

    //! Destructor
    ~Subbasin();

    //! Check input size
    bool CheckInputSize(int n);

    // Set functions

    //! Set cell index list, as well as subbasin area
    void SetCellList(int n_cells, int* cells);

    //! area of subbasin
    void SetArea(const float area) { area_ = area; }

    //! average slope (%)
    void SetSlope(const float slp) { slope_ = slp; }

    //! Set slope of current subbasin as the average of all cells
    void SetSlope(float* slope);

    //! Set slope correction factor of current subbasin
    void SetSlopeCoefofBasin(const float slope_basin) { slope_coef_ = slope_basin; }

    //! pet
    void SetPet(const float pet) { pet_ = pet; }

    //! Set average percolation (mm)
    void SetPerco(const float perco) { perco_ = perco; };

    //! Set average deep percolation (mm)
    void SetPerde(const float perde) { deep_perco_ = perde; };

    //! groundwater revaporization
    void SetEg(const float eg) { revap_ = eg; }

    //! Set groundwater storage
    void SetGw(const float gw) { gw_ = gw; }

    //! Set groundwater discharge
    void SetQg(const float qg) { qg_ = qg; }

    //! Set groundwater runoff
    void SetRg(const float rg) { rg_ = rg; };

    //! Is revap changed
    void SetIsRevapChanged(const bool isrevap) { revap_changed_ = isrevap; };

    // Get functions

    //! Get subbasin ID
    int GetId() { return subbsn_id_; };

    //! Get valid cells number
    int GetCellCount() { return n_cells_; };

    //! Get index of valid cells
    int* GetCells() { return cells_; };

    //! Get the output flag (true mean output), the function will be deprecated. By LJ
    bool GetIsOutput() { return output_; };

    //! area of subbasin
    float GetArea() { return area_; }

    //! Get the Revap change flat (true mean changed from last time step)
    bool GetIsRevapChanged() { return revap_changed_; };

    //! Get average PET
    float GetPet() { return pet_; };

    //! Get average percolation (mm)
    float GetPerco() { return perco_; };

    //! Get average deep percolation (mm)
    float GetPerde() { return deep_perco_; };

    //! Get average slope (%)
    float GetSlope() { return slope_; };

    //! Get slope coefficient of basin
    float GetSlopeCoef() { return slope_coef_; };

    //! groundwater revaporization
    float GetEg() { return revap_; };

    //! Get groundwater storage
    float GetGw() { return gw_; };

    //! Get groundwater discharge
    float GetQg() { return qg_; };

    //! Get groundwater runoff
    float GetRg() { return rg_; };
private:
    //! Subbasin ID
    int subbsn_id_;
    //! valid cells number
    int n_cells_;
    //! index of valid cells
    int* cells_;
    float cell_area_; ///< area of the cell(s)
                      ///< todo This should be float* when irregular polygon is supported. By lj.
    //! area of current Subbasin
    float area_;

    //! PET
    float pet_;
    //! average percolation (mm) of each valid cells
    float perco_;

    // Subbasin scale parameters' mean value

    // 1. Soil water balance related parameters

    //! precipitation
    float pcp_;
    //! interception loss
    float intercep_;
    //! ET from interception storage
    float intercep_et_;
    //! depression evaporation
    float depression_et_;
    //! infiltration loss
    float infil_;
    //! soil et
    float soil_et_;
    //! total ET
    float total_et_;
    //! net percolation
    float net_perco_;
    //! surface runoff generated (mm)
    float runoff_;
    //! subsurface (interflow) runoff
    float interflow_;
    //! soil moisture (mm)
    float soil_wtr_;
    //! net precipitation
    float net_pcp_;
    //! mean temperature
    float mean_tmp_;
    //! soil temperature
    float soil_tmp_;
    // 2. Groundwater related parameters
    //float m_GW0;

    //! maximum groundwater storage
    float gwmax_;
    //! baseflow recession coefficient
    float kg_;
    //! groundwater revaporization coefficient
    float revap_coef_;
    //! baseflow recession exponent
    float base_ex_;
    //! convert coefficient from mm to m3/s
    float qg_cvt_;
    //! slope correction factor of current subbasin
    float slope_coef_;
    //! average slope of the subbasin
    float slope_;
    //! revaporization from groundwater
    float revap_;
    //! initial groundwater or time (t-1)
    float gw_;
    //! deep percolation
    float deep_perco_;
    //! groundwater discharge (m3/s)
    float qg_;
    //! groundwater runoff (mm)
    float rg_;
    //! Is output defined by file.out or not
    bool output_;
    //! Is the revap (m_EG) is different from last time step
    bool revap_changed_;
};

/*!
 * \class clsSubbasins
 * \ingroup data
 * \brief Manager all Subbasin related parameters and methods.
 */
class clsSubbasins: Interface {
public:
    /*!
     * \brief Constructor
     *
     * Query and constructor basic subbasin's information from MongoDB
     *
     * \param[in] rs_map Map of rasters that have been loaded
     * \param[in] prefix_id subbasin ID as prefix in MongoDB
     */
    clsSubbasins(map<string, FloatRaster *>& rs_map, int prefix_id);
    /*!
     * \brief Check input parameters to ensure the successful constructor
     */
    static clsSubbasins* Init(map<string, FloatRaster *>& rs_map, int prefix_id);
    /// Destructor
    ~clsSubbasins();

    /// Get single reach information by subbasin ID
    Subbasin* GetSubbasinByID(const int id) { return subbasin_objs_.at(id); }

    /// Get subbasin number
    int GetSubbasinNumber() { return n_subbasins_; }

    /// Get subbasin IDs
    vector<int>& GetSubbasinIDs() { return subbasin_ids_; }

    /// Get map of subbasin objects
    map<int, Subbasin *>& GetSubbasinObjects() { return subbasin_objs_; }

    /*!
     * \brief Set slope coefficient for each subbasin according to the basin slope
     * \todo This function will set slope_coef_ to 1.f in MPI version.
     *       Currently, the real slope_coef_ is calculated in `seims_mpi/CalculateProcess.cpp/line 77~`.
     *       In the future, we should think of an elegant way to deal with this issue. By lj. 06/28/18
     */
    void SetSlopeCoefficient(float* rs_slope);

    /*!
     * \brief Get basin (watershed) scale variable (key) value
     * \param [in] key Variable name which is defined in text.h
     */
    float Subbasin2Basin(const string& key);

private:
    /// Subbasins number
    int n_subbasins_;
    /// Subbasin IDs
    vector<int> subbasin_ids_;
    /*!
     * Map container to store all Subbasins information
     * key: Subbasin ID
     * value: Subbasin instance (pointer)
     */
    map<int, Subbasin *> subbasin_objs_;
};
#endif /* SEIMS_SUBBASIN_CLS_H */
