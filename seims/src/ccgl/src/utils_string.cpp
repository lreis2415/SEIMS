#include "utils_string.h"

#include <fstream>
#if defined(CPP_GCC) || defined(CPP_ICC)
#include <stdio.h>
#include <ctype.h>
#include <wctype.h>
#define _strtoi64 strtoll
#define _strtoui64 strtoull
#define _wcstoi64 wcstoll
#define _wcstoui64 wcstoull
#endif

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

string itoa(vint number) {
    char buffer[100];
    ITOA_S(number, buffer, sizeof(buffer) / sizeof(*buffer), 10);
    return buffer;
}

wstring itow(vint number) {
    wchar_t buffer[100];
    ITOW_S(number, buffer, sizeof(buffer) / sizeof(*buffer), 10);
    return buffer;
}

string i64toa(vint64_t number) {
    char buffer[100];
    I64TOA_S(number, buffer, sizeof(buffer) / sizeof(*buffer), 10);
    return buffer;
}

wstring i64tow(vint64_t number) {
    wchar_t buffer[100];
    I64TOW_S(number, buffer, sizeof(buffer) / sizeof(*buffer), 10);
    return buffer;
}

string utoa(vuint number) {
    char buffer[100];
    UITOA_S(number, buffer, sizeof(buffer) / sizeof(*buffer), 10);
    return buffer;
}

wstring utow(vuint number) {
    wchar_t buffer[100];
    UITOW_S(number, buffer, sizeof(buffer) / sizeof(*buffer), 10);
    return buffer;
}

string u64toa(vuint64_t number) {
    char buffer[100];
    UI64TOA_S(number, buffer, sizeof(buffer) / sizeof(*buffer), 10);
    return buffer;
}

wstring u64tow(vuint64_t number) {
    wchar_t buffer[100];
    UI64TOW_S(number, buffer, sizeof(buffer) / sizeof(*buffer), 10);
    return buffer;
}

string ftoa(double number) {
    char buffer[320];
    _gcvt_s(buffer, 320, number, 30);
    vint len = vint(strlen(buffer));
    if (buffer[len - 1] == '.')
    {
        buffer[len - 1] = '\0';
    }
    return buffer;
}

wstring ftow(double number) {
    return atow(ftoa(number));
}

vint _wtoa(const wchar_t* w, char* a, vint chars) {
#if defined CPP_MSVC
    return WideCharToMultiByte(CP_THREAD_ACP, 0, w, -1, a, int(a ? chars : 0), 0, 0);
#elif defined(CPP_GCC) || defined(CPP_ICC)
    return wcstombs(a, w, chars - 1) + 1;
#endif
}

string wtoa(const wstring& wstr) {
    vint len = _wtoa(wstr.c_str(), 0, 0);
    char* buffer = new char[len];
    memset(buffer, 0, len*sizeof(*buffer));
    _wtoa(wstr.c_str(), buffer, int(len));
    string s = buffer;
    delete[] buffer;
    return s;
}

vint _atow(const char* a, wchar_t* w, vint chars) {
#if defined CPP_MSVC
    return MultiByteToWideChar(CP_THREAD_ACP, 0, a, -1, w, int(w ? chars : 0));
#elif defined CPP_GCC
    return mbstowcs(w, a, chars - 1) + 1;
#endif
}

wstring atow(const string& astr) {
    vint len = _atow(astr.c_str(), 0, 0);
    wchar_t* buffer = new wchar_t[len];
    memset(buffer, 0, len*sizeof(*buffer));
    _atow(astr.c_str(), buffer, int(len));
    wstring s = buffer;
    delete[] buffer;
    return s;
}

vint IsInt(const string& num_str, bool& success) {
    char* endptr = nullptr;
    int result = strtol(num_str.c_str(), &endptr, 10);
    success = endptr == num_str.c_str() + num_str.length() && itoa(result) == num_str;
    return result;
}

vint IsInt(const wstring& num_str, bool& success) {
    wchar_t* endptr = nullptr;
    int result = wcstol(num_str.c_str(), &endptr, 10);
    success = endptr == num_str.c_str() + num_str.length() && itow(result) == num_str;
    return result;
}

vint64_t IsInt64(const string& num_str, bool& success) {
    char* endptr = 0;
    vint64_t result = _strtoi64(num_str.c_str(), &endptr, 10);
    success = endptr == num_str.c_str() + num_str.length() && i64toa(result) == num_str;
    return result;
}

vint64_t IsInt64(const wstring& num_str, bool& success) {
    wchar_t* endptr = 0;
    vint64_t result = _wcstoi64(num_str.c_str(), &endptr, 10);
    success = endptr == num_str.c_str() + num_str.length() && i64tow(result) == num_str;
    return result;
}

vuint IsUInt(const string& num_str, bool& success) {
    char* endptr = 0;
    vuint result = strtoul(num_str.c_str(), &endptr, 10);
    success = endptr == num_str.c_str() + num_str.length() && utoa(result) == num_str;
    return result;
}

vuint IsUInt(const wstring& num_str, bool& success) {
    wchar_t* endptr = 0;
    vint64_t result = wcstoul(num_str.c_str(), &endptr, 10);
    success = endptr == num_str.c_str() + num_str.length() && utow(result) == num_str;
    return result;
}

vuint64_t IsUInt64(const string& num_str, bool& success) {
    char* endptr = 0;
    vuint64_t result = _strtoui64(num_str.c_str(), &endptr, 10);
    success = endptr == num_str.c_str() + num_str.length() && u64toa(result) == num_str;
    return result;
}

vuint64_t IsUInt64(const wstring& num_str, bool& success) {
    wchar_t* endptr = 0;
    vuint64_t result = _wcstoui64(num_str.c_str(), &endptr, 10);
    success = endptr == num_str.c_str() + num_str.length() && u64tow(result) == num_str;
    return result;
}

double IsDouble(const string& num_str, bool& success) {
    char* endptr = 0;
    double result = strtod(num_str.c_str(), &endptr);
    success = endptr == num_str.c_str() + num_str.length();
    return result;
}

double IsDouble(const wstring& num_str, bool& success) {
    wchar_t* endptr = 0;
    double result = wcstod(num_str.c_str(), &endptr);
    success = endptr == num_str.c_str() + num_str.length();
    return result;
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
