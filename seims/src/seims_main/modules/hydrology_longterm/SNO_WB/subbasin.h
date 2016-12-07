#pragma once

#include <vector>
#include <map>

using namespace std;

class subbasin
{
public:
    subbasin(int);

    ~subbasin(void);

private:
    //subbasin id
    int m_id;

    //all the cells
    vector<int> m_cells;

    float m_P;
    float m_P_net;
    float m_P_blow;
    float m_T;
    float m_Wind;
    float m_SR;
    float m_SE;
    float m_SM;
    float m_SA;

public:
    void addCell(int);

    float getAverage(string key);

    vector<int> *getCells();

    void addP(float);

    void addPnet(float);

    void addPblow(float);

    void addWind(float);

    void addT(float);

    void addSR(float);

    void addSE(float);

    void addSM(float);

    void addSA(float);

    void clear();

    int getId();

public:
    //static map<int,subbasin*>* getSubbasinList(int cellCount, float* subbasinGrid, int subbasinSelectedCount, float* subbasinSelected);
};

