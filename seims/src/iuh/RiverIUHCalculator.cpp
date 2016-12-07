#include "RiverIUHCalculator.h"
#include <cmath>
#include <fstream>

using namespace std;

RiverIUHCalculator::RiverIUHCalculator(string inputfile, string watershedfile,
                                       string stream_networkfile, string t0file, string deltafile,
                                       string uhfile)
        : IUHCalculator(inputfile, watershedfile, stream_networkfile, t0file, deltafile),
          uhFile(uhfile)
{
    readData();
}

RiverIUHCalculator::~RiverIUHCalculator(void)
{
}

void RiverIUHCalculator::readData()
{
    IUHCalculator::readData();

    linkCellNum.resize(nSubs);
    for (int i = 0; i < nSubs; ++i)
        linkCellNum[i] = 0;

    uhCell.resize(nLinks);
    for (int i = 0; i < nLinks; ++i)
    {
        uhCell[i].resize(mt + 1);
        for (int j = 0; j <= mt; ++j)
            uhCell[i][j] = 0.0;
    }

    uhRiver.resize(nSubs);
    uhSub.resize(nSubs);
    for (int i = 0; i < nSubs; ++i)
    {
        uhRiver[i].resize(mt + 1);
        uhSub[i].resize(mt + 1);
        for (int j = 0; j <= mt; ++j)
        {
            uhRiver[i][j] = 0.0;
            uhSub[i][j] = 0.0;
        }
    }

    t0_new.resize(nRows);
    t0_h_network.resize(nRows);
    temp_t0.resize(nRows);

    delta_new.resize(nRows);
    delta_h_network.resize(nRows);
    temp_d.resize(nRows);
    for (int i = 0; i < nRows; i++)
    {
        t0_new[i].resize(nCols);
        t0_h_network[i].resize(nCols);
        temp_t0[i].resize(nCols);

        delta_new[i].resize(nCols);
        delta_h_network[i].resize(nCols);
        temp_d[i].resize(nCols);
    }
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nCols; j++)
        {
            if (strnetwork[i][j] < 0)
            {
                t0_h_network[i][j] = -99;
                delta_h_network[i][j] = -99;
            }
            else
            {
                t0_h_network[i][j] = t0[i][j] * strnetwork[i][j];
                delta_h_network[i][j] = delta[i][j] * strnetwork[i][j];
            }
        }
    }

}

int RiverIUHCalculator::get_New_T0andDelta()
{
    int subi = 0;
    vector<double> min_t0_rc, min_d_rc;   // t0_reach_outlet = Min(t0_reach_cell), delta_reach_outlet = Min(delta_reach_cell)
    min_t0_rc.resize(nSubs);
    min_d_rc.resize(nSubs);
    for (int i = 0; i < nSubs; i++)
    {
        min_t0_rc[i] = 99999999.0; // set a big enough initial value for compare
        min_d_rc[i] = 99999999.0;
    }
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nCols; j++)
        {
            subi = link[i][j];
            if (subi < 0)
            {
                continue;
            }
            if (t0_h_network[i][j] < min_t0_rc[subi - 1])
            {
                min_t0_rc[subi - 1] = t0_h_network[i][j];
            }
            if (delta_h_network[i][j] < min_d_rc[subi - 1])
            {
                min_d_rc[subi - 1] = delta_h_network[i][j];
            }
        }
    }
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nCols; j++)
        {
            subi = link[i][j];
            if (subi < 0)
            {
                t0_new[i][j] = -99;
                delta_new[i][j] = -99;
            }
            else
            {
                t0_new[i][j] = t0_h_network[i][j] - min_t0_rc[subi - 1];
                delta_new[i][j] = delta_h_network[i][j] - min_d_rc[subi - 1];
            }
        }
    }
    return 0;
}

int RiverIUHCalculator::calIUH(void)
{
    get_New_T0andDelta();

    ofstream subOf(uhFile.c_str());
    //calculate IUH for each river cell
    int nc = 0;
    int maxtRiver = 0;
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; ++j)
        {
            if (link[i][j] < 0)
                continue;
            //this part is the same as the corresponding part in the WatershedIUHCalculator
            //start
            int mint = int(max(0.0, t0_new[i][j] - 3 * delta_new[i][j]) + 0.5);    //start time of IUH
            int maxt = min(int(t0_new[i][j] + 5 * delta_new[i][j] + 0.5), mt);       //end time
            maxt = max(maxt, 1);
            double sumUh = 0.0;
            for (int m = mint; m <= maxt; ++m)
            {
                double delta0 = max(0.01, delta_new[i][j]);
                double t00 = max(0.01, t0_new[i][j]);
                double ti = max(0.01, double(m));
                uhCell[nc][m] = IUHti(delta0, t00, ti);
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

            int mint0 = 0;
            for (int m = mint; m <= maxt; ++m)
            {
                if (uhCell[nc][m] > 0.0005)
                {
                    mint0 = m;
                    break;
                }
            }
            mint = mint0;

            int maxt0 = 0;
            for (int m = mint; m <= maxt; ++m)
            {
                if (uhCell[nc][m] < 0.0005)
                {
                    maxt0 = m - 1;
                    break;
                }
            }
            maxt = max(mint, maxt0);
            //end
            maxtRiver = max(maxtRiver, maxt);

            //summation of uhCell for each streamlink
            int ns = link[i][j];
            for (int m = mint; m <= maxt; ++m)
            {
                uhRiver[ns - 1][m] += uhCell[nc][m];
            }
            linkCellNum[ns - 1] += 1;

            nc += 1;
        }
    }

    int mint, maxt;
    //calculate IUH for each streamlink
    for (int i = 0; i < nSubs; ++i)
    {
        double uhSum = 0.0;
        for (int j = 0; j <= maxtRiver; ++j)
        {
            uhRiver[i][j] /= linkCellNum[i];
            if (uhRiver[i][j] > 0.001)
                uhSum += uhRiver[i][j];
        }
        for (int j = 0; j <= maxtRiver; ++j)
        {
            if (uhRiver[i][j] > 0.001)
            {
                mint = j;
                break;
            }
        }
        for (int j = mint; j <= maxtRiver; ++j)
        {
            uhRiver[i][j] /= uhSum;
            if (uhRiver[i][j] < 0.001)
            {
                maxt = max(mint + 1, j - 1);
                break;
            }
        }

        //river IUH integration
        if (dt > 1)
        {
            double uhSum = 0.0;
            for (int k = 0; k <= int(maxt / dt); ++k)
            {
                for (int x = 0; x < dt; ++x)
                    uhSub[i][k] += uhRiver[i][k * dt + x];
                uhSum += uhSub[i][k];
            }
            mint = 0;
            maxt = int(maxt / dt);
            for (int k = 0; k <= maxt; ++k)
                uhSub[i][k] /= uhSum;
        }
        else
        {
            for (int j = mint; j <= maxt; ++j)
                uhSub[i][j] = uhRiver[i][j];
        }

        subOf << mint << "\t" << maxt << "\t";
        for (int k = mint; k <= maxt; ++k)
            subOf << uhSub[i][k] << "\t";
        subOf << "\n";
    }

    subOf.close();
    return 0;
}
