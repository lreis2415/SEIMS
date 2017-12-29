#include "GridLayering.h"

GridLayering::GridLayering(int id, MongoGridFS *gfs, const char *out_dir) :
    m_subbasinID(id), m_gfs(gfs), m_outputDir(out_dir),
    m_flowdir(nullptr),  // clsRasterData<int>
    m_nRows(-1), m_nCols(-1), m_dirNoData(-9999), m_outNoData(-9999), m_nValidCells(-1),
    m_posIndex(nullptr), m_flowdirMatrix(nullptr), m_reverseDir(nullptr),
    m_flowInNum(nullptr), m_flowInTimes(0), m_flowInCells(nullptr),
    m_flowOutNum(nullptr), m_flowOutTimes(0), m_flowOutCells(nullptr),
    m_layers_updown(nullptr), m_layers_downup(nullptr),
    m_flowdir_name(""), m_flowin_index_name(""), m_flowout_index_name(""),
    m_layering_updown_name(""), m_layering_downup_name("") {

}

GridLayering::~GridLayering() {
    if (nullptr != m_flowdir) delete m_flowdir;  // m_flowdirMatrix will be released too.
    if (nullptr != m_posIndex) Release1DArray(m_posIndex);
    if (nullptr != m_reverseDir) Release1DArray(m_reverseDir);
    if (nullptr != m_flowInNum) Release1DArray(m_flowInNum);
    if (nullptr != m_flowInCells) Release1DArray(m_flowInCells);
    if (nullptr != m_flowOutNum) Release1DArray(m_flowOutNum);
    if (nullptr != m_flowOutCells) Release1DArray(m_flowOutCells);
    if (nullptr != m_layers_updown) Release1DArray(m_layers_updown);
    if (nullptr != m_layers_downup) Release1DArray(m_layers_downup);
}

bool GridLayering::Execute() {
    if (!LoadData()) return false;
    CalPositionIndex();
    return OutputFlowOut() && OutputFlowIn() && GridLayeringFromSource() && GridLayeringFromOutlet();
}

void GridLayering::CalPositionIndex() {
    if (m_nValidCells > 0 && nullptr != m_posIndex) return;
    Initialize1DArray(m_nRows * m_nCols, m_posIndex, -1);
    m_nValidCells = 0;
    for (int i = 0; i < m_nRows * m_nCols; ++i) {
        if (m_flowdirMatrix[i] != m_dirNoData) {
            m_posIndex[i] = m_nValidCells++;
        }
    }
}

void GridLayering::GetReverseDirMatrix() {
    if (nullptr == m_reverseDir) Initialize1DArray(m_nRows * m_nCols, m_reverseDir, 0);
#pragma omp parallel for
    for (int i = 0; i < m_nRows; i++) {
        for (int j = 0; j < m_nCols; j++) {
            int index = i * m_nCols + j;
            int flow_dir = m_flowdirMatrix[index];
            if (flow_dir == m_dirNoData || flow_dir < 0) {
                m_reverseDir[index] = m_dirNoData;
                continue;
            }

            if ((flow_dir & 1) && j != m_nCols - 1 && m_flowdirMatrix[index + 1] != m_dirNoData
                && m_flowdirMatrix[index + 1] > 0) {
                m_reverseDir[index + 1] += 16;
            }

            if ((flow_dir & 2) && i != m_nRows - 1 && j != m_nCols - 1 &&
                m_flowdirMatrix[index + m_nCols + 1] != m_dirNoData &&
                m_flowdirMatrix[index + m_nCols + 1] > 0) {
                m_reverseDir[index + m_nCols + 1] += 32;
            }

            if ((flow_dir & 4) && i != m_nRows - 1 && m_flowdirMatrix[index + m_nCols] != m_dirNoData &&
                m_flowdirMatrix[index + m_nCols] > 0) {
                m_reverseDir[index + m_nCols] += 64;
            }

            if ((flow_dir & 8) && i != m_nRows - 1 && j != 0 && m_flowdirMatrix[index + m_nCols - 1] != m_dirNoData &&
                m_flowdirMatrix[index + m_nCols - 1] > 0) {
                m_reverseDir[index + m_nCols - 1] += 128;
            }

            if ((flow_dir & 16) && j != 0 && m_flowdirMatrix[index - 1] != m_dirNoData
                && m_flowdirMatrix[index - 1] > 0) {
                m_reverseDir[index - 1] += 1;
            }

            if ((flow_dir & 32) && i != 0 && j != 0 && m_flowdirMatrix[index - m_nCols - 1] != m_dirNoData &&
                m_flowdirMatrix[index - m_nCols - 1] > 0) {
                m_reverseDir[index - m_nCols - 1] += 2;
            }

            if ((flow_dir & 64) && i != 0 && m_flowdirMatrix[index - m_nCols] != m_dirNoData
                && m_flowdirMatrix[index - m_nCols] > 0) {
                m_reverseDir[index - m_nCols] += 4;
            }

            if ((flow_dir & 128) && i != 0 && j != m_nCols - 1 && m_flowdirMatrix[index - m_nCols + 1] != m_dirNoData &&
                m_flowdirMatrix[index - m_nCols + 1] > 0) {
                m_reverseDir[index - m_nCols + 1] += 8;
            }
        }
    }
}

void GridLayering::CountFlowInCells() {
    int n = m_nRows * m_nCols;
    if (nullptr == m_flowInNum) Initialize1DArray(n, m_flowInNum, m_dirNoData);
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int flow_dir = m_reverseDir[i];
        if (flow_dir == m_dirNoData || flow_dir < 0) { continue; }

        m_flowInNum[i] = 0;
        if (flow_dir & 1) { m_flowInNum[i]++; }
        if (flow_dir & 2) { m_flowInNum[i]++; }
        if (flow_dir & 4) { m_flowInNum[i]++; }
        if (flow_dir & 8) { m_flowInNum[i]++; }
        if (flow_dir & 16) { m_flowInNum[i]++; }
        if (flow_dir & 32) { m_flowInNum[i]++; }
        if (flow_dir & 64) { m_flowInNum[i]++; }
        if (flow_dir & 128) { m_flowInNum[i]++; }
    }
    int total = 0;
#pragma omp parallel for reduction(+:total)
    for (int index = 0; index < n; index++) {
        if (m_flowInNum[index] <= 0 || m_flowInNum[index] == m_dirNoData) { continue; }
        total += m_flowInNum[index];
    }
    m_flowInTimes = total;
}

void GridLayering::_build_multi_flow_out_array(const int *compressedDir,
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
                    pOutput[counter++] = m_posIndex[index + 1];
                }
            }
            if (acc_flowin_dir & 2) {
                if (i != m_nRows - 1 && j != m_nCols - 1 &&
                    compressedDir[index + m_nCols + 1] != m_dirNoData) {
                    pOutput[counter++] = m_posIndex[index + m_nCols + 1];
                }
            }
            if (acc_flowin_dir & 4) {
                if (i != m_nRows - 1 && compressedDir[index + m_nCols] != m_dirNoData) {
                    pOutput[counter++] = m_posIndex[index + m_nCols];
                }
            }
            if (acc_flowin_dir & 8) {
                if (i != m_nRows - 1 && j != 0 && compressedDir[index + m_nCols - 1] != m_dirNoData) {
                    pOutput[counter++] = m_posIndex[index + m_nCols - 1];
                }
            }
            if (acc_flowin_dir & 16) {
                if (j != 0 && compressedDir[index - 1] != m_dirNoData) {
                    pOutput[counter++] = m_posIndex[index - 1];
                }
            }
            if (acc_flowin_dir & 32) {
                if (i != 0 && j != 0 && compressedDir[index - m_nCols - 1] != m_dirNoData) {
                    pOutput[counter++] = m_posIndex[index - m_nCols - 1];
                }
            }
            if (acc_flowin_dir & 64) {
                if (i != 0 && compressedDir[index - m_nCols] != m_dirNoData) {
                    pOutput[counter++] = m_posIndex[index - m_nCols];
                }
            }
            if (acc_flowin_dir & 128) {
                if (i != 0 && j != m_nCols - 1 && compressedDir[index - m_nCols + 1] != m_dirNoData) {
                    pOutput[counter++] = m_posIndex[index - m_nCols + 1];
                }
            }
        }
    }
}
void GridLayering::BuildFlowInCellsArray() {
    int nOutput = m_flowInTimes + m_nValidCells + 1;
    if (nullptr == m_flowInCells) Initialize1DArray(nOutput, m_flowInCells, 0.f);
    this->_build_multi_flow_out_array(m_reverseDir, m_flowInNum, m_flowInCells);
}

void GridLayering::CountFlowOutCells() {
    int n = m_nRows * m_nCols;
    if (nullptr == m_flowOutNum) Initialize1DArray(n, m_flowOutNum, m_dirNoData);
#pragma omp parallel for
    for (int index = 0; index < n; index++) {
        if (m_posIndex[index] < 0) continue;
        int i = index / m_nCols;
        int j = index % m_nCols;
        m_flowOutNum[index] = 0;
        int flow_dir = m_flowdirMatrix[index];

        if ((flow_dir & 1) && (j != m_nCols - 1) && (m_flowdirMatrix[index + 1] != m_dirNoData)) {
            m_flowOutNum[index]++;
        }
        if ((flow_dir & 2) && (i != m_nRows - 1) && (j != m_nCols - 1) &&
            (m_flowdirMatrix[index + m_nCols + 1] != m_dirNoData)) {
            m_flowOutNum[index]++;
        }
        if ((flow_dir & 4) && (i != m_nRows - 1) &&
            (m_flowdirMatrix[index + m_nCols] != m_dirNoData)) {
            m_flowOutNum[index]++;
        }
        if ((flow_dir & 8) && (i != m_nRows - 1) && (j != 0) &&
            (m_flowdirMatrix[index + m_nCols - 1] != m_dirNoData)) {
            m_flowOutNum[index]++;
        }
        if ((flow_dir & 16) && (j != 0) &&
            (m_flowdirMatrix[index - 1] != m_dirNoData)) {
            m_flowOutNum[index]++;
        }
        if ((flow_dir & 32) && (i != 0) && (j != 0) &&
            (m_flowdirMatrix[index - m_nCols - 1] != m_dirNoData)) {
            m_flowOutNum[index]++;
        }
        if ((flow_dir & 64) && (i != 0) &&
            (m_flowdirMatrix[index - m_nCols] != m_dirNoData)) {
            m_flowOutNum[index]++;
        }
        if ((flow_dir & 128) && (i != 0) && (j != m_nCols - 1) &&
            (m_flowdirMatrix[index - m_nCols + 1] != m_dirNoData)) {
            m_flowOutNum[index]++;
        }
    }
    int total = 0;
#pragma omp parallel for reduction(+:total)
    for (int index = 0; index < n; index++) {
        if (m_flowOutNum[index] <= 0 || m_flowOutNum[index] == m_dirNoData) { continue; }
        total += m_flowOutNum[index];
    }
    m_flowOutTimes = total;
}

void GridLayering::BuildFlowOutCellsArray() {
    int nOutput = m_flowOutTimes + m_nValidCells + 1;
    if (nullptr == m_flowOutCells) Initialize1DArray(nOutput, m_flowOutCells, 0.f);
    this->_build_multi_flow_out_array(m_flowdirMatrix, m_flowOutNum, m_flowOutCells);
}

template<typename T>
bool GridLayering::_output_2dimension_array(string name,
                                            int length,
                                            string header,
                                            const T *matrix) {
    string outpath = string(m_outputDir) + "/" + name + ".txt";
    ofstream ofs(outpath.c_str());
    ofs << matrix[0] << endl;
    ofs << header << endl;
    int tmpCount = 1;
    for (int i = 0; i < (int) matrix[0]; i++) {
        int count = (int) matrix[tmpCount++];
        ofs << i << "\t" << count << "\t";
        for (int j = 0; j < count; j++) {
            ofs << matrix[tmpCount++] << ", ";
        }
        ofs << endl;
    }
    ofs.close();

    bool flag = false;
    int max_loop = 3;
    int cur_loop = 1;
    while (cur_loop < max_loop) {
        if (!OutputToMongoDB(name.c_str(), length, (char *) matrix)) {
            cur_loop++;
        } else {
            cout << "Output " << name << " done!" << endl;
            flag = true;
            break;
        }
    }
    return flag;
}
bool GridLayering::OutputFlowIn() {
    GetReverseDirMatrix();
    CountFlowInCells();
    BuildFlowInCellsArray();
    string header = "ID\tUpstreamCount\tUpstreamID";
    int datalength = m_nValidCells + m_flowInTimes + 1;
    return _output_2dimension_array(m_flowin_index_name, datalength, header, m_flowInCells);
}

void GridLayering::UpdateDownStream(int row, int col, int &numNextLayer, int *nextLayer) {
    int index = row * m_nCols + col;
    int dir = m_flowdirMatrix[index];

    if ((dir & 1) && (col != m_nCols - 1)) {
        if ((--m_flowInNum[index + 1]) == 0) {
            nextLayer[numNextLayer++] = index + 1;
        }
    }

    if ((dir & 2) && row != m_nRows - 1 && col != m_nCols - 1) {
        if ((--m_flowInNum[index + m_nCols + 1]) == 0) {
            nextLayer[numNextLayer++] = index + m_nCols + 1;
        }
    }

    if ((dir & 4) && row != m_nRows - 1) {
        if ((--m_flowInNum[index + m_nCols]) == 0) {
            nextLayer[numNextLayer++] = index + m_nCols;
        }
    }

    if ((dir & 8) && row != m_nRows - 1 && col != 0) {
        if ((--m_flowInNum[index + m_nCols - 1]) == 0) {
            nextLayer[numNextLayer++] = index + m_nCols - 1;
        }
    }

    if ((dir & 16) && col != 0) {
        if ((--m_flowInNum[index - 1]) == 0) {
            nextLayer[numNextLayer++] = index - 1;
        }
    }

    if ((dir & 32) && row != 0 && col != 0) {
        if ((--m_flowInNum[index - m_nCols - 1]) == 0) {
            nextLayer[numNextLayer++] = index - m_nCols - 1;
        }
    }

    if ((dir & 64) && row != 0) {
        if ((--m_flowInNum[index - m_nCols]) == 0) {
            nextLayer[numNextLayer++] = index - m_nCols;
        }
    }

    if ((dir & 128) && row != 0 && col != m_nCols - 1) {
        if ((--m_flowInNum[index - m_nCols + 1]) == 0) {
            nextLayer[numNextLayer++] = index - m_nCols + 1;
        }
    }
}

void GridLayering::UpdateUpStream(int row, int col, int &numNextLayer, int *nextLayer) {
    int index = row * m_nCols + col + 1;
    if (col != m_nCols - 1 && m_flowdirMatrix[index] & 16) {
        if ((--m_flowOutNum[index]) == 0) {
            nextLayer[numNextLayer++] = index;
        }
    }
    index = (row + 1) * m_nCols + col + 1;
    if (row != m_nRows - 1 && col != m_nCols - 1 && m_flowdirMatrix[index] & 32) {
        if ((--m_flowOutNum[index]) == 0) {
            nextLayer[numNextLayer++] = index;
        }
    }
    index = (row + 1) * m_nCols + col;
    if (row != m_nRows - 1 && m_flowdirMatrix[index] & 64) {
        if ((--m_flowOutNum[index]) == 0) {
            nextLayer[numNextLayer++] = index;
        }
    }
    index = (row + 1) * m_nCols + col - 1;
    if (row != m_nRows - 1 && col != 0 && m_flowdirMatrix[index] & 128) {
        if ((--m_flowOutNum[index]) == 0) {
            nextLayer[numNextLayer++] = index;
        }
    }
    index = row * m_nCols + col - 1;
    if (col != 0 && m_flowdirMatrix[index] & 1) {
        if ((--m_flowOutNum[index]) == 0) {
            nextLayer[numNextLayer++] = index;
        }
    }
    index = (row - 1) * m_nCols + col - 1;
    if (row != 0 && col != 0 && m_flowdirMatrix[index] & 2) {
        if ((--m_flowOutNum[index]) == 0) {
            nextLayer[numNextLayer++] = index;
        }
    }
    index = (row - 1) * m_nCols + col;
    if (row != 0 && m_flowdirMatrix[index] & 4) {
        if ((--m_flowOutNum[index]) == 0) {
            nextLayer[numNextLayer++] = index;
        }
    }
    index = (row - 1) * m_nCols + col + 1;
    if (row != 0 && col != m_nCols - 1 && m_flowdirMatrix[index] & 8) {
        if ((--m_flowOutNum[index]) == 0) {
            nextLayer[numNextLayer++] = index;
        }
    }
}

bool GridLayering::GridLayeringFromSource() {
    int n = m_nRows * m_nCols;
    Initialize1DArray(n, m_layers_updown, m_outNoData);

    int *lastLayer = nullptr;
    Initialize1DArray(n, lastLayer, m_outNoData);
    int numLastLayer = 0; // the number of cells of last layer

    int index = 0;
    // 1. find source grids
    for (int i = 0; i < m_nRows; i++) {
        for (int j = 0; j < m_nCols; j++) {
            index = i * m_nCols + j;
            if (m_flowdirMatrix[index] == m_dirNoData || m_flowdirMatrix[index] < 0) {
                continue;
            } else {
                if (m_flowInNum[index] == 0) {
                    lastLayer[numLastLayer++] = index;
                }
            }
        }
    }
    // 2. loop and layer
    int numNextLayer = 0;
    int curNum = 0; //the layering number of the current layer. In the end, it is the total number of layers.
    int *nextLayer = nullptr;
    Initialize1DArray(n, nextLayer, m_outNoData);
    int *tmp = nullptr;
    ostringstream oss;
    while (numLastLayer > 0) {
        oss << "\n" << numLastLayer << "\t";
        curNum++;

        numNextLayer = 0;
        for (int iInLayer = 0; iInLayer < numLastLayer; iInLayer++) {
            index = lastLayer[iInLayer];
            oss << m_posIndex[index] << "\t";

            m_layers_updown[index] = curNum;

            int i = index / m_nCols;
            int j = index % m_nCols;

            UpdateDownStream(i, j, numNextLayer, nextLayer);
        }

        numLastLayer = numNextLayer;

        tmp = lastLayer;
        lastLayer = nextLayer;
        nextLayer = tmp;
    }

    Release1DArray(lastLayer);
    Release1DArray(nextLayer);

    ostringstream outputOss;
    outputOss << m_nValidCells << "\t" << curNum << oss.str();

    return _output_grid_layering(m_layering_updown_name, m_layers_updown, outputOss.str());
}

bool GridLayering::GridLayeringFromOutlet() {
    int n = m_nRows * m_nCols;
    Initialize1DArray(n, m_layers_downup, m_outNoData);

    int *lastLayer = nullptr;
    Initialize1DArray(n, lastLayer, m_outNoData);
    int numLastLayer = 0;

    bool flag = true;
    int curNum = 0;

    // 1. find outlet grids
#pragma omp parallel for
    for (int i = 0; i < m_nRows; i++) {
        for (int j = 0; j < m_nCols; j++) {
            int index = i * m_nCols + j;
            if (m_flowdirMatrix[index] != m_dirNoData && m_flowOutNum[index] == 0) {
                lastLayer[numLastLayer++] = index;
            }
        }
    }

    string strLayers = "";
    //cout << "nubmer of outlets: " << numLastLayer << endl;

    // 2. loop and layer
    int numNextLayer = 0;
    int *nextLayer = nullptr;
    Initialize1DArray(n, nextLayer, m_outNoData);
    int *tmp;

    while (numLastLayer > 0) {
        curNum++;

        ostringstream oss;
        oss << "\n" << numLastLayer << "\t";

        numNextLayer = 0;
        for (int iInLayer = 0; iInLayer < numLastLayer; iInLayer++) {
            int index = lastLayer[iInLayer];
            oss << m_posIndex[index] << "\t";

            m_layers_downup[index] = curNum;

            int i = index / m_nCols;
            int j = index % m_nCols;

            UpdateUpStream(i, j, numNextLayer, nextLayer);
        }
        strLayers = oss.str() + strLayers;

        numLastLayer = numNextLayer;

        tmp = lastLayer;
        lastLayer = nextLayer;
        nextLayer = tmp;
    }

    ostringstream outputOss;
    outputOss << m_nValidCells << "\t" << curNum << strLayers;
    // 3. reverse the layer number
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (m_layers_downup[i] != m_outNoData) {
            m_layers_downup[i] = curNum - m_layers_downup[i] + 1;
        }
    }
    Release1DArray(lastLayer);
    Release1DArray(nextLayer);

    return _output_grid_layering(m_layering_downup_name, m_layers_downup, outputOss.str());
}

bool GridLayering::OutputToMongoDB(const char *name, int number, char *s) {
    bson_t p = BSON_INITIALIZER;
    BSON_APPEND_INT32(&p, "SUBBASIN", m_subbasinID);
    BSON_APPEND_UTF8(&p, "TYPE", name);
    BSON_APPEND_UTF8(&p, "ID", name);
    BSON_APPEND_UTF8(&p, "DESCRIPTION", name);
    BSON_APPEND_DOUBLE(&p, "NUMBER", number);

    m_gfs->removeFile(string(name));
    size_t n = number * sizeof(float);
    m_gfs->writeStreamData(string(name), s, n, &p);
    bson_destroy(&p);
    if (NULL == m_gfs->getFile(name)) { return false; }
    else { return true; }
}

bool GridLayering::_output_grid_layering(string &name, int *layer_grid, string output_str) {
    string outpath = string(m_outputDir) + "/" + name + ".tif";
    clsRasterData<int>(m_flowdir, layer_grid).outputFileByGDAL(outpath);
    outpath = string(m_outputDir) + "/" + name + ".txt";
    ofstream ofs(outpath.c_str());
    ofs << output_str;
    ofs.close();

    bool flag = false;
    int max_loop = 3;
    int cur_loop = 1;
    while (cur_loop < max_loop) {
        if (!OutputToMongoDB(name.c_str(), m_nRows * m_nCols, (char *) layer_grid)) {
            cur_loop++;
        } else {
            cout << "Output Grid Layering done: " << name << endl;
            flag = true;
            break;
        }
    }
    return flag;
}
