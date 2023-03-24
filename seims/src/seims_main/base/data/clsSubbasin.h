/*!
 * \file clsSubbasin.h
 * \brief Class for managing subbasin data
 * \author Junzhi Liu, Liang-Jun Zhu
 * \date Aug., 2022
 */
#ifndef SEIMS_SUBBASIN_CLS_H
#define SEIMS_SUBBASIN_CLS_H

#include "basic.h"
#include "db_mongoc.h"
#include "data_raster.hpp"

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
    void SetArea(const FLTPT area) { area_ = area; }

    //! average slope (%)
    void SetSlope(const FLTPT slp) { slope_ = slp; }

    //! Set slope of current subbasin as the average of all cells
    void SetSlope(FLTPT* slope);

    //! Set slope correction factor of current subbasin
    void SetSlopeCoefofBasin(const FLTPT slope_basin) { slope_coef_ = slope_basin; }

    //! pet
    void SetPet(const FLTPT pet) { pet_ = pet; }

    //! Set average percolation (mm)
    void SetPerco(const FLTPT perco) { perco_ = perco; }

    //! Set average deep percolation (mm)
    void SetPerde(const FLTPT perde) { deep_perco_ = perde; }

    //! groundwater revaporization
    void SetEg(const FLTPT eg) { revap_ = eg; }

    //! Set groundwater storage
    void SetGw(const FLTPT gw) { gw_ = gw; }

    //! Set groundwater discharge
    void SetQg(const FLTPT qg) { qg_ = qg; }

    //! Set groundwater runoff
    void SetRg(const FLTPT rg) { rg_ = rg; }

    //! Is revap changed
    void SetIsRevapChanged(const bool isrevap) { revap_changed_ = isrevap; }

    // Get functions

    //! Get subbasin ID
    int GetId() { return subbsn_id_; }

    //! Get valid cells number
    int GetCellCount() { return n_cells_; }

    //! Get index of valid cells
    int* GetCells() { return cells_; }

    //! Get the output flag (true mean output), the function will be deprecated. By LJ
    bool GetIsOutput() { return output_; }

    //! area of subbasin
    FLTPT GetArea() { return area_; }

    //! Get the Revap change flat (true mean changed from last time step)
    bool GetIsRevapChanged() { return revap_changed_; }

    //! Get average PET
    FLTPT GetPet() { return pet_; }

    //! Get average percolation (mm)
    FLTPT GetPerco() { return perco_; }

    //! Get average deep percolation (mm)
    FLTPT GetPerde() { return deep_perco_; }

    //! Get average slope (%)
    FLTPT GetSlope() { return slope_; }

    //! Get slope coefficient of basin
    FLTPT GetSlopeCoef() { return slope_coef_; }

    //! groundwater revaporization
    FLTPT GetEg() { return revap_; }

    //! Get groundwater storage
    FLTPT GetGw() { return gw_; }

    //! Get groundwater discharge
    FLTPT GetQg() { return qg_; }

    //! Get groundwater runoff
    FLTPT GetRg() { return rg_; }
private:
    //! Subbasin ID
    int subbsn_id_;
    //! valid cells number
    int n_cells_;
    //! index of valid cells
    int* cells_;
    FLTPT cell_area_; ///< area of the cell(s)
                      ///< todo This should be float* when irregular polygon is supported. By lj.
    //! area of current Subbasin
    FLTPT area_;

    //! PET
    FLTPT pet_;
    //! average percolation (mm) of each valid cells
    FLTPT perco_;

    // Subbasin scale parameters' mean value

    // 1. Soil water balance related parameters

    //! precipitation
    FLTPT pcp_;
    //! interception loss
    FLTPT intercep_;
    //! ET from interception storage
    FLTPT intercep_et_;
    //! depression evaporation
    FLTPT depression_et_;
    //! infiltration loss
    FLTPT infil_;
    //! soil et
    FLTPT soil_et_;
    //! total ET
    FLTPT total_et_;
    //! net percolation
    FLTPT net_perco_;
    //! surface runoff generated (mm)
    FLTPT runoff_;
    //! subsurface (interflow) runoff
    FLTPT interflow_;
    //! soil moisture (mm)
    FLTPT soil_wtr_;
    //! net precipitation
    FLTPT net_pcp_;
    //! mean temperature
    FLTPT mean_tmp_;
    //! soil temperature
    FLTPT soil_tmp_;
    // 2. Groundwater related parameters

    //! maximum groundwater storage
    FLTPT gwmax_;
    //! baseflow recession coefficient
    FLTPT kg_;
    //! groundwater revaporization coefficient
    FLTPT revap_coef_;
    //! baseflow recession exponent
    FLTPT base_ex_;
    //! convert coefficient from mm to m3/s
    FLTPT qg_cvt_;
    //! slope correction factor of current subbasin
    FLTPT slope_coef_;
    //! average slope of the subbasin
    FLTPT slope_;
    //! revaporization from groundwater
    FLTPT revap_;
    //! initial groundwater or time (t-1)
    FLTPT gw_;
    //! deep percolation
    FLTPT deep_perco_;
    //! groundwater discharge (m3/s)
    FLTPT qg_;
    //! groundwater runoff (mm)
    FLTPT rg_;
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
     * \param[in] rs_int_map Map of integer rasters that have been loaded
     * \param[in] prefix_id subbasin ID as prefix in MongoDB
     */
    clsSubbasins(map<string, IntRaster*>& rs_int_map,
                 map<string, FloatRaster *>& rs_map, int prefix_id);
    /*!
     * \brief Check input parameters to ensure the successful constructor
     */
    static clsSubbasins* Init(map<string, IntRaster*>& rs_int_map,
                              map<string, FloatRaster *>& rs_map, int prefix_id);
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
    void SetSlopeCoefficient(FLTPT* rs_slope);

    /*!
     * \brief Get basin (watershed) scale variable (key) value
     * \param [in] key Variable name which is defined in text.h
     */
    FLTPT Subbasin2Basin(const string& key);

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
