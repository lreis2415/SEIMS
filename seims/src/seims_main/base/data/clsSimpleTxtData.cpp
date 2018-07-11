#include "clsSimpleTxtData.h"

#include <fstream>

#include "basic.h"
#include "utils_array.h"
#include "utils_string.h"
#include "utils_filesystem.h"

using namespace ccgl;
using namespace utils_array;
using namespace utils_string;
using namespace utils_filesystem;

clsSimpleTxtData::clsSimpleTxtData(const string& filename) : row_(0), data_(nullptr) {
    if (!FileExists(filename)) {
        throw ModelException("clsSimpleTxtData", "ReadFile", "The file " + filename +
                             " does not exist or has not read permission.");
    }
    std::ifstream myfile;
    myfile.open(filename.c_str(), std::ifstream::in);
    string line;
    char* end = nullptr;
    //get number of lines
    if (myfile.is_open()) {
        vector<float> data;
        while (!myfile.eof()) {
            if (myfile.good()) {
                getline(myfile, line);
                TrimSpaces(line);
                if (!line.empty() && line[0] != '#') {
                    // ignore comments and empty lines
                    vector<string> tokens = SplitString(line, '|');
                    if (!tokens.empty()) {
                        TrimSpaces(tokens[0]);
                        if (tokens[0].find_first_of("0123456789") == string::npos) {
                            continue;
                        }
                        data.emplace_back(CVT_FLT(strtod(tokens[0].c_str(), &end))); // add data
                    }
                }
            }
        }
        myfile.close();

        row_ = CVT_INT(data.size());
        if (row_ > 0) {
            data_ = new(nothrow) float[row_];
            int i = 0;
            for (auto it = data.begin(); it < data.end(); ++it) {
                data_[i] = *it;
                i++;
            }
        }
    }
}

clsSimpleTxtData::~clsSimpleTxtData() {
    if (data_ != nullptr) Release1DArray(data_);
}

void clsSimpleTxtData::Dump(std::ostream* fs) {
    if (nullptr == fs) return;
    if (nullptr == data_) return;
    for (int i = 0; i < row_; i++) {
        *fs << data_[i] << endl;
    }
}

void clsSimpleTxtData::GetData(int* n_row, float** data) {
    *n_row = row_;
    *data = data_;
}
