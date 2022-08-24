/*!
 * \file clsSimpleTxtData.h
 * \brief A simple text read class
 * \author Junzhi Liu, Liangjun Zhu
 * \version 1.1
 * \date Aug., 2022
 */
#ifndef SEIMS_SIMPLE_TEXT_H
#define SEIMS_SIMPLE_TEXT_H
#include <fstream>

#include "utils_array.h"
#include "utils_string.h"
#include "utils_filesystem.h"
#include "basic.h"
#include <seims.h>

using namespace ccgl;
using namespace utils_array;
using namespace utils_string;
using namespace utils_filesystem;

/*!
 * \ingroup data
 * \class clsSimpleTxtData
 * \brief read string line from text file
 *
 */
template <typename T>
class clsSimpleTxtData: Interface {
public:
    //! Constructor, from text file read lines data
    explicit clsSimpleTxtData(const string& filename);

    //! Destructor
    ~clsSimpleTxtData();

    //! Get line number and data
    void GetData(int* n_row, T** data);

    //! Output lines data to \a ostream
    void Dump(std::ostream* fs);

private:
    //! line number
    int row_;
    //! lines data
    T* data_;
};

template <typename T>
clsSimpleTxtData<T>::clsSimpleTxtData(const string& filename) : row_(0), data_(nullptr) {
    if (!FileExists(filename)) {
        throw ModelException("clsSimpleTxtData", "ReadFile", "The file " + filename +
                             " does not exist or has not read permission.");
    }
    std::ifstream myfile;
    myfile.open(filename.c_str(), std::ifstream::in);
    char* end = nullptr;
    //get number of lines
    if (myfile.is_open()) {
        string line;
        vector<T> data;
        while (!myfile.eof()) {
            if (!myfile.good()) {
                continue;
            }
            getline(myfile, line);
            TrimSpaces(line);
            if (line.empty() || line[0] == '#') {
                continue; // ignore comments and empty lines
            }
            vector<string> tokens = SplitString(line, '|');
            if (!tokens.empty()) {
                TrimSpaces(tokens[0]);
                if (tokens[0].find_first_of("0123456789") == string::npos) {
                    continue;
                }
                data.emplace_back(T(strtod(tokens[0].c_str(), &end))); // add data
            }
        }
        myfile.close();

        row_ = CVT_INT(data.size());
        if (row_ > 0) {
            data_ = new(nothrow) T[row_];
            int i = 0;
            for (auto it = data.begin(); it < data.end(); ++it) {
                data_[i] = *it;
                i++;
            }
        }
    }
}

template <typename T>
clsSimpleTxtData<T>::~clsSimpleTxtData() {
    if (data_ != nullptr) { Release1DArray(data_); }
}

template <typename T>
void clsSimpleTxtData<T>::Dump(std::ostream* fs) {
    if (nullptr == fs) return;
    if (nullptr == data_) return;
    for (int i = 0; i < row_; i++) {
        *fs << data_[i] << endl;
    }
}

template <typename T>
void clsSimpleTxtData<T>::GetData(int* n_row, T** data) {
    *n_row = row_;
    *data = data_;
}

#endif /* SEIMS_SIMPLE_TEXT_H */
