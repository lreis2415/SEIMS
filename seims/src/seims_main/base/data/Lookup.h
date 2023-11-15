#ifndef SEIMS_LOOKUP_H
#define SEIMS_LOOKUP_H

#include <set>

bool IsGlacier(int landuse){
    std::set<int> glacier = {300};
    return glacier.find(landuse) != glacier.end();
}

#endif