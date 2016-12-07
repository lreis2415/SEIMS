#include <iostream>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <omp.h>
#include "DiffusiveWave.h"
#include "clsRasterData.h"

using namespace std;

float dx = 30.f;

void SetValue(float *a, int n, float value)
{
    for (int i = 0; i < n; ++i)
        a[i] = value;
}

void Output(float *a, int n, ofstream &ofs)
{
    for (int i = 0; i < n; ++i)
        if (i % 20 == 0)
            ofs << a[i] * 60.f << "\t";
    ofs << endl;
}

void Test(DiffusiveWave &ikw)
{
    for (int i = 0; i < 1000; ++i)
    {
        //cout << i << endl;
        //ikw.SetDate(i+1);
        //if (i == 180)
        //	SetValue(sr, cellsCount, 0.0f);
        ikw.Execute();
        //ikw.Get1DData("CellQs", &nn, &qs);
        //ikw.Get1DData("CellH", &nn, &h);
        //Output(qs, cellsCount, qFile);
        //Output(h, cellsCount, hFile);
    }
}

int main(int argc, const char **argv)
{
    if (argc < 4)
    {
        cout << "Usage: ikwtest RoutingLayerFile dx ThreadCount\n";
        exit(1);
    }

    dx = (float) atof(argv[2]);

    int threadCount = atoi(argv[3]);
    omp_set_num_threads(threadCount);

    // read data from files
    ifstream fLayer(argv[1]);
    string tmp;
    int layerCount;
    fLayer >> tmp >> layerCount;
    float **rtLayers = new float *[layerCount];
    for (int i = 0; i < layerCount; ++i)
    {
        int nCells;
        fLayer >> nCells;
        rtLayers[i] = new float[nCells + 1];
        rtLayers[i][0] = (float) nCells;
        for (int j = 1; j <= nCells; ++j)
            fLayer >> rtLayers[i][j];
    }
    fLayer.close();

    ifstream fFlowIn("RoutingFlowIn.txt");
    int cellsCount;
    fFlowIn >> tmp >> cellsCount;
    float **flowIn = new float *[cellsCount];
    float *dir = new float[cellsCount];
    for (int i = 0; i < cellsCount; ++i)
    {
        int inNum;
        fFlowIn >> dir[i] >> tmp >> inNum;
        flowIn[i] = new float[inNum + 1];
        flowIn[i][0] = (float) inNum;
        for (int j = 1; j <= inNum; ++j)
        {
            fFlowIn >> flowIn[i][j];
        }
    }
    fFlowIn.close();

    clsRasterData slope("slope.txt");
    float *s0 = slope.getRasterDataPointer();
    for (int i = 0; i < cellsCount; ++i)
    {
        if (abs(s0[i]) < 0.01f)
            s0[i] = 0.01f;
    }
    //float *s0  = new float[cellsCount];
    //SetValue(s0, cellsCount, 0.0016f);

    float dt = 10;
    float *mn = new float[cellsCount];


    float *sr = new float[cellsCount];
    //SetValue(mn, cellsCount, 0.025f);
    SetValue(mn, cellsCount, 0.2f);
    SetValue(sr, cellsCount, 5.5556e-6f * dt);

    DiffusiveWave ikw;

    ikw.SetDate(1);

    ikw.SetValue("Theta", 0.65f);
    ikw.SetValue("TimeStep", dt);
    //ikw.SetValue("CellWidth", 9.144f);
    ikw.SetValue("CellWidth", dx);

    ikw.Set1DData("Slope", cellsCount, s0);
    ikw.Set1DData("Manning_n", cellsCount, mn);
    ikw.Set1DData("FlowDirection", cellsCount, dir);
    ikw.Set1DData("D_SURU", cellsCount, sr);

    ikw.Set2DData("FlowInIndex", cellsCount, 1, (float **) flowIn);
    ikw.Set2DData("RoutingLayers", layerCount, 1, (float **) rtLayers);

    //int nn;
    //float *h, *qs;
    ofstream qFile("q.txt");
    ofstream hFile("h.txt");
    clock_t t1, t2;
    t1 = clock();
    Test(ikw);
    //for (int i = 0; i < 1000; ++i)
    //{
    //	//cout << i << endl;
    //	ikw.SetDate(i+1);
    //	if (i == 180)
    //		SetValue(sr, cellsCount, 0.0f);
    //	ikw.ParallelExecute();
    //	ikw.Execute();
    //	ikw.Get1DData("CellQs", &nn, &qs);
    //	ikw.Get1DData("CellH", &nn, &h);
    //	Output(qs, cellsCount, qFile);
    //	Output(h, cellsCount, hFile);
    //}
    t2 = clock();
    cout << t2 - t1 << endl;
    qFile.close();
    hFile.close();

    for (int i = 0; i < layerCount; ++i)
        delete[] rtLayers[i];
    for (int i = 0; i < cellsCount; ++i)
        delete[] flowIn[i];
    delete[] rtLayers;
    delete[] flowIn;
    delete[] dir;
    delete[] mn;
    //delete[] s0;
    delete[] sr;
    //system("pause");
    return 0;
}