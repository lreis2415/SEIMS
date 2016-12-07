#include "IUHCalculator.h"
#include <fstream>
#include <cmath>

using namespace std;

IUHCalculator::IUHCalculator(string inputfile, string watershedfile,
                             string stream_networkfile, string t0file, string deltafile)
        : inputFile(inputfile), watershedFile(watershedfile),
          strnetworkFile(stream_networkfile), t0File(t0file), deltaFile(deltafile),
          mt(200), nRows(0), nCols(0), dt(24), nCells(0), nSubs(0), nLinks(0)
{
}


void IUHCalculator::readData()
{
    string strTemp;

    ////ifstream inputIf(inputFile.c_str());    // file.in
    ////getline(inputIf, strTemp);
    ////inputIf >> dt;
    ////inputIf.close();

    ifstream watershedIf(watershedFile.c_str());  // subbasin.asc
    watershedIf >> strTemp >> nCols;
    watershedIf >> strTemp >> nRows;
    for (int i = 0; i < 6; ++i)
        getline(watershedIf, strTemp);

    ifstream strnetworkIf(strnetworkFile.c_str());
    ifstream t0If(t0File.c_str());
    ifstream deltaIf(deltaFile.c_str());
    //ifstream runoffCoIf(runoffCoFile.c_str());

    for (int i = 0; i < 7; ++i)
    {
        getline(strnetworkIf, strTemp);
        getline(t0If, strTemp);
        getline(deltaIf, strTemp);
        //getline(runoffCoIf,strTemp);
    }

    watershed.resize(nRows);
    strnetwork.resize(nRows);
    link.resize(nRows);
    t0.resize(nRows);
    delta.resize(nRows);
    //runoffCo.resize(nRows);

    for (int i = 0; i < nRows; ++i)
    {
        watershed[i].resize(nCols);
        strnetwork[i].resize(nCols);
        link[i].resize(nCols);
        t0[i].resize(nCols);
        delta[i].resize(nCols);
        //runoffCo[i].resize(nCols);
    }

    //read the watershed and link data
    nCells = 0;
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; ++j)
        {
            watershedIf >> watershed[i][j];
            strnetworkIf >> strnetwork[i][j];
            if (watershed[i][j] <= 0 || strnetwork[i][j] <= 0)
            {
                link[i][j] = -99;
            }
            //linkIf >> link[i][j];
            t0If >> t0[i][j];
            deltaIf >> delta[i][j];
            //runoffCoIf >> runoffCo[i][j];

            if (watershed[i][j] > 0)
            {
                nCells += 1;
                nSubs = max(nSubs, watershed[i][j]);
                if (strnetwork[i][j] > 0)
                {
                    link[i][j] = watershed[i][j] * strnetwork[i][j];
                    nLinks += 1;
                }
            }
        }
    }

    watershedIf.close();
    strnetworkIf.close();
    t0If.close();
    deltaIf.close();
    //runoffCoIf.close();
}

IUHCalculator::~IUHCalculator(void)
{
}

double IUHCalculator::IUHti(double delta0, double t00, double ti)
{
    double tmp1 = 1 / (delta0 * sqrt(2 * 3.1416 * pow(ti, 3.0) / pow(t00, 3.0)));
    double tmp2 = pow(ti - t00, 2.0) / (2.0 * pow(delta0, 2.0) * ti / t00);
    return tmp1 * exp(-tmp2);
}

