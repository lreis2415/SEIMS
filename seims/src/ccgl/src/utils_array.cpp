#include "utils_array.h"

#include <sstream>
#include <fstream>

namespace ccgl {
namespace utils_array {
void Output1DArrayToTxtFile(const int n, const float* data, const char* filename) {
    std::ofstream ofs(filename);
    for (int i = 0; i < n; i++) {
        ofs << data[i] << "\n";
    }
    ofs.close();
}

void Output2DArrayToTxtFile(const int rows, const int cols, const float** data, const char* filename) {
    std::ofstream ofs(filename);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            ofs << data[i][j] << "\t";
        }
        ofs << "\n";
    }
    ofs.close();
}

template <typename T>
void Read1DArrayFromTxtFile(const char* filename, int& rows, T*& data) {
    std::ifstream ifs(filename);
    string tmp;
    ifs >> tmp >> rows;
    data = new T[rows];
    for (int i = 0; i < rows; i++) {
        ifs >> data[i];
    }
    ifs.close();
}

template <typename T>
void Read2DArrayFromTxtFile(const char* filename, int& rows, T**& data) {
    std::ifstream ifs(filename);
    string tmp;
    ifs >> tmp >> rows;
    data = new T*[rows];
    int n;
    for (int i = 0; i < rows; i++) {
        ifs >> n;
        data[i] = new T[n + 1];
        data[i][0] = n;
        for (int j = 1; j <= n; j++) {
            ifs >> data[i][j];
        }
    }
    ifs.close();
}

template <typename T>
void Read2DArrayFromString(const char* s, int& rows, T**& data) {
    std::istringstream ifs(s);
    string tmp;
    ifs >> tmp >> rows;
    data = new T*[rows];
    int n;
    for (int i = 0; i < rows; i++) {
        ifs >> n;
        data[i] = new T[n + 1];
        data[i][0] = n;
        for (int j = 1; j <= n; j++) {
            ifs >> data[i][j];
        }
    }
}
} /* namespace: utils_array */

} /* namespace: ccgl */
