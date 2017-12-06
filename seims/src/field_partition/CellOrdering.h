#pragma once

#include <map>
#include <vector>
#include "clsRasterData.h"
#include "Cell.h"
#include "Field.h"
#include "fieldTnode.h"
#include "FieldPartition.h"
// Build by Wu Hui, 2012.4.28
// objective: to build the relationships of the each field, and to aggregate very small upstream fields
//  into their downstream fields. This is controlled by the threshold given by user. 
//
using namespace std;

#define IntRaster   clsRasterData<int>
#define FloatRaster clsRasterData<float>

class CellOrdering {
public:
    CellOrdering(IntRaster *rsDir, IntRaster *rsMask, FlowDirectionMethod flowDirMtd);

    CellOrdering(IntRaster *rsDir, IntRaster *rsLandU, IntRaster *rsMask, FlowDirectionMethod flowDirMtd,
                 int threshold);

    CellOrdering(IntRaster *rsDir, IntRaster *rsLandU, IntRaster *rsStreamLink, IntRaster *rsMask,
                 FlowDirectionMethod flowDirMtd, int threshold);

    // threshold is the number of cells which defined by user for minimal size of field
    ~CellOrdering(void);

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
    int m_threshold;
    /// used to store the flow in and out relationship of cells
    std::vector<Cell *> m_cells;

    std::vector<Field *> m_fields;

    /// index in the compressed array without cells of nodata
    std::vector<int> m_compressedIndex;

    /// 2d vector used to store the hierarchy information
    std::vector <std::vector<int>> m_layers;

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

public:
    bool Execute(int iOutlet, int jOutlet);

    void BuildTree(void);

    /// build routing layers from outlet
    void BuildRoutingLayer(int idOutlet, int layerNum);

    void OutRoutingLayer(const char *filename);

    void CalCompressedIndex(void);

    void OutputCompressedLayer(const char *filename);

    void OutputCompressedFlowOut(const char *filename);

    void OutputCompressedFlowIn(const char *filename);

    // Output to MongoDB is not needed here. By Liangjun
    //void OutputLayersToMongoDB(int id, gridfs* gfs);
    //void OutputLayersToMongoDB2(int id, gridfs* gfs);
    //void OutputFlowOutToMongoDB(int id, gridfs* gfs);
    //void OutputFlowInToMongoDB(int id, gridfs* gfs);
    //int WriteStringToMongoDB(gridfs *gfs, int id, const char* type, int cellNumber, string s);

    /// build routing layers from most up cells
    bool Execute2(int iOutlet, int jOutlet);

    void BuildRoutingLayer2(int idOutlet);

    void OutputCompressedLayer2(const char *filename);

    // added by wuhui
    bool ExcuteFieldsDis(int iOutlet, int jOutlet);

    void BuildFieldsTree(int iOutlet, int jOutlet);

    void BuildField(int id, Field *pfield);  //degree is the degree of the tree
    void OutputFieldMap(const char *filename);

    void OutputFieldRelationship(const char *filename);

    //revised by wuhui
    //void DestoryFieldsTree(fieldTnode * proot);

private:
    void MergeSameDegreeLanduseFields();

    void AggregateSmallField();

    void ReMergeSameLanduseField(int fieldID, int degree);

    void MergeSameFatherSameLanduseField(
        int rootID);  // merge the fields which are neighbor with the same land use with same father (down-stream-filed ID)
    // Is cell's neighbor
    bool IsCellsNeighbor(int id1, int id2);

    bool IsFieldsNeighbor(int fid1, int fid2);

    void reclassFieldID();

    std::map<int, int> m_relassFID;    // int oldid, int newid
    std::vector <vector<int>> m_sameDegreeFID;
    std::map<int, std::vector<int> > m_degreeFIDs;

    /* //build tree struct
    struct Tnode {
    Tnode* Tparent;
    vector<Tnode*> Tchildren;
    int degree;
    vector<Field*> childFieldsVec;
    Field* field;
    } fieldTnode;

    struct fieldTnode* m_root;*/
    //fieldTnode* m_root;

    //void buildfield(int ioutlet, int joutlet);

    map<int, Field *> m_mapfields;
    map<int, vector<Field *> > m_mapSameDegreefields;

    vector<int> m_posterorderfieldId;

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

    map<int, int> m_newoldfidmap;
};
