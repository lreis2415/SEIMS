#include "clsSimpleTxtData.h"

clsSimpleTxtData::clsSimpleTxtData(string fileName) : m_row(0), m_data(nullptr) {
    if (!FileExists(fileName)) {
        throw ModelException("clsSimpleTxtData", "ReadFile", "The file " + fileName +
            " does not exist or has not read permission.");
    }

    //StatusMessage(("read " + fileName + "...").c_str());

    ifstream myfile;
    myfile.open(fileName.c_str(), ifstream::in);
    string line;

    //get number of lines
    if (myfile.is_open()) {
        vector<float> data;

        while (!myfile.eof()) {
            if (myfile.good()) {
                getline(myfile, line);
                TrimSpaces(line);
                if ((!line.empty()) && (line[0] != '#')) // ignore comments and empty lines
                {
                    vector<string> tokens = SplitString(line, '|');
                    if (!tokens.empty()) {
                        TrimSpaces(tokens[0]);
                        data.push_back(float(atof(tokens[0].c_str())));//add data
                    }
                }
            }
        }
        myfile.close();

        m_row = int(data.size());
        if (m_row > 0) {
            m_data = new float[m_row];
            vector<float>::iterator it;
            int i = 0;
            for (it = data.begin(); it < data.end(); it++) {
                m_data[i] = *it;
                i++;
            }
        }

    }
}

clsSimpleTxtData::~clsSimpleTxtData() {
    Release1DArray(m_data);
}

void clsSimpleTxtData::dump(ostream *fs) {
    if (fs == nullptr) return;
    if (m_data == nullptr) return;
    for (int i = 0; i < m_row; i++) {
        *fs << m_data[i] << endl;
    }
}

void clsSimpleTxtData::getData(int *nRow, float **data) {
    *nRow = m_row;
    *data = m_data;
}
