#include <iostream>

#include "StormGreenAmpt.h"
#include <ctime>

using namespace std;

#define N 1


void InitValue(float *a, int n, float value)
{
    for (int i = 0; i < n; ++i)
        a[i] = value;
}

int main()
{
    int threadCount = 10;

    StormGreenAmpt ga;
    ga.SetDate(100);
    ga.SetValue("TimeStep", 1.0f);
    ga.SetValue("T_snow", 0.0f);
    ga.SetValue("t_soil", 0.0f);
    ga.SetValue("T0", 1.0f);

    float *ks = new float[N];//mm/h
    InitValue(ks, N, 13.2f);
    ga.Set1DData("Conductivity", N, ks);

    float *clay = new float[N];
    InitValue(clay, N, 0.10f);
    ga.Set1DData("Clay", N, clay);

    float *sand = new float[N];
    InitValue(sand, N, 0.70f);
    ga.Set1DData("Sand", N, sand);

    float *porosity = new float[N];
    InitValue(porosity, N, 0.437f);
    ga.Set1DData("porosity", N, porosity);

    float *wp = new float[N];
    InitValue(wp, N, 10.0f);
    ga.Set1DData("Wiltingpoint", N, wp);

    float *initMst = new float[N];
    InitValue(initMst, N, 0.1f);
    ga.Set1DData("InitialSoilMoisture", N, initMst);

    float *pNet = new float[N];
    InitValue(pNet, N, 0.6f);
    ga.Set1DData("D_NEPR", N, pNet);

    float *tMin = new float[N];
    InitValue(tMin, N, 10.0f);
    ga.Set1DData("D_Tmin", N, tMin);

    float *tMax = new float[N];
    InitValue(tMax, N, 25.0f);
    ga.Set1DData("D_Tmax", N, tMax);

    float *soilMoisture = new float[N];
    InitValue(soilMoisture, N, 0.1f);
    ga.Set1DData("D_SOMO", N, soilMoisture);

    float *sd = new float[N];
    InitValue(sd, N, 0.0f);
    ga.Set1DData("D_DPST", N, sd);

    float *tSoil = new float[N];
    InitValue(tSoil, N, 20.0f);
    ga.Set1DData("D_SOTE", N, tSoil);

    float *sa = new float[N];
    InitValue(sa, N, 0.0f);
    ga.Set1DData("D_SNAC", N, sa);

    float *sm = new float[N];
    InitValue(sm, N, 0.0f);
    ga.Set1DData("D_SNME", N, sm);

    clock_t t1, t2;
    t1 = clock();

    //#pragma omp parallel for
    for (int i = 0; i < 200; i++)
    {
        ga.Execute();
        float *p;
        int n;
        ga.Get1DData("Infil", &n, &p);
        //cout << p[0] << "\n";
    }
    cout << endl;
    t2 = clock();

    cout << t2 - t1 << endl;


    return 0;
}
