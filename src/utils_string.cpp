#include "utils_string.h"

#include <fstream>

namespace ccgl {
namespace utils_string {
string GetUpper(const string& str) {
    string str_tmp1 = CVT_STR(str);
    for (vint j = 0; j < CVT_VINT(str_tmp1.length()); j++) str_tmp1[j] = CVT_CHAR(toupper(str_tmp1[j]));
    return str_tmp1;
}

void TrimSpaces(string& str) {
    // Find the first character position after excluding leading blank spaces
    size_t startpos = str.find_first_not_of(" \t");
    // Find the first character position from reverse
    size_t endpos = str.find_last_not_of(" \t");
    // if all spaces or empty return an empty string
    if (string::npos == startpos || string::npos == endpos) {
        str = "";
    } else {
        str = str.substr(startpos, endpos - startpos + 1);
    }
}

vector<string> SplitString(const string& item, const char delimiter) {
    std::istringstream iss(item);
    vector<string> tokens;
    string field;
    while (getline(iss, field, delimiter)) {
        tokens.emplace_back(field);
    }
    vector<string>(tokens).swap(tokens);
    // tokens.shrink_to_fit(); // C++11, which may not supported by compiler
    return tokens;
}

vector<string> SplitString(const string& item) {
    std::istringstream iss(item);
    vector<string> tokens;
    string field;
    iss >> field;
    while (!iss.eof()) {
        tokens.emplace_back(field);
        iss >> field;
    }
    tokens.emplace_back(field);
    return tokens;
}

bool StringMatch(const char* a, const char* b) {
    return strcasecmp(a, b) == 0;
}

bool StringMatch(const string& text1, const string& text2) {
    // convert to UPPERCASE for comparison
    return GetUpper(text1) == GetUpper(text2);
}

string& Trim(string& s) {
    if (s.empty()) {
        return s;
    }
    s.erase(0, s.find_first_not_of(" \n\r\t"));
    return s.erase(s.find_last_not_of(" \n\r\t") + 1);
}
} /* namespace: utils_string */

} /* namespace: ccgl */
