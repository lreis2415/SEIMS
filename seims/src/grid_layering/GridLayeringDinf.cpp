#include "GridLayeringDinf.h"

GridLayeringDinf::GridLayeringDinf(int id, MongoGridFS *gfs, const char *out_dir) :
    GridLayering(id, gfs, out_dir),
    m_flowangle(nullptr),
    m_angle(nullptr), m_flowInAngle(nullptr),
    m_flowangle_name(""), m_flowin_angle_name("") {
    string prefix = ValueToString(m_subbasinID);
    m_flowdir_name = prefix + "_FLOW_DIR_DINF";
    m_flowangle_name = prefix + "_FLOW_DIR_ANGLE_DINF";
    m_flowin_index_name = prefix + "_FLOWIN_INDEX_DINF";
    m_flowin_angle_name = prefix + "_FLOWIN_PERCENTAGE_DINF";
    m_flowout_index_name = prefix + "_FLOWOUT_INDEX_DINF";
    m_layering_updown_name = prefix + "_ROUTING_LAYERS_UP_DOWN_DINF";
    m_layering_downup_name = prefix + "_ROUTING_LAYERS_DOWN_UP_DINF";
}

GridLayeringDinf::~GridLayeringDinf() {
    if (nullptr != m_flowangle) delete m_flowangle;
    if (nullptr != m_flowInAngle) delete m_flowInAngle;
}

bool GridLayeringDinf::LoadData() {
    m_flowdir = clsRasterData<int>::Init(m_gfs, m_flowdir_name.c_str(), false);
    if (nullptr == m_flowdir) return false;
    m_nRows = m_flowdir->getRows();
    m_nCols = m_flowdir->getCols();
    m_dirNoData = m_flowdir->getNoDataValue();
    m_flowdirMatrix = m_flowdir->getRasterDataPointer();
    if (m_dirNoData != m_outNoData) m_flowdir->replaceNoData(m_outNoData);

    m_flowangle = clsRasterData<float>::Init(m_gfs, m_flowangle_name.c_str(), false);
    if (nullptr == m_flowangle) return false;
    m_angle = m_flowangle->getRasterDataPointer();
    return true;
}

void GridLayeringDinf::_build_multi_flow_out_angle_array(const int *compressedDir,
                                                         const int *connectCount, float *&pOutput) {
    pOutput[0] = m_nValidCells;
    int counter = 1;
    for (int i = 0; i < m_nRows; ++i) {
        for (int j = 0; j < m_nCols; ++j) {
            int index = i * m_nCols + j;
            if (m_posIndex[index] < 0) continue;
            /// flow in cell's number
            pOutput[counter++] = connectCount[index];
            /// accumulated flow in directions
            int acc_flowin_dir = compressedDir[index];
            /// append the compressed index of flow in cells
            if (acc_flowin_dir & 1) {
                if (j != m_nCols - 1 && compressedDir[index + 1] != m_dirNoData) {
                    pOutput[counter++] = GetPercentage(m_angle[index + 1], 0, 1);;
                }
            }
            if (acc_flowin_dir & 2) {
                if (i != m_nRows - 1 && j != m_nCols - 1 &&
                    compressedDir[index + m_nCols + 1] != m_dirNoData) {
                    pOutput[counter++] = GetPercentage(m_angle[index + m_nCols + 1], 1, 1);
                }
            }
            if (acc_flowin_dir & 4) {
                if (i != m_nRows - 1 && compressedDir[index + m_nCols] != m_dirNoData) {
                    pOutput[counter++] = GetPercentage(m_angle[index + m_nCols], 1, 0);
                }
            }
            if (acc_flowin_dir & 8) {
                if (i != m_nRows - 1 && j != 0 && compressedDir[index + m_nCols - 1] != m_dirNoData) {
                    pOutput[counter++] = GetPercentage(m_angle[index + m_nCols - 1], 1, -1);
                }
            }
            if (acc_flowin_dir & 16) {
                if (j != 0 && compressedDir[index - 1] != m_dirNoData) {
                    pOutput[counter++] = GetPercentage(m_angle[index - 1], 0, -1);
                }
            }
            if (acc_flowin_dir & 32) {
                if (i != 0 && j != 0 && compressedDir[index - m_nCols - 1] != m_dirNoData) {
                    pOutput[counter++] = GetPercentage(m_angle[index - m_nCols - 1], -1, -1);
                }
            }
            if (acc_flowin_dir & 64) {
                if (i != 0 && compressedDir[index - m_nCols] != m_dirNoData) {
                    pOutput[counter++] = GetPercentage(m_angle[index - m_nCols], -1, 0);
                }
            }
            if (acc_flowin_dir & 128) {
                if (i != 0 && j != m_nCols - 1 && compressedDir[index - m_nCols + 1] != m_dirNoData) {
                    pOutput[counter++] = GetPercentage(m_angle[index - m_nCols + 1], -1, 1);
                }
            }
        }
    }
}
bool GridLayeringDinf::OutputFlowIn() {
    GetReverseDirMatrix();
    CountFlowInCells();
    BuildFlowInCellsArray();
    string header = "ID\tUpstreamCount\tUpstreamID";
    int datalength = m_nValidCells + m_flowInTimes + 1;
    bool flag1 = _output_2dimension_array_txt(m_flowin_index_name, header, m_flowInCells) &&
        _output_array_as_gfs(m_flowin_index_name, datalength, m_flowInCells);

    if (nullptr == m_flowInAngle) Initialize1DArray(datalength, m_flowInAngle, 0.f);
    this->_build_multi_flow_out_angle_array(m_flowdirMatrix, m_flowInNum, m_flowInAngle);
    header = "ID\tUpstreamCount\tFlowInPartition";
    bool flag2 = _output_2dimension_array_txt(m_flowin_angle_name, header, m_flowInAngle) &&
        _output_array_as_gfs(m_flowin_angle_name, datalength, m_flowInAngle);
    return flag1 && flag2;
}
bool GridLayeringDinf::OutputFlowOut() {
    CountFlowOutCells();
    BuildFlowOutCellsArray();
    string header = "ID\tDownstreamCount\tDownstreamID";
    int datalength = m_nValidCells + m_flowOutTimes + 1;
    return _output_2dimension_array_txt(m_flowout_index_name, header, m_flowOutCells) &&
        _output_array_as_gfs(m_flowout_index_name, datalength, m_flowOutCells);
}

float GridLayeringDinf::GetPercentage(float angle, int di, int dj) {
    float a = 4.f * angle / PI;
    int n = int(a) % 7;
    float r = a - n;
    switch (n) {
        case 0: { return (di != 0 ? r : 1 - r); }
        case 1: { return (dj == 0 ? r : 1 - r); }
        case 2: { return (dj != 0 ? r : 1 - r); }
        case 3: { return (di == 0 ? r : 1 - r); }
        case 4: { return (di != 0 ? r : 1 - r); }
        case 5: { return (dj == 0 ? r : 1 - r); }
        case 6: { return (dj != 0 ? r : 1 - r); }
        case 7: { return (di == 0 ? r : 1 - r); }
        default: {
            cout << "Invalid angle value: " << angle << endl;
            exit(-1);
        }
    }
}
