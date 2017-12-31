#include "GridLayeringD8.h"

GridLayeringD8::GridLayeringD8(int id, MongoGridFS *gfs, const char *out_dir) :
    GridLayering(id, gfs, out_dir) {
    string prefix = ValueToString(m_subbasinID);
    m_flowdir_name = prefix + "_FLOW_DIR";
    m_flowin_index_name = prefix + "_FLOWIN_INDEX_D8";
    m_flowout_index_name = prefix + "_FLOWOUT_INDEX_D8";
    m_layering_updown_name = prefix + "_ROUTING_LAYERS_UP_DOWN";
    m_layering_downup_name = prefix + "_ROUTING_LAYERS_DOWN_UP";
}

bool GridLayeringD8::LoadData() {
    m_flowdir = clsRasterData<int>::Init(m_gfs, m_flowdir_name.c_str(), false);
    if (nullptr == m_flowdir) return false;
    m_nRows = m_flowdir->getRows();
    m_nCols = m_flowdir->getCols();
    m_dirNoData = m_flowdir->getNoDataValue();
    m_flowdirMatrix = m_flowdir->getRasterDataPointer();
    if (m_dirNoData != m_outNoData) m_flowdir->replaceNoData(m_outNoData);
    return true;
}

bool GridLayeringD8::OutputFlowOut() {
    CountFlowOutCells();
    float *pOutput = nullptr;
    Initialize1DArray(m_nValidCells, pOutput, -1.f);
#pragma omp parallel for
    for (int i = 0; i < m_nRows; ++i) {
        for (int j = 0; j < m_nCols; ++j) {
            int index = i * m_nCols + j;
            if (m_flowdirMatrix[index] == m_dirNoData || m_flowdirMatrix[index] < 0) {
                continue;
            }
            int ci = m_posIndex[index];
            int flow_dir = m_flowdirMatrix[index];

            if ((flow_dir & 1) && (j != m_nCols - 1) && (m_flowdirMatrix[index + 1] != m_dirNoData)) {
                pOutput[ci] = m_posIndex[index + 1];
            } else if ((flow_dir & 2) && (i != m_nRows - 1) && (j != m_nCols - 1) &&
                (m_flowdirMatrix[index + m_nCols + 1] != m_dirNoData)) {
                pOutput[ci] = m_posIndex[index + m_nCols + 1];
            } else if ((flow_dir & 4) && (i != m_nRows - 1) && (m_flowdirMatrix[index + m_nCols] != m_dirNoData)) {
                pOutput[ci] = m_posIndex[index + m_nCols];
            } else if ((flow_dir & 8) && (i != m_nRows - 1) && (j != 0) &&
                (m_flowdirMatrix[index + m_nCols - 1] != m_dirNoData)) {
                pOutput[ci] = m_posIndex[index + m_nCols - 1];
            } else if ((flow_dir & 16) && (j != 0) && (m_flowdirMatrix[index - 1] != m_dirNoData)) {
                pOutput[ci] = m_posIndex[index - 1];
            } else if ((flow_dir & 32) && (i != 0) && (j != 0)
                && (m_flowdirMatrix[index - m_nCols - 1] != m_dirNoData)) {
                pOutput[ci] = m_posIndex[index - m_nCols - 1];
            } else if ((flow_dir & 64) && (i != 0) && (m_flowdirMatrix[index - m_nCols] != m_dirNoData)) {
                pOutput[ci] = m_posIndex[index - m_nCols];
            } else if ((flow_dir & 128) && (i != 0) && (j != m_nCols - 1)
                && (m_flowdirMatrix[index - m_nCols + 1] != m_dirNoData)) {
                pOutput[ci] = m_posIndex[index - m_nCols + 1];
            } else { pOutput[ci] = -1; }
        }
    }
    // output to txt
    string outpath = string(m_outputDir) + "/" + m_flowout_index_name + ".txt";
    ofstream ofs(outpath.c_str());
    ofs << "ID\tDownstreamID" << endl;
    for (int i = 0; i < m_nValidCells; i++) {
        ofs << i << "\t" << pOutput[i] << endl;
    }
    ofs.close();

    bool flag = _output_array_as_gfs(m_flowout_index_name, m_nValidCells, pOutput);
    Release1DArray(pOutput);
    return flag;
}
