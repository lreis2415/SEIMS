#include "basic.h"

#include <ctime>
#include <sstream>
#include <iostream>
#include <fstream>
#ifdef SUPPORT_OMP
#include <omp.h>
#endif /* SUPPORT_OMP */

using std::string;

namespace ccgl {
/// NotCopyable implementation
NotCopyable::NotCopyable() {
}

NotCopyable::NotCopyable(const NotCopyable&) {
}

NotCopyable& NotCopyable::operator=(const NotCopyable&) {
    return *this;
}

/// DefaultClass
Object::~Object() {
}

/// Interface
Interface::~Interface() {
}

ModelException::ModelException(const string& class_name, const string& function_name, const string& msg):
    runtime_error_("Class:" + class_name + "\n" + "Function:" +
                   function_name + "\n" + "Message:" + msg) {
}

string ModelException::ToString() {
    return runtime_error_.what();
}

const char* ModelException::what() const NOEXCEPT {
    return runtime_error_.what();
}

bool IsIpAddress(const char* ip) {
    bool rv = true;
    int tmp1, tmp2, tmp3, tmp4;
    while (true) {
        int i = stringscanf(ip, "%d.%d.%d.%d", &tmp1, &tmp2, &tmp3, &tmp4);
        if (i != 4) {
            rv = false;
            //cout << "IP Address format is not correct!" << endl;
            break;
        }
        if (tmp1 > 255 || tmp2 > 255 || tmp3 > 255 || tmp4 > 255 ||
            tmp1 < 0 || tmp2 < 0 || tmp3 < 0 || tmp4 < 0) {
            rv = false;
            //cout << "IP Address format is not correct!" << endl;
            break;
        }
        for (const char* p_char = ip; *p_char != 0; p_char++) {
            if (*p_char != '.' && (*p_char < '0' || *p_char > '9')) {
                rv = false;
                //cout << "IP Address format is not correct!" << endl;
                break;
            }
        }
        break;
    }
    return rv;
}

void Log(const string& msg, const string& logpath /* = "debugInfo.log" */) {
    struct tm* timeptr = new tm();
    time_t now;
    char buffer[32];
    time(&now);
#ifdef windows
    localtime_s(timeptr, &now);
    asctime_s(buffer, 32, timeptr);
#else
    localtime_r(&now, timeptr);
    asctime_r(timeptr, buffer);
#endif /* windows */
    string timestamp = buffer;
    timestamp = timestamp.substr(0, timestamp.length() - 1);
    std::fstream fs(logpath.c_str(), std::ios::app);
    if (fs.is_open()) {
        fs << timestamp;
        fs << ": ";
        fs << msg;
        fs << std::endl;
        fs.close();
    }
    delete timeptr;
}

int GetAvailableThreadNum() {
#ifdef windows
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#endif /* windows */
#ifdef linux
    return CVT_INT(sysconf(_SC_NPROCESSORS_ONLN));
#endif /* linux */
#ifdef macos
    return CVT_INT(sysconf(_SC_NPROCESSORS_ONLN));
#endif /* macOS X 10.5 and later */
}

void SetDefaultOpenMPThread() {
#ifdef SUPPORT_OMP
    // omp thread have not been set
    if (omp_get_num_threads() <= 1) {
        // set one half of the available threads as default
        omp_set_num_threads(GetAvailableThreadNum() / 2);
    }
#endif /* SUPPORT_OMP */
    /// do nothing if OMP is not supported
}

void SetOpenMPThread(const int n) {
#ifdef SUPPORT_OMP
    omp_set_num_threads(n);
#endif /* SUPPORT_OMP */
    /// do nothing if OMP is not supported
}

void StatusMessage(const char* msg) {
    /// Just for debugging ///
#ifdef _DEBUG
    std::cout << msg << std::endl;
#endif /* DEBUG */
}
} /* namespace: ccgl */
