#ifndef FIELD_PARTITION_CELL_ORDERING
#define FIELD_PARTITION_CELL_ORDERING

#include "Cell.h"
#include "Field.h"
#include "FieldPartition.h"
#include "clsRasterData.h"

#include <map>
#include <vector>

// Build by Wu Hui, 2012.4.28
// objective: to build the relationships of the each field, and to aggregate very small upstream fields
//  into their downstream fields. This is controlled by the threshold given by user. 
//
using namespace std;

class CellOrdering {
public:
    CellOrdering(IntRaster *rsDir, IntRaster *rsLandU, IntRaster *rsMask,
                 FlowDirectionMethod flowDirMtd, int threshold);

    ~CellOrdering();

public:
    bool Execute(int iOutlet, int jOutlet);

    void BuildTree();

    /// build routing layers from outlet
    void BuildRoutingLayer(int idOutlet, int layerNum);

    // added by wuhui
    bool ExcuteFieldsDis(int iOutlet, int jOutlet);

    void BuildFieldsTree(int iOutlet, int jOutlet);

    // degree is the degree of the tree
    void BuildField(int id, Field *pfield);

    void OutputFieldMap(const char *filename);

    void OutputFieldRelationship(const char *filename);

private:
    void ReMergeSameLanduseField(int fieldID, int degree);

    // merge the fields which are neighbor with the same land use with same father (down-stream-filed ID)
    void MergeSameFatherSameLanduseField(int rootID);
    // Is cell's neighbor
    bool IsCellsNeighbor(int id1, int id2);

    bool IsFieldsNeighbor(int fid1, int fid2);

    /*!
     * \brief Merge flow-in fields of the same landuse
     *
     * \param[in] pfield current field instance
     * \note This is the original algorithm by Hui Wu. If the function is invoked from root field and the whole data is huge,
     *           the recursive may end with infinite loop. So, it should be invoked from uppermost field without a field flow in.
     * \sa MergeSameLanduseChildFieldsFromUpDown()
     * \deprecated Bad efficiency
     */
    void MergeSameLanduseChildFields(Field *pfield);

    /*!
     * \brief  Merge flow-in fields of the same landuse in one upstream layer
     *
     * \param[in] pfield current field instance
     * \sa MergeSameLanduseChildFields()
     */
    void MergeSameLanduseChildFieldsOneLayer(Field *pfield);

    /*!
     * \brief Merge flow-in fields of the same landuse from upstream to downstream
     * \note Added by Liang-Jun Zhu, 2016-6-20
     */
    void MergeSameLanduseChildFieldsFromUpDown();

    //bool IsFieldsNeighbor(Field* f1, Field* f2);
    void AggregateSmallField(Field *pfield);

    void getpostorderfield(Field *pfield);

    void mergefieldsofsamefather(Field *f1, Field *f2);   // merge f1 to f2

    void mergefieldschild2father(Field *child, Field *father);

    void remergesamelandusefield(Field *pfield);

    void reclassfieldid(Field *pfield, int degree);   //reclassify field and get degree

    void sortReclassedfieldid();
public:
    /* index off set of the following order
    5 6 7
    4   0
    3 2 1
    */
    const static int m_d1[8];
    const static int m_d2[8];

    static int cfid;
    static int FID;

private:
    IntRaster *m_dir;
    IntRaster *m_mask;
    IntRaster *m_landu;
    IntRaster *m_streamlink;
    FlowDirectionMethod m_flowDirMtd;
    // threshold is the number of cells which defined by user for minimal size of field
    int m_threshold;
    /// used to store the flow in and out relationship of cells
    std::vector<Cell *> m_cells;

    std::vector<Field *> m_fields;

    /// 2d vector used to store the hierarchy information
    std::vector<std::vector<int>> m_layers;

    int m_nRows, m_nCols, m_size;
    int m_validCellsCount;
    float m_cellwidth;

    int m_FieldNum;
    int m_maxDegree;

    int m_rootID;
    /* convert direction code to index number
    For FlowDirectionMethod is ArcGIS,
    32 64 128
    64     1
    8   4  2
    For FlowDirectionMethod is TauDEM,
    4  3  2
    5     1
    6  7  8
    */
    std::map<int, int> m_dirToIndexMap;

    std::map<int, int> m_relassFID;    // int oldid, int newid

    map<int, Field *> m_mapfields;

    map<int, vector<Field *> > m_mapSameDegreefields;

    vector<int> m_posterorderfieldId;

    map<int, int> m_newoldfidmap;
};

#endif /* FIELD_PARTITION_CELL_ORDERING */