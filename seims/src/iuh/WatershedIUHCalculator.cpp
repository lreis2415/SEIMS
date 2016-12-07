#include "WatershedIUHCalculator.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

WatershedIUHCalculator::WatershedIUHCalculator(string inputfile, string watershedfile,
                                               string stream_networkfile, string t0file, string deltafile,
                                               string runoffcofile,
                                               string uhcellfile, string uhsubfile, string uhwatershedfile)
        : IUHCalculator(inputfile, watershedfile, stream_networkfile, t0file,
                        deltafile), runoffCoFile(runoffcofile),
          uhCellFile(uhcellfile), uhSubFile(uhsubfile), uhWatershedFile(uhwatershedfile)
{
    readData();
}

WatershedIUHCalculator::WatershedIUHCalculator(string inputfile, string watershedfile,
                                               string stream_networkfile, string t0file, string deltafile,
                                               string runoffcofile,
                                               string uhwatershedfile)
        : IUHCalculator(inputfile, watershedfile, stream_networkfile, t0file,
                        deltafile), runoffCoFile(runoffcofile),
          uhWatershedFile(uhwatershedfile)
{
    readData();
}

void WatershedIUHCalculator::readData()
{
    IUHCalculator::readData();
    //initialization
    string strTemp;

    ifstream runoffCoIf(runoffCoFile.c_str());
    runoffCo.resize(nRows);
    for (int i = 0; i < nRows; ++i)
        runoffCo[i].resize(nCols);
    for (int i = 0; i < 7; ++i)
        getline(runoffCoIf, strTemp);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; ++j)
            runoffCoIf >> runoffCo[i][j];
    }
    runoffCoIf.close();

    uhCell.resize(nCells);
    uh1.resize(nCells);
    for (int i = 0; i < nCells; ++i)
    {
        uhCell[i].resize(mt + 1);
        uh1[i].resize(mt + 1);
        for (int j = 0; j <= mt; ++j)
        {
            uhCell[i][j] = 0.0;
            uh1[i][j] = 0.0;
        }
    }

    uhSub.resize(nSubs);
    uh2.resize(nSubs);
    runoffSub.resize(nSubs);
    for (int i = 0; i < nSubs; ++i)
    {
        runoffSub[i] = 0.0;
        uhSub[i].resize(mt + 1);
        uh2[i].resize(mt + 1);
        for (int j = 0; j <= mt; ++j)
        {
            uhSub[i][j] = 0.0;
            uh2[i][j] = 0.0;
        }
    }

    uhWatershed.resize(mt + 1);
    uh3.resize(mt + 1);
    for (int i = 0; i <= mt; ++i)
    {
        uhWatershed[i] = 0.0;
        uh3[i] = 0.0;
    }
}

WatershedIUHCalculator::~WatershedIUHCalculator(void)
{

}


int WatershedIUHCalculator::calCell(void)
{
    ofstream cellOf(uhCellFile.c_str());

    int nc = 0;                       //number of cell
    maxtSub = 0;                  //maximum length of uhSub
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; ++j)
        {
            if (watershed[i][j] <= 0)
                continue;
            //this part is the same as the corresponding part in the RiverIUHCalculator
            //start
            int mint = int(max(0.0, t0[i][j] - 3 * delta[i][j]) + 0.5);    //start time of IUH
            int maxt = min(int(t0[i][j] + 5 * delta[i][j] + 0.5), mt);       //end time
            maxt = max(maxt, 1);
            double sumUh = 0.0;
            for (int m = mint; m <= maxt; ++m)
            {
                double delta0 = max(0.01, delta[i][j]);
                double t00 = max(0.01, t0[i][j]);
                double ti = max(0.01, double(m));
                uhCell[nc][m] = IUHti(delta0, t00, ti);
                //double test = uhCell[nc][m];
                //cout<<"uhCell:"<<nc<<" = " <<test<<"\n";   // test
                sumUh += uhCell[nc][m];
            }

            if (abs(sumUh) < IUHZERO)
            {
                uhCell[nc][0] = 1.0;
                mint = 0;
                maxt = 1;
            }
            else
            {
                for (int m = mint; m <= maxt; ++m)
                {
                    uhCell[nc][m] = uhCell[nc][m] / sumUh;  //make sum of uhCell to 1
                    if (uhCell[nc][m] < 0.001 || uhCell[nc][m] > 1)
                        uhCell[nc][m] = 0.0;
                }
            }
            //define start and end time of uh_cell
            int mint0 = 0;
            for (int m = mint; m <= maxt; ++m)
            {
                if (uhCell[nc][m] > 0.0005)
                {
                    mint0 = m;
                    break;
                }
            }
            mint = mint0;   //!actual start time

            int maxt0 = 0;
            for (int m = mint; m <= maxt; ++m)
            {
                if (uhCell[nc][m] < 0.0005)
                {
                    maxt0 = m - 1;
                    break;
                }
            }
            maxt = max(mint, maxt0);  //!actual end time
            //end

            maxtSub = max(maxtSub, maxt);
            //cell IUH integration
            if (dt >= 1)
            {
                double uhSum = 0.0;
                for (int k = 0; k <= int(maxt / dt); ++k)
                {
                    for (int x = 1; x <= dt; ++x)
                    {
                        uh1[nc][k] += uhCell[nc][k * dt + x - 1];
                    }
                    uhSum += uh1[nc][k];
                }
                mint0 = 0;
                maxt0 = int(maxt / dt);

                for (int k = mint0; k <= maxt0; ++k)
                    uh1[nc][k] /= uhSum;
            }
            else
            {
                for (int m = mint; m <= maxt; ++m)
                    uh1[nc][m] = uhCell[nc][m];
                mint0 = mint;
                maxt0 = maxt;
            }
            cellOf << setw(7) << mint0 << setw(7) << maxt0;
            for (int k = mint0; k <= maxt0; ++k)
                cellOf << setiosflags(ios::fixed) << setw(7) << setprecision(3) << uh1[nc][k];
            cellOf << "\n";

            //summation of uhCell for each subwatershed
            int ns = watershed[i][j] - 1;
            for (int m = mint; m <= maxt; ++m)
                uhSub[ns][m] += uhCell[nc][m] * runoffCo[i][j];
            runoffSub[ns] += runoffCo[i][j];

            nc += 1;
        }
    }
    cellOf.close();
    return 0;
}


int WatershedIUHCalculator::calSub(void)
{
    ofstream subOf(uhSubFile.c_str());
    sumRunCo = 0.0;
    for (int i = 0; i < nSubs; ++i)
    {
        int mint = 0;
        int maxt = 0;
        double uhSum = 0.0;
        for (int j = 0; j <= maxtSub; ++j)
        {
            uhSub[i][j] /= runoffSub[i];
            if (uhSub[i][j] >= 0.001)
                uhSum += uhSub[i][j];
            uhWatershed[j] += uhSub[i][j] * runoffSub[i];
        }

        for (int j = 0; j <= maxtSub; ++j)
        {
            if (uhSub[i][j] > 0.001)
            {
                mint = j;
                break;
            }
        }
        for (int j = mint; j <= maxtSub; ++j)
        {
            uhSub[i][j] /= uhSum;
            if (uhSub[i][j] < 0.001)
            {
                maxt = max(mint + 1, j - 1);
                break;
            }
        }

        //subwatershed IUH integration
        if (dt > 1)
        {
            uhSum = 0.0;
            for (int k = 0; k <= int(maxt / dt); ++k)
            {
                for (int x = 0; x < dt; ++x)
                    uh2[i][k] += uhSub[i][k * dt + x];
                uhSum += uh2[i][k];
            }
            mint = 0;
            maxt = int(maxt / dt);
            for (int k = mint; k <= maxt; ++k)
                uh2[i][k] /= uhSum;
        }
        else
        {
            for (int j = mint; j <= maxt; ++j)
                uh2[i][j] = uhSub[i][j];
        }

        subOf << setw(7) << mint << setw(7) << maxt;
        for (int k = mint; k <= maxt; ++k)
            subOf << setiosflags(ios::fixed) << setw(7) << setprecision(3) << uh2[i][k];
        subOf << "\n";

        sumRunCo += runoffSub[i];
    }
    subOf.close();
    return 0;
}

int WatershedIUHCalculator::calWatershed(void)
{
    ofstream wsOf(uhWatershedFile.c_str());

    int mint = 0;
    int maxt = 0;
    double uhSum = 0.0;
    for (int i = 0; i <= maxtSub; ++i)
    {
        uhWatershed[i] /= sumRunCo;
        if (uhWatershed[i] >= 0.001)
            uhSum += uhWatershed[i];
    }
    for (int i = 0; i <= maxtSub; ++i)
    {
        if (uhWatershed[i] > 0.001)
        {
            mint = i;
            break;
        }
    }
    for (int i = mint; i <= maxtSub; ++i)
    {
        uhWatershed[i] /= uhSum;
        if (uhWatershed[i] < 0.001)
        {
            maxt = max(mint, i - 1);
        }
    }

    //watershed IUH integration
    if (dt > 1)
    {
        uhSum = 0.0;
        for (int k = 0; k <= int(maxt / dt); ++k)
        {
            for (int x = 0; x < dt; ++x)
                uh3[k] += uhWatershed[k * dt + x];
            uhSum += uh3[k];
        }
        mint = 0;
        maxt = int(maxt / dt);
        for (int k = mint; k <= maxt; ++k)
            uh3[k] /= uhSum;
    }
    else
    {
        for (int j = mint; j <= maxt; ++j)
            uh3[j] = uhWatershed[j];
    }

    wsOf << mint << "\t" << maxt << "\t";
    for (int k = mint; k <= maxt; ++k)
        wsOf << uh3[k] << "\t";
    wsOf << "\n";

    wsOf.close();
    return 0;
}

int WatershedIUHCalculator::calIUH(void)
{
    calCell();
    calSub();
    calWatershed();
    return 0;
}

int WatershedIUHCalculator::calIUHSubwatershed(void)
{
    calCell();
    calSub();
    return 0;
}

int WatershedIUHCalculator::calIUHCell(void)
{
    return calCell();
}