#include "stats.h"
// using namespace std; // Avoid to using the entire namespace of std. Comment by Liangjun, 01/23/19

static float dokern(float x, int kern)
{
    if (kern == 1) return (1.f);
    if (kern == 2) return (exp(-0.5f * x * x));
    return (0.f);
}

void BDRksmooth(float *x, float *y, int n, float *xp, float *yp, int np, int kern, float bw)
{
    int imin = 0;
    float cutoff = 0.f, num, den, x0, w;

    /* bandwidth is in units of half inter-quartile range. */
    if (kern == 1)
    {
        bw *= 0.5f;
        cutoff = bw;
    }
    if (kern == 2)
    {
        bw *= 0.3706506f;
        cutoff = 4.f * bw;
    }
    while (x[imin] < xp[0] - cutoff && imin < n) imin++;
    for (int j = 0; j < np; j++)
    {
        num = 0.f;
		den = 0.f;
        x0 = xp[j];
        for (int i = imin; i < n; i++)
        {
            if (x[i] < x0 - cutoff) imin = i;
            else
            {
                if (x[i] > x0 + cutoff) break;
                w = dokern(fabs(x[i] - x0) / bw, kern);
                num += w * y[i];
                den += w;
            }
        }
        if (den > ZERO) yp[j] = num / den;
		else yp[j] = MISSINGFLOAT;
    }
}

void BDRksmooth(vector<float> &x, vector<float> &y, vector<float> &xp, vector<float> &yp, int kern, float bw)
{/// ksmooth function in R
    int imin = 0;
    int n = x.size();
    int np = xp.size();
    float cutoff = 0.f, num, den, x0, w;
    /* bandwidth is in units of half inter-quartile range. */
    if (kern == 1)
    {
        bw *= 0.5f;
        cutoff = bw;
    }
    if (kern == 2)
    {
        bw *= 0.3706506f;
        cutoff = 4.f * bw;
    }
    while (x[imin] < xp[0] - cutoff && imin < n) imin++;
    for (int j = 0; j < np; j++)
    {
        num = 0.f;
		den = 0.f;
        x0 = xp[j];
        for (int i = imin; i < n; i++)
        {
            if (x[i] < x0 - cutoff) imin = i;
            else
            {
                if (x[i] > x0 + cutoff) break;
                w = dokern(fabs(x[i] - x0) / bw, kern);
                num += w * y[i];
                den += w;
            }
        }
        if (den > ZERO) yp.push_back(num / den);
		else yp.push_back(MISSINGFLOAT);
    }
}

void findTurnPoints(float *x, int n, priority_queue<int> &pks, priority_queue<int> &vlys)
{
    float *newx = new float[n + 2];
    newx[0] = MISSINGFLOAT;
    newx[n + 1] = MISSINGFLOAT;
    int i = 0, j = 0;
    for (i = 0; i < n; i++)
        newx[i + 1] = x[i];
    float **matrix = new float *[n];
    bool *pksIdx = new bool[n];
    bool *vlysIdx = new bool[n];
    int tempIdx;
    for (i = 0; i < n; i++)
    {
        matrix[i] = new float[3];
        for (j = 0; j < 3; j++)
        {
            matrix[i][j] = newx[i + j];
            //cout<<matrix[i][j]<<",";
        }
        //cout<<endl;
    }
    for (i = 0; i < n; i++)
    {
        tempIdx = 0;
        for (j = 1; j < 3; j++)
            if (matrix[i][j] > matrix[i][j - 1])
                tempIdx = j;
        if (tempIdx == 1)
            pksIdx[i] = true;
        else
            pksIdx[i] = false;
        //cout<<pksIdx[i]<<endl;
    }
    newx[0] = -1.f * MISSINGFLOAT;
    newx[n + 1] = -1.f * MISSINGFLOAT;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < 3; j++)
        {
            matrix[i][j] = -1.f * newx[i + j];
            //cout<<matrix[i][j]<<",";
        }
        //cout<<endl;
    }
    for (i = 0; i < n; i++)
    {
        tempIdx = 0;
        for (j = 1; j < 3; j++)
            if (matrix[i][j] >= matrix[i][j - 1])
                tempIdx = j;
        if (tempIdx == 1)
            vlysIdx[i] = true;
        else
            vlysIdx[i] = false;
        //cout<<vlysIdx[i]<<endl;
    }
    priority_queue<int> pksindex;
    priority_queue<int> vlysindex;
    for (i = 0; i < n; i++)
    {
        if (pksIdx[i] & !vlysIdx[i])
        {
            pksIdx[i] = true;
            pksindex.push(i);
        }
        else if (!pksIdx[i] & vlysIdx[i])
        {
            vlysIdx[i] = true;
            vlysindex.push(i);
        }
        else
        {
            pksIdx[i] = false;
            vlysIdx[i] = false;
        }
    }

    int pksnum = pksindex.size();
    int vlysnum = vlysindex.size();
    int *pksi = new int[pksnum];
    int *vlysi = new int[vlysnum];
    i = pksnum;
    while (!pksindex.empty())
    {
        pksi[i - 1] = pksindex.top();
        pksindex.pop();
        pks.push(pksi[i - 1]);
        //cout<<pksi[i-1]<<endl;
        i--;
    }
    i = vlysnum;
    while (!vlysindex.empty())
    {
        vlysi[i - 1] = vlysindex.top();
        vlysindex.pop();
        vlys.push(vlysi[i - 1]);
        //cout<<vlysi[i-1]<<endl;
        i--;
    }
    if (pksi[0] != 0)
    {
        vlys.push(0);
        //cout<<0<<endl;
    }
    if (pksi[pksnum - 1] != n - 1)
    {
        vlys.push(n - 1);
        //cout<<n-1<<endl;
    }
    if (pksnum == 1)
    {
        while (!vlys.empty())
            vlys.pop();
        vlys.push(0);
        vlys.push(n - 1);
        //cout<<0<<endl;
        //cout<<n-1<<endl;
    }
}

void findTurnPoints(vector<float> &x, vector<float> &pks, vector<float> &vlys)
{

    /// in R: x<-x[!is.na(x)]
    ///       if(length(unique(x)) == 1)
    vector<float> uniquex(x);
    uniquex.erase(unique(uniquex.begin(), uniquex.end()), uniquex.end());
    if (uniquex.size() == 1)
    {
        pks.push_back(round(x.size() / 2.f));
        vlys.push_back(1);
        vlys.push_back(x.size());
    }
    else
    {
        int n = x.size();
        /// in R: z <- embed(rev(as.vector(c(-Inf, x, -Inf))), dim = 3)
        ///       z <- z[rev(seq(nrow(z))), ]
        vector<float> newx(x);
        newx.insert(newx.begin(), MISSINGFLOAT);
        newx.push_back(MISSINGFLOAT);
        vector<vector<float> > matrix(n, vector<float>(3));
        vector<bool> pksIdx(n, false), vlysIdx(n, false);
        int tempIdx, i, j;
        for (i = 0; i < n; i++)
            for (j = 0; j < 3; j++)
                matrix[i][j] = newx[i + j];
        /// in R: v <- max.col(x, ties.method = "first") == 2
        for (i = 0; i < n; i++)
        {
            tempIdx = 0;
            for (j = 1; j < 3; j++)
                if (matrix[i][j] > matrix[i][j - 1])
                    tempIdx = j;
            if (tempIdx == 1)
                pksIdx[i] = true;
        }
        /// in R: v <- max.col(-x, ties.method = "first") == 2
        newx[0] = -1.f * MISSINGFLOAT;
        newx[n + 1] = -1.f * MISSINGFLOAT;
        for (i = 0; i < n; i++)
            for (j = 0; j < 3; j++)
                matrix[i][j] = -1.f * newx[i + j];
        for (i = 0; i < n; i++)
        {
            tempIdx = 0;
            for (j = 1; j < 3; j++)
                if (matrix[i][j] >= matrix[i][j - 1])
                    tempIdx = j;
            if (tempIdx == 1)
                vlysIdx[i] = true;
        }
        priority_queue<int> pksindex;
        priority_queue<int> vlysindex;
        for (i = 0; i < n; i++)
        {
            if (pksIdx[i] & !vlysIdx[i])
            {
                pksIdx[i] = true;
                pks.push_back(i);
            }
            else if (!pksIdx[i] & vlysIdx[i])
            {
                vlysIdx[i] = true;
                vlys.push_back(i);
            }
            else
            {
                pksIdx[i] = false;
                vlysIdx[i] = false;
            }
        }
        sort(pks.begin(), pks.end());
        if (*pks.begin() != 0)
            vlys.push_back(0);
        if (*pks.rbegin() != n - 1)
            vlys.push_back(n - 1);
        if (pks.size() == 1)
        {
            vlys.clear();
            vlys.push_back(0);
            vlys.push_back(n - 1);
        }
        sort(vlys.begin(), vlys.end());
        //vlys.erase(unique(vlys.begin(),vlys.end()),vlys.end());
    }
}

// some basic matrix operation
float matrix_sum(float *x, int n)
{
    int i = 0;
    float sum = 0;
    for (i = 0; i < n; i++)
        sum += x[i];
    return sum;
}

float *matrix_add(float *x, float *y, int n)
{
    float *mtxadd = new float[n];
    int i = 0;
    for (i = 0; i < n; i++)
        mtxadd[i] = x[i] + y[i];
    return mtxadd;
}

float *matrix_minus(float *x, float *y, int n)
{
    float *mtxminus = new float[n];
    int i = 0;
    for (i = 0; i < n; i++)
        mtxminus[i] = x[i] - y[i];
    return mtxminus;
}

float *matrix_times(float *x, float *y, int n)
{
    float *mtxtimes = new float[n];

    int i = 0;
    for (i = 0; i < n; i++)
        mtxtimes[i] = x[i] * y[i];
    return mtxtimes;
}

float *matrix_divide(float *x, float *y, int n)
{
    float *mtxdiv = new float[n];
    int i = 0;
    for (i = 0; i < n; i++)
    {
        if (y[i] < ZERO)
            mtxdiv[i] = MISSINGFLOAT;
        else
            mtxdiv[i] = x[i] / y[i];
    }
    return mtxdiv;
}

float *matrix_add(float *x, float y, int n)
{
    float *mtxadd = new float[n];
    int i = 0;
    for (i = 0; i < n; i++)
        mtxadd[i] = x[i] + y;
    return mtxadd;
}

float *matrix_minus(float *x, float y, int n)
{
    float *mtxminus = new float[n];
    int i = 0;
    for (i = 0; i < n; i++)
        mtxminus[i] = x[i] - y;
    return mtxminus;
}

float *matrix_times(float *x, float y, int n)
{
    float *mtxtimes = new float[n];
    int i = 0;
    for (i = 0; i < n; i++)
        mtxtimes[i] = x[i] * y;
    return mtxtimes;
}

float *matrix_divide(float *x, float y, int n)
{
    float *mtxdiv = new float[n];
    int i = 0;
    for (i = 0; i < n; i++)
        if (y < ZERO)
            mtxdiv[i] = MISSINGFLOAT;
        else
            mtxdiv[i] = x[i] / y;
    return mtxdiv;
}

float *dnorm(float *x, int n, float mean, float sd, bool iflog)
{
    float *p = new float[n];
    int i = 0;
    for (i = 0; i < n; i++)
    {
        if (sd < ZERO)
        {
            p[i] = MISSINGFLOAT;
        }
        else
        {
            p[i] = (1.f / (sd * sqrt(2.f * PI))) * exp(-1.f * (x[i] - mean) * (x[i] - mean) / 2.f / sd / sd);
            if (p[i] < ZERO)
                p[i] = ZERO;
            if (iflog)
                p[i] = log(p[i]);
        }
    }
    return p;
}

bool isNA(float x)
{
    if (x == MISSINGFLOAT || x == -1.f * MISSINGFLOAT)
        return true;
    else
        return false;

}

int Bigauss_esti_moment(vector<float> &x, vector<float> &y, float powerIdx, vector<float> &sigma_ratio_limit,
                        vector<float> &fit)
{
    vector<float> validX;
    vector<float> validY;
    vector<int> valid_Y_idx;
    int i = 0;
    int num = x.size();
    for (i = 0; i < num; i++)
        if (y[i] > ZERO && !isNA(y[i]))
            valid_Y_idx.push_back(i);
    if (valid_Y_idx.size() == x.size())
    {
        validX.resize(x.size());
        copy(x.begin(), x.end(), validX.begin());
        validY.resize(x.size());
        copy(y.begin(), y.end(), validY.begin());
    }
    else if (valid_Y_idx.empty())
        return 0;
    else
    {
        num = valid_Y_idx.size();
        validX.resize(num);
        validY.resize(num);
        for (i = 0; i < num; i++)
        {
            validX[i] = x[valid_Y_idx[i]];
            validY[i] = y[valid_Y_idx[i]];
        }
    }
    if (valid_Y_idx.size() < 2)
    {
        fit.resize(4);
        fit[0] = validX[0];
        fit[1] = 1.f;
        fit[2] = 1.f;
        fit[3] = 0.f;
        return 1;
    }
    else
    {
        vector<float> y_0(validY);
        float max_y_0 = *max_element(y_0.begin(), y_0.end());
        vector<float>::iterator iter = y_0.begin();
        for (; iter != y_0.end(); iter++)
            *iter = pow(*iter / max_y_0, powerIdx);
        y.clear();
        y = validY;
        validY = y_0;
        vector<float> dx(num - 1);
        for (i = 0; i < num - 1; i++)
            dx[i] = validX[i + 1] - validX[i];
        float min_d = *min_element(dx.begin(), dx.end());
        //cout<<min_d<<endl;
        dx.resize(num);
        dx[0] = (validX[1] - validX[0] > 4.f * min_d) ? 4.f * min_d : validX[1] - validX[0];
        dx[num - 1] = (validX[num - 1] - validX[num - 2] > 4.f * min_d) ? 4.f * min_d : validX[num - 1] - validX[num - 2];
        for (i = 1; i < num - 1; i++)
        {
            dx[i] = (validX[i + 1] - validX[i - 1]) / 2.f;
            if (dx[i] > 4.f * min_d)
                dx[i] = 4.f * min_d;
        }
        vector<float> y_cum(num);
        vector<float> x_y_cum(num);
        vector<float> xsqr_y_cum(num);
        vector<float> y_cum_rev(num);
        vector<float> x_y_cum_rev(num);
        vector<float> xsqr_y_cum_rev(num);

        //transform(validY.begin(),validY.end(),dx.begin(),y_cum.begin(),multiplies<float>());
        for (i = 0; i < num; i++)
        {
            y_cum[i] = validY[i] * dx[i];
            x_y_cum[i] = validY[i] * validX[i] * dx[i];
            xsqr_y_cum[i] = validY[i] * validX[i] * validX[i] * dx[i];
            y_cum_rev[i] = y_cum[i];
            x_y_cum_rev[i] = x_y_cum[i];
            xsqr_y_cum_rev[i] = xsqr_y_cum[i];
            if (i != 0)
            {
                y_cum[i] += y_cum[i - 1];
                x_y_cum[i] += x_y_cum[i - 1];
                xsqr_y_cum[i] += xsqr_y_cum[i - 1];
            }
        }
        for (i = num - 2; i >= 0; i--)
        {
            y_cum_rev[i] += y_cum_rev[i + 1];
            x_y_cum_rev[i] += x_y_cum_rev[i + 1];
            xsqr_y_cum_rev[i] += xsqr_y_cum_rev[i + 1];
        }

        int start = num, end = 0;
        for (i = 0; i < num; i++)
        {
            if (y_cum[i] >= sigma_ratio_limit[0] / (sigma_ratio_limit[0] + 1.f) * y_cum[num - 1]) if (start > i)
                start = i;
            if (y_cum[i] <= sigma_ratio_limit[1] / (sigma_ratio_limit[1] + 1.f) * y_cum[num - 1]) if (end < i)
                end = i;
        }
        if (end == 0)
            end = num - 2;
        if (start == num)
            start = 0;
        float m = 0.f, tempmean = 0.f;
        vector<float> m_candi;
        if (end <= start)
        {
            int tempidx = 0;
            for (i = end; i <= start; i++)
                tempmean += validX[i];
            tempmean = (end == start) ? validX[start] : tempmean / (start - end);
            for (i = 0; i < num; i++)
                if (y_cum_rev[i] > 0) if (i > tempidx)
                    tempidx = i;
            m = min(tempmean, validX[tempidx]);
        }
        else
        {
            m_candi.resize(end - start + 1);
            vector<vector<float> > rec(end - start + 1, vector<float>(3));
            vector<float> s1(end - start + 1);
            vector<float> s2(end - start + 1);
            vector<float> d(end - start + 1);
            for (i = start; i <= end; i++)
            {
                m_candi[i - start] = (validX[i + 1] + validX[i]) / 2.f;
                s1[i - start] = sqrt((xsqr_y_cum[i] + m_candi[i - start] * m_candi[i - start] * y_cum[i] -
                                      2.f * m_candi[i - start] * x_y_cum[i]) / y_cum[i]);
                s2[i - start] = sqrt(
                        (xsqr_y_cum_rev[i + 1] + m_candi[i - start] * m_candi[i - start] * y_cum_rev[i + 1] -
                         2.f * m_candi[i - start] * x_y_cum_rev[i + 1]) / y_cum_rev[i + 1]);
                rec[i - start][0] = s1[i - start];
                rec[i - start][1] = s2[i - start];
                rec[i - start][2] = y_cum[i] / y_cum_rev[i + 1];
                d[i - start] = log(rec[i - start][0] / rec[i - start][1]) - log(rec[i - start][2]);
            }
            float temp_d_min = *min_element(d.begin(), d.end());
            float temp_d_max = *max_element(d.begin(), d.end());
            if (temp_d_max * temp_d_min < 0)
            {
                /// operator 0 means less than value, while 1 means greater than.
                pair<vector<float>, vector<int>> d_less_0_ = which(d, 0,0.f);
                vector<float> d_less_0 = d_less_0_.first;
                /// operator 2 means greater_equal than, and 3 means less_equal than
                pair<vector<float>, vector<int>> d_greater_0_ = which(d, 2, 0.f);
                vector<float> d_greater_0 = d_greater_0_.first;
                float temp_d_less_0_max = *max_element(d_less_0.begin(), d_less_0.end());
                float temp_d_greater_0_min = *min_element(d_greater_0.begin(), d_greater_0.end());
                int temp_d_less_0_max_idx = 0;
                int temp_d_greater_0_min_idx = 0;
                for (i = 0; i < d.size(); i++)
                {
                    if (d[i] == temp_d_less_0_max)
                        temp_d_less_0_max_idx = i;
                    if (d[i] == temp_d_greater_0_min)
                        temp_d_greater_0_min_idx = i;
                }
                m = (abs(d[temp_d_less_0_max_idx]) * m_candi[temp_d_less_0_max_idx] +
                     d[temp_d_greater_0_min_idx] * m_candi[temp_d_greater_0_min_idx]) /
                    (d[temp_d_greater_0_min_idx] + abs(d[temp_d_less_0_max_idx]));
            }
            else
            {
                int min_d_idx = d.size() - 1;
                for (i = 0; i < d.size(); i++)
                    d[i] = abs(d[i]);
                for (i = 0; i < d.size(); i++)
                    if (d[i] == temp_d_min)
                        min_d_idx = i;
                m = m_candi[min_d_idx];
            }
        }
        vector<int> sel1, sel2;
        for (i = 0; i < num; i++)
        {
            if (validX[i] >= m)
                sel2.push_back(i);
            if (validX[i] < m)
                sel1.push_back(i);
        }
        float tempS1 = 0.f, tempS1_2 = 0.f, tempS2 = 0.f, tempS2_2 = 0.f;
        float *tempx1 = new float[sel1.size()];
        float *tempx2 = new float[sel2.size()];
        for (i = 0; i < sel1.size(); i++)
        {
            tempS1 += (validX[sel1[i]] - m) * (validX[sel1[i]] - m) * validY[sel1[i]] * dx[sel1[i]];
            tempS1_2 += validY[sel1[i]] * dx[sel1[i]];
            tempx1[i] = validX[sel1[i]];
        }
        tempS1 = sqrt(tempS1 / tempS1_2);
        for (i = 0; i < sel2.size(); i++)
        {
            tempS2 += (validX[sel2[i]] - m) * (validX[sel2[i]] - m) * validY[sel2[i]] * dx[sel2[i]];
            tempS2_2 += validY[sel2[i]] * dx[sel2[i]];
            tempx2[i] = validX[sel2[i]];
        }
        tempS2 = sqrt(tempS2 / tempS2_2);
        if (powerIdx != 1)
        {
            tempS1 *= sqrt(powerIdx);
            tempS2 *= sqrt(powerIdx);
        }
        float *d1 = dnorm(tempx1, sel1.size(), m, tempS1);
        float *d2 = dnorm(tempx2, sel2.size(), m, tempS2);
        vector<float> density;
        for (i = 0; i < sel1.size(); i++)
            density.push_back(d1[i] * tempS1);
        for (i = 0; i < sel2.size(); i++)
            density.push_back(d2[i] * tempS2);
        float scale = 0.f, scale_1 = 0.f;
        for (i = 0; i < num; i++)
        {
            scale += density[i] * density[i] * log(y[i] / density[i]);
            scale_1 += density[i] * density[i];
        }
        scale = exp(scale / scale_1);

        if (isNA(fit[0]) || isNA(fit[1]) || isNA(fit[2]) || isNA(fit[3]))
        {
            float sumY = accumulate(y.begin(), y.end(), 0.f);
            float sumProductXY = inner_product(validX.begin(), validX.end(), y.begin(), 0.f);
            m = sumProductXY / sumY;
            float tempv = 0.f;
            for (i = 0; i < num; i++)
                tempv += y[i] * (x[i] - m) * (x[i] - m);
            tempS1 = tempv / sumY;
            tempS2 = tempS1;
            scale = sumY / tempS1;
        }
        fit[0] = m;
        fit[1] = tempS1;
        fit[2] = tempS2;
        fit[3] = scale;
    }
    return 1;
}

int BiGaussianMix(vector<float> &x, vector<float> &y, vector<float> &sigma_ratio_limit,float bandwidth,float powerIdx,
	int esti_method,float eliminate,float epsilon, int max_iter, vector<vector<float> > &fit_results)
{
	int i = 0, j = 0, k = 0, num_origin;
	if (x.size() != y.size())
	{
		cout<<"x and y should have the same size, please check the input!"<<endl;
		return 0;
	}
	else
	{
		num_origin = x.size(); /// original number of (x,y) coordinates
		//for(i = 0; i < num_origin; i++)  // output the input x,y vectors.
		//	cout<<x[i]<<","<<y[i]<<endl;
		float minX = *min_element(x.begin(),x.end()),maxX = *max_element(x.begin(),x.end()),min_bw,max_bw;
		vector<float> bw;
		min_bw = (maxX - minX)/30;
		max_bw = min_bw * 2;
		bw.push_back(min(max(bandwidth * (maxX - minX),min_bw),max_bw));
		bw.push_back(bw[0]*2);
		if(bw[0] > 1.5*min_bw)
			bw.insert(bw.begin(),max(min_bw,bw[0]/2));
		sort(bw.begin(),bw.end());
		///printVector("bandwidth is: ", bw);
		/// the vector x is already ascent!
		vector<vector<float> > smoother_pk_rec(bw.size()),smoother_vly_rec(bw.size()); /// in R: smoother.pk.rec<-smoother.vly.rec<-new("list")
		vector<float> bic_rec(bw.size()); /// in R: bic.rec<-all.bw, bic_rec: Bayesian Information Criterion
		vector<float> nash_coef(bw.size());
		vector<float **> results(bw.size());
		vector<int> results_group(bw.size());
		float last_num_pks = MISSINGFLOAT; /// in R: last.num.pks<-Inf
		int bw_n;
		int kernel = 2; // kernel can be 1:"box" or 2:"normal", currently, "normal" based smooth is implemented.
		int nn = max(100,num_origin);
		vector<float> nx,ny;
		for(i=0;i<nn;i++)
			nx.push_back(x[0]+i*(x[num_origin-1]-x[0])/(nn-1));
		for(bw_n = bw.size()-1;bw_n>=0;bw_n--)
		{
			///cout<<"--bw.n is: "<<bw_n<<endl;
			float bw_cur = bw[bw_n]; /// in R: bw<-all.bw[bw.n]
			ny.clear();
			BDRksmooth(x, y,nx, ny, kernel,bw_cur); /// ksmooth function in R
			vector<float> pks,vlys;
			findTurnPoints(ny,pks,vlys); /// find peaks and valleys points
			for(i = 0; i < pks.size();i++)
				pks[i] = nx[int(pks[i])];
			for(i = 0; i < vlys.size(); i++)
				vlys[i] = nx[int(vlys[i])];
			vlys.push_back(abs(MISSINGFLOAT));
			vlys.insert(vlys.begin(),MISSINGFLOAT);
			///printVector("  pks is: ",pks);
			///printVector("  vlys is: ", vlys);
			smoother_pk_rec[bw_n] = pks;
			smoother_vly_rec[bw_n] = vlys;
			int pksNum = pks.size();
			if (pks.size() != last_num_pks)
			{

				last_num_pks = pks.size();
				///cout<<"    last.num.pks is: "<<last_num_pks<<endl;
				vector<float> dx(num_origin);
				if (num_origin == 2)
				{
					dx[0] = x[1] - x[0];
					dx[1] = dx[0];
				}
				else
				{
					dx[num_origin-1] = x[num_origin-1] - x[num_origin-2];
					dx[0] = x[1] - x[0];
					for (i = 1; i < num_origin - 1; i++)
						dx[i] = (x[i+1]-x[i-1])/2.0;
				}
				/// INITIATION
				vector<float> m(pks),s1(pks),s2(pks),delta(pks); /// in R: m<-s1<-s2<-delta<-pks
				for (i = 0; i < m.size(); i++)
				{
					/// Calculate s1
					/// in R:
					///      sel.1<-which(x >= max(vlys[vlys < m[i]]) & x < m[i])
					///      s1[i]<-sqrt(sum((x[sel.1]-m[i])^2 * y[sel.1]*dx[sel.1])/sum(y[sel.1]*dx[sel.1]))
					delta[i] = 0.0;
					float tempValue;
					pair<vector<float>,vector<int>> tempVector_ = which(vlys,0,m[i]);
					vector<float> tempVector = tempVector_.first;
					priority_queue<int> tempIndex;
					int tempxNumS1,tempxNumS2;
					float *tempxS1,*tempxS2;
					float *tempyS1,*tempyS2;
					float *tempdxS1,*tempdxS2;

					//auto it = copy_if(vlys.begin(),vlys.end(),tempVector.begin(),bind2nd(less<float>(),m[i]));
					//tempVector.resize(distance(tempVector.begin(),it));
					tempValue = *max_element(tempVector.begin(),tempVector.end());
					/// currently, tempValue is the maximum of vlys that less than m[i], i.e., max(vlys[vlys < m[i]])
					for(j=0;j<num_origin;j++)
						if(x[j] >= tempValue && x[j] < m[i]) /// i.e., which(x >= max(vlys[vlys < m[i]]) & x < m[i])
							tempIndex.push(j);
					if(tempIndex.empty())
					{
						s1[i] = MISSINGFLOAT;
						delta[i] = MISSINGFLOAT;
					}
					else
					{
						tempxNumS1 = tempIndex.size();
						tempxS1 = new float[tempxNumS1];
						if(tempxS1 == NULL) return 0;
						tempyS1 = new float[tempxNumS1];
						if(tempyS1 == NULL) return 0;
						tempdxS1 = new float[tempxNumS1];
						if(tempdxS1 == NULL) return 0;
						for(k = tempxNumS1-1; k >= 0; k--)
						{
							tempxS1[k] = x[tempIndex.top()];
							tempyS1[k] = y[tempIndex.top()];
							tempdxS1[k] = dx[tempIndex.top()];
							tempIndex.pop();
						}
						s1[i] = sqrt(matrix_sum( matrix_times(matrix_times(matrix_minus(tempxS1,m[i],tempxNumS1),matrix_minus(tempxS1,m[i],tempxNumS1),tempxNumS1),matrix_times(tempyS1,tempdxS1,tempxNumS1),tempxNumS1) ,tempxNumS1)/matrix_sum(matrix_times(tempyS1,tempdxS1,tempxNumS1),tempxNumS1));  // this style is not acceptable, but currently, I mainly focus on the functionality....
					}
					/// End the calculation of s1[i]
					/// Calculate s2
					///      sel.2<-which(x >= m[i] & x < min(vlys[vlys > m[i]]))
					///      s2[i]<-sqrt(sum((x[sel.2]-m[i])^2 * y[sel.2] * dx[sel.2])/sum(y[sel.2]*dx[sel.2]))
					tempVector.clear();
					tempVector.resize(vlys.size());

					tempVector_ = which(vlys,1,m[i]);
					tempVector = tempVector_.first;
					//it = copy_if(vlys.begin(),vlys.end(),tempVector.begin(),bind2nd(greater<float>(),m[i]));
					//tempVector.resize(distance(tempVector.begin(),it));
					tempValue = *min_element(tempVector.begin(),tempVector.end());
					// currently, tempValue is the minimum of vlys that greater than m[i]
					for(j=0;j<num_origin;j++)
						if(x[j] < tempValue && x[j] >= m[i])
							tempIndex.push(j);
					if(tempIndex.empty())
					{
						s2[i] = MISSINGFLOAT;
						delta[i] = MISSINGFLOAT;
					}
					else
					{
						tempxNumS2 = tempIndex.size();
						tempxS2 = new float[tempxNumS2];
						if(tempxS2 == NULL) return 0;
						tempyS2 = new float[tempxNumS2];
						if(tempyS2 == NULL) return 0;
						tempdxS2 = new float[tempxNumS2];
						if(tempdxS2 == NULL) return 0;
						for(k = tempxNumS2-1; k >= 0; k--)
						{
							tempxS2[k] = x[tempIndex.top()];
							tempyS2[k] = y[tempIndex.top()];
							tempdxS2[k] = dx[tempIndex.top()];
							tempIndex.pop();
						}
						s2[i] = sqrt(matrix_sum( matrix_times(matrix_times(matrix_minus(tempxS2,m[i],tempxNumS2),matrix_minus(tempxS2,m[i],tempxNumS2),tempxNumS2),matrix_times(tempyS2,tempdxS2,tempxNumS2),tempxNumS2) ,tempxNumS2)/matrix_sum(matrix_times(tempyS2,tempdxS2,tempxNumS2),tempxNumS2));  // this style is not acceptable, but currently, I mainly focus on the functionality....
					}
					/// End the calculation of s2[i]
					/// Calculate delta
					///     in R: delta[i]<-(sum(y[sel.1]*dx[sel.1]) + sum(y[sel.2]*dx[sel.2]))/((sum(dnorm(x[sel.1], mean=m[i], sd=s1[i])) * s1[i] /2)+(sum(dnorm(x[sel.2], mean=m[i], sd=s2[i])) * s2[i] /2))
					if (delta[i] == 0.0)
					{
						delta[i] = (matrix_sum(matrix_times(tempyS1,tempdxS1,tempxNumS1),tempxNumS1) + matrix_sum(matrix_times(tempyS2,tempdxS2,tempxNumS2),tempxNumS2))/((matrix_sum(dnorm(tempxS1,tempxNumS1,m[i], s1[i]),tempxNumS1) * s1[i] /2)+(matrix_sum(dnorm(tempxS2,tempxNumS2,m[i], s2[i]),tempxNumS2) * s2[i] /2));
					}
					//cout<<s1[i]<<","<<s2[i]<<","<<delta[i]<<endl;

				}
				/// END INITIATION
				for (i=0;i<m.size();i++)
				{
					if(delta[i] == MISSINGFLOAT || delta[i] == -1*MISSINGFLOAT)
						delta[i] = 1e-10;
					if(s1[i] == MISSINGFLOAT || s1[i] == -1*MISSINGFLOAT)
						s1[i] = 1e-10;
					if(s2[i] == MISSINGFLOAT || s2[i] == -1*MISSINGFLOAT)
						s2[i] = 1e-10;
				}
				//printVector("    s1 is: ",s1);
				//printVector("    s2 is: ",s2);
				//printVector("    delta is: ",delta);
				//printVector("    m is: ",m);
				vector<vector<float> > fit(x.size(),vector<float>(pksNum,0.0f)); /// in R: fit<-matrix(0,ncol=length(m), nrow=length(x))
				float this_change = -1 * MISSINGFLOAT;
				int counter = 0;
				vector<float> cuts;
				int this_bigauss;
				while (this_change > 0.1 && counter <= max_iter)
				{
					///cout<<"      this.change is: "<<this_change<<",iterator number is: "<<counter<<endl;
					counter++;
					vector<float> old_m(m);
					///  E step
					cuts = m;
					cuts.push_back(-1 * MISSINGFLOAT);
					cuts.insert(cuts.begin(),MISSINGFLOAT); /// in R: cuts<-c(-Inf, m, Inf)
					for (i = 1; i < cuts.size(); i++)
					{
						priority_queue<int> tempIndex;
						for(j = 0; j < x.size(); j++)
							if(x[j] >= cuts[i-1] && x[j] < cuts[i])  /// in R: sel<-which(x >= cuts[i-1] & x < cuts[i])
								tempIndex.push(j);
						if(!tempIndex.empty())
						{
							int tempxNum = tempIndex.size();
							float *tempx = new float[tempxNum];
							if(tempx == NULL) return 0;
							int *sel = new int[tempxNum];
							for(k = tempxNum-1; k >= 0; k--)
							{
								sel[k] = tempIndex.top();
								tempx[k] = x[sel[k]];
								tempIndex.pop();
							}
							/// tempIndex is empty now
							float *s_to_use = new float[s2.size()];
							if(s_to_use == NULL) return 0;
							for(j = 0; j < s2.size(); j++)
							{
								s_to_use[j] = s2[j]; /// in R: s.to.use<-s2
								if(j >= (i - 1))  /// in R: use.s1<-which(1:length(m) >= (i-1))
									tempIndex.push(j);
							}
							if(!tempIndex.empty()) /// tempIndex is use.s1
							{
								int tempxNum2 = tempIndex.size();
								for(k = tempxNum2-1; k >= 0; k--)
								{
									int iii = tempIndex.top();
									s_to_use[iii] = s1[iii]; /// in R: s.to.use[use.s1]<-s1[use.s1]
									tempIndex.pop();
								}
							}
							float *temp_dnorm = new float[tempxNum];
							if(temp_dnorm == NULL) return 0;
							for(k=0;k<fit[0].size();k++){  /// cols
								temp_dnorm = dnorm(tempx,tempxNum, m[k], s_to_use[k]);
								for(j = 0;j < tempxNum; j++){ /// rows
									fit[sel[j]][k] = temp_dnorm[j]  * s_to_use[k] * delta[k];
								}
							}
						}
					}
					//for (int k = 0; k < x.size(); k++)
					//{
					//	string sss = "      E step, fit["+toString(k)+"] is: ";
					//	printVector(sss,fit[k]);
					//}
					/// Elimination step
					float sum_fit;
					vector<vector<float> > fit2(num_origin,vector<float>(fit[0].size()));
					for(i=0;i<num_origin;i++){
						sum_fit = 0.0;
						for(j=0;j<fit[0].size();j++){
							if(isNA(fit[i][j]))
								fit[i][j] = 0.0;
							sum_fit += fit[i][j];
						}
						for(j=0;j<fit[0].size();j++){
							fit[i][j] /= sum_fit;
							fit2[i][j] = fit[i][j] * y[i];
						}
					}
					/// in R: perc.explained<-apply(fit2,2,sum)/sum(y)
					float *perc_explained = new float[fit2[0].size()];
					if(perc_explained == NULL) return 0;
					float sumY = accumulate(y.begin(),y.end(),0.0);
					for(j=0;j<fit2[0].size();j++){
						perc_explained[j] = 0;
						for(i=0;i<num_origin;i++)
							perc_explained[j]+=fit2[i][j];
						perc_explained[j]/= sumY;
					}
					//printArray("      E step, perc_explained is: ",perc_explained, fit2[0].size());
					int max_erase = max(0,int(round(float(fit2[0].size())/5.0f))); /// in R: max(1, round(length(perc.explained)/5))
					/// in R: to.erase<-which(perc.explained <= min(eliminate, perc.explained[order(perc.explained, na.last=FALSE)[max.erase]]))
					int *perc_explained_order = new int[fit2[0].size()];
					if(perc_explained_order == NULL) return 0;
					perc_explained_order = order(perc_explained,fit2[0].size(),false);
					//printArray("      E step, perc_explained is: ",perc_explained, fit2[0].size());
					//printArray("      E step, perc_explained_order is: ",perc_explained_order, fit2[0].size());
					vector<int> to_erase_que;
					if (max_erase <= fit2[0].size())
					{
						float tempEliminate = min(eliminate,perc_explained[perc_explained_order[max_erase]]);
						for(j = 0;j<fit2[0].size();j++)
							if(perc_explained[j] <= tempEliminate)
								to_erase_que.push_back(j);
					}
					int to_erase_que_size = 0;
					if (!to_erase_que.empty())
					{
						for (j = fit2[0].size()-1; j >= 0; j--)
						{
							if(!(find(to_erase_que.begin(),to_erase_que.end(),j)==to_erase_que.end()))
							{
								m.erase(m.begin()+j); /// in R: m<-m[-to.erase]
								s1.erase(s1.begin()+j);
								s2.erase(s2.begin()+j);
								delta.erase(delta.begin()+j);
								for (k=0;k<num_origin;k++)
									fit[k].erase(fit[k].begin()+j);
								old_m.erase(old_m.begin()+j);
							}
						}
						for(i=0;i<num_origin;i++){
							sum_fit = accumulate(fit[i].begin(),fit[i].end(),0.0f);
							for(j=0;j<fit[i].size();j++)
								if(sum_fit == 0.0)
									fit[i][j] = MISSINGFLOAT;
								else
									fit[i][j] /= sum_fit;
						}
					}
					///printVector("      Elimination step, old.m is: ",old_m);
					/// M setp

					for (i = 0; i < m.size(); i++)
					{
						vector<float> this_y(y.size());
						for (j = 0; j < y.size(); j++)
							if(isNA(fit[j][i]))
								this_y[j] = MISSINGFLOAT;
							else
								this_y[j] = fit[j][i] * y[j];
						vector<float> this_fit(4);

						if (esti_method == 0)
							this_bigauss = Bigauss_esti_moment(x,this_y,powerIdx,sigma_ratio_limit,this_fit);
						else
							this_bigauss = Bigauss_esti_em(x,this_y,max_iter,epsilon,powerIdx,sigma_ratio_limit,this_fit);
						if (this_bigauss == 0){
							counter = max_iter+1;
							m[i] = MISSINGFLOAT;
							s1[i] = MISSINGFLOAT;
							s2[i] = MISSINGFLOAT;
							delta[i] = MISSINGFLOAT;
						}
						else
						{
							m[i] = this_fit[0];
							s1[i] = this_fit[1];
							s2[i] = this_fit[2];
							delta[i] = this_fit[3];
						}

					}
					for(i = 0; i < delta.size(); i++)
						if(isNA(delta[i]))
							delta[i] = 0.0f;
					/// amount of change
					if(isNA(this_change))
						this_change = 0.0f;
					for(i = 0; i < m.size(); i++)
						if(!isNA(m[i]))
							this_change += (old_m[i]-m[i])*(old_m[i]-m[i]);
					//printVector("      M step, m is: ",m);
					//printVector("      M step, s1 is: ",s1);
					//printVector("      M step, s2 is: ",s2);
					//printVector("      M step, delta is: ",delta);
					//cout<<"      M step, this.change is: "<<this_change<<endl;
				}
				cuts.clear();
				cuts = m;
				cuts.push_back(-1 * MISSINGFLOAT);
				cuts.insert(cuts.begin(),MISSINGFLOAT);
				for (j = 0; j < fit.size(); j++)
				{
					for (k = 0; k < fit[j].size(); k++)
					{
						fit[j][k] = 0.0;
					}
				}
				for (j = 1; j < cuts.size(); j++)
				{
					vector<int> sel,use_s1;
					for (k = 0; k < num_origin; k++)
					{
						if(x[k] >= cuts[j-1] && x[k] < cuts[j])
							sel.push_back(k);
					}
					float *tempx = new float[sel.size()];
					if(tempx == NULL) return 0;
					for (k = 0; k < sel.size(); k++)
						tempx[k] = x[sel[k]];

					for (k = 0; k < m.size(); k++)
						if(k >= (j-1))
							use_s1.push_back(k);
					vector<float> s_to_use(s2);
					if(!use_s1.empty())
						for (i = 0; i < use_s1.size(); i++)
							s_to_use[use_s1[i]] = s1[use_s1[i]];
					for (i = 0; i < fit[0].size(); i++)
					{
						float *temp_dnorm = dnorm(tempx,sel.size(),m[i],s_to_use[i]);
						if (s_to_use[i] != 0)
						{
							for (k = 0; k < sel.size(); k++)
							{
								fit[sel[k]][i] = temp_dnorm[k] * s_to_use[i] * delta[i];
							}
						}
					}
				}
				vector<float> area(delta.size());
				for (i = 0; i < delta.size(); i++)
				{
					if (!isNA(delta[i]))
						area[i] = delta[i] * (s1[i] + s2[i]) / 2;
					else
						area[i] = MISSINGFLOAT;
				}
				float rss = 0.0, fit_mean = 0.0, rss2 = 0.0;
				for (i = 0; i < y.size(); i++)
					fit_mean += accumulate(fit[i].begin(),fit[i].end(),0.0);
				fit_mean /= (fit.size() * fit[0].size());
				for (i = 0; i < y.size(); i++)
				{
					float tempfitrow = accumulate(fit[i].begin(),fit[i].end(),0.0);
					rss += (y[i] - tempfitrow)*(y[i] - tempfitrow);
					rss2 += (fit_mean - tempfitrow)*(fit_mean - tempfitrow);
				}
				num_origin = x.size();
				float bic,nash;
				if(this_bigauss == 1){
					bic = num_origin * log(rss/num_origin) + 4 * m.size() * log((float)num_origin);
					nash = 1 - rss / rss2;
				}
				else{
					bic = MISSINGFLOAT;
					nash = MISSINGFLOAT;
				}
				results[bw_n] = new float*[m.size()];
				if(results[bw_n] == NULL) return 0;
				results_group[bw_n] = m.size();
				for (i = 0; i < m.size(); i++)
				{
					results[bw_n][i] = new float[5];
					results[bw_n][i][0] = m[i];
					results[bw_n][i][1] = s1[i];
					results[bw_n][i][2] = s2[i];
					results[bw_n][i][3] = delta[i];
					results[bw_n][i][4] = area[i];
				}
				bic_rec[bw_n] = bic;
				nash_coef[bw_n] = nash;
			}
			else
			{
				results[bw_n] = NULL;
				bic_rec[bw_n] = MISSINGFLOAT;
				results[bw_n] = results[bw_n + 1];
			}
		}
		int sel = 0, sel2 = 0, sel_single = 0;
		vector<int> sel_v, sel_v2,sel_single_v;
		float temp_bic_rec,temp_nash_coef;
		for(i = 0; i < results_group.size(); i++)
		{
			if(results_group[i] == 1 && !isNA(bic_rec[i]))
				sel_single_v.push_back(i);
			else if(results_group[i] == 1 && !isNA(bic_rec[i]))
				sel_v.push_back(i);
		}
		if(sel_single_v.size() == 1)
			sel_single = sel_single_v[0];
		else if(sel_single_v.size() > 1)
		{
			sel_single = sel_single_v[0];
			temp_bic_rec = bic_rec[sel_single];
			temp_nash_coef = nash_coef[sel_single];
			for(i = 1; i < sel_single_v.size(); i++)
				if(bic_rec[sel_single_v[i]] < temp_bic_rec || nash_coef[sel_single_v[i]] > temp_nash_coef)
					sel_single = sel_single_v[i];
		}
		else
			sel_single = MISSINGSHORT;
		if(sel_v.size() == 1)
			sel = sel_v[0];
		else if(sel_v.size() > 1)
		{
			sel = sel_v[0];
			temp_bic_rec = bic_rec[sel];
			temp_nash_coef = nash_coef[sel];
			for(i = 1; i < sel_v.size(); i++)
				if(bic_rec[sel_v[i]] < temp_bic_rec || nash_coef[sel_v[i]] > temp_nash_coef)
					sel = sel_v[i];
		}
		else
			sel = MISSINGSHORT;

		if(sel == MISSINGSHORT && sel_single != MISSINGSHORT)
			sel2 = sel_single;
		else if(sel != MISSINGSHORT && sel_single == MISSINGSHORT)
			sel2 = sel;
		else if(sel != sel_single && sel != MISSINGSHORT && sel_single != MISSINGSHORT)
		{
			if(nash_coef[sel_single] >= 0.8 * nash_coef[sel] || bic_rec[sel_single] >= 0.8 * bic_rec[sel])
				sel2 = sel_single;
			else
				sel2 = sel;
		}
		else
			sel2 = MISSINGSHORT;
		if(sel2 == MISSINGSHORT)
			return 0;
		else
		{
			fit_results.resize(results_group[sel2]);
			for (i = 0; i < results_group[sel2]; i++)
			{
				fit_results[i].resize(5);
				fit_results[i][0] = results[sel2][i][0]; /// m : peak center
				fit_results[i][1] = results[sel2][i][1]; /// s1: sigma1 (left)
				fit_results[i][2] = results[sel2][i][2]; /// s2: sigma2 (right)
				fit_results[i][3] = results[sel2][i][3]; /// delta
				fit_results[i][4] = nash_coef[sel2];     /// nash: nash-sutcliffe coefficient
			}
			return 1;
		}
	}
}


int Bigauss_esti_em(vector<float> &old_x, vector<float> &old_y, int max_iter, float epsilon, float powerIdx,
                    vector<float> &sigma_ratio_limit, vector<float> &fit)
{
    //function takes into x and y, and then computes the value of
    //sigma.1, sigma.2 and a using iterative method. the returned
    //values include estimated sigmas, a and a boolean variable on
    //whether the termination criteria is satisfied upon the end of the
    //program.
    pair<vector<float>, vector<int>> sel = which(old_y, 1, 1e-10f);
    vector<int> yIdx = sel.second;
    vector<float> y = sel.first;
    fit.clear();
    if (y.size() == 0)
    {
        fit.push_back(median(old_y));
        fit.push_back(1.f);
        fit.push_back(1.f);
        fit.push_back(0.f);
        return 1;
    }
    if (y.size() == 1)
    {
        fit.push_back(y[0]);
        fit.push_back(1.f);
        fit.push_back(1.f);
        fit.push_back(0.f);
        return 1;
    }
    vector<float> x = copyByIndex(old_x, yIdx);
    /// epsilon is the threshold for continuing the iteration. change in
    /// a smaller than epsilon will terminate the iteration

    /// using the median value of x as the initial value of a (the breaking point).
    float a_old = x.at(distance(y.begin(), max_element(y.begin(), y.end()))); /// in R: a.old <- x[which(y==max(y))[1]]
    float a_new = a_old;
    float change = 10.f * epsilon;
    /// n_iter is the number of iteration covered so far.
    int n_iter = 0;
    pair<float, float> sigma;
    while (change > epsilon && n_iter < max_iter)
    {
        ///cout<<"Bigauss estim em, n.iterator is: "<<n_iter<<", change is: "<<change<<endl;
        ///cout<<"a_old is: "<<a_old<<", a_new is: "<<a_new<<endl;
        a_old = a_new;
        n_iter++;
        sigma = solveSigma(x, y, a_old);
        if (n_iter == 1)
        {
            /// sigma[is.na(sigma)]<-as.numeric(sigma[which(!is.na(sigma))])[1]/10
            if (isNA(sigma.first) && !isNA(sigma.second))
                sigma.first = sigma.second / 10.f;
            if (isNA(sigma.second) && !isNA(sigma.first))
                sigma.second = sigma.first / 10.f;
        }
        a_new = solveBreakPoint(x, y, a_old, sigma.first, sigma.second);
        change = abs(a_new - a_old);
    }
    vector<float> d(y), dnew(y);
    sigma.first = sqrt(sigma.first);
    sigma.second = sqrt(sigma.second);
	if (sigma.first != sigma.first) sigma.first = MISSINGFLOAT;
	if (sigma.second != sigma.second) sigma.second = MISSINGFLOAT;
    /// d[t<a.new]<-dnorm(t[t<a.new],mean=a.new,sd=sigma$sigma.1)*sigma$sigma.1
    pair<vector<float>, vector<int>> d1 = which(x, 0, a_new);
    int d1_num = d1.first.size();
    float *d1_dnorm = new float[d1_num];
    for (int i = 0; i < d1_num; i++)
        d1_dnorm[i] = d1.first.at(i);
    float *d1_dnorm_new = dnorm(d1_dnorm, d1_num, a_new, sigma.first);
    for (int i = 0; i < d1_num; i++)
        dnew[d1.second[i]] = d1_dnorm_new[i] * sigma.first;

    /// d[t>=a.new]<-dnorm(t[t>=a.new],mean=a.new,sd=sigma$sigma.2)*sigma$sigma.2
    pair<vector<float>, vector<int>> d2 = which(x, 2, a_new);
    int d2_num = d2.first.size();
    float *d2_dnorm = new float[d2_num];
    for (int i = 0; i < d2_num; i++)
        d2_dnorm[i] = d2.first.at(i);
    float *d2_dnorm_new = dnorm(d2_dnorm, d2_num, a_new, sigma.second);
    for (int i = 0; i < d2_num; i++)
        dnew[d2.second[i]] = d2_dnorm_new[i] * sigma.second;
    pair<vector<float>, vector<int>> dnew2 = which(dnew, 1, 1e-3f);
    float sum1 = 0.f, sum2 = 0.f;
    for (int i = 0; i < dnew2.first.size(); i++)
    {
        float tempD = dnew2.first[i];
        float tempY = y[dnew2.second[i]];
        sum1 += pow(tempD, 2.f) * log(tempY / tempD);
        sum2 += pow(tempD, 2.f);
    }

    /// scale<-exp(sum(d[d>1e-3]^2*log(x[d>1e-3]/d[d>1e-3]))/sum(d[d>1e-3]^2))
	float scale = MISSINGFLOAT;
	if(sum2 > ZERO)
		scale = exp(sum1 / sum2);
	if(scale != scale) scale = MISSINGFLOAT;
    if (!fit.empty()) fit.clear();
    fit.push_back(a_new);
    fit.push_back(sigma.first);
    fit.push_back(sigma.second);
    fit.push_back(scale);
    return 1;
}

float solveBreakPoint(vector<float> &x, vector<float> &y, float a, float sigma1, float sigma2)
{
    /// w <- y * (as.numeric(x<a)/sigma.1 + as.numeric(x>=a)/sigma.2)
    float tempSum1 = 0.f, tempSum2 = 0.f;
    for (int i = 0; i < x.size(); i++)
    {
        float tempV;
        if (x[i] < a)
            tempV = y[i] * 1.f / sigma1;
        else
            tempV = y[i] * 1.f / sigma2;
        tempSum1 += x[i] * tempV;
        tempSum2 += tempV;
    }
    return tempSum1 / tempSum2;
}

pair<float, float> solveSigma(vector<float> &x, vector<float> &y, float a)
{
    /// this function calculate the square estimated of sigma1 and sigma2
    /// take the x, y, and assumed breaking point a
    vector<float> temp(x.size());
    float u = 0.f, v = 0.f;
    for (int i = 0; i < x.size(); i++)
    {
        float tempV = (x[i] - a) * (x[i] - a) * y[i];
        temp.push_back(tempV); /// temp <- (x-a)^2 * y
        if (x[i] < a)
            u += tempV; /// u <- sum(temp * as.numeric(x<a))
        if (x[i] >= a)
            v += tempV; /// v <- sum(temp * as.numeric(x>=a))
    }
    float y_sum = accumulate(y.begin(), y.end(), 0.f);
    float sigma1, sigma2;
    if (u < ZERO)
        sigma1 = MISSINGFLOAT;
    else
        sigma1 = u / y_sum * (pow((v / u), 1.f / 3.f) + 1.f);
    if (v < ZERO)
        sigma2 = MISSINGFLOAT;
    else
        sigma2 = v / y_sum * (pow((u / v), 1.f / 3.f) + 1.f);
	if (sigma1 != sigma1) sigma1 = MISSINGFLOAT;
	if (sigma2 != sigma2) sigma2 = MISSINGFLOAT;
    return make_pair(sigma1, sigma2);
}

float median(vector<float> &x)
{
    vector<float> newx(x);
    sort(newx.begin(), newx.end());
    int n = newx.size();
    return n % 2 ? newx[n / 2] : (newx[n / 2 - 1] + newx[n / 2]) / 2.f;
}

///
pair<vector<float>, vector<int>> which(vector<float> &x, int op, float v)
{
    vector<float> tempVector(x.size());

    vector<int> idxVector(x.size());
    int count = 0;
    for (auto it = x.begin(); it != x.end(); ++it)
    {
        int idx = distance(x.begin(), it);
        bool flag = false;
        switch (op)
        {
            case 0:
            {
                flag = *it < v;
                break;
            }
            case 1:
            {
                flag = *it > v;
                break;
            }
            case 2:
            {
                flag = *it >= v;
                break;
            }
            case 3:
            {
                flag = *it <= v;
                break;
            }
            default:
                break;
        }
        if (flag)
        {
            idxVector[count] = idx;
            tempVector[count] = x.at(idx);
            count++;
        }
    }
    idxVector.resize(count);
    tempVector.resize(count);
    return make_pair(tempVector, idxVector);
}

vector<float> copyByIndex(vector<float> &x, vector<int> &idx)
{
    vector<float> newV(idx.size());
    for (auto it = idx.begin(); it != idx.end(); ++it)
    {
        int indexNum = distance(idx.begin(), it);
        newV[indexNum] = x.at(*it);
    }
    return newV;
}

float STDcal(float *values, int num, bool flag, float ptv)
{
    vector<float> calValue;
    int i = 0;
    for (i = 0; i < num; i++)
    {
        if (flag)
        { // if flag is true, calculate values greater equal than ptv
            if (values[i] >= ptv)
                calValue.push_back(values[i]);
        }
        else
        {
            if (values[i] <= ptv)
                calValue.push_back(values[i]);
        }
    }
    //float mean = accumulate(calValue.begin(), calValue.end(), 0.f);
    //mean /= calValue.size();
    //float sigma = 0.f;
    //for (i = 0; i < calValue.size(); i++)
    //    sigma += (calValue[i] - mean) * (calValue[i] - mean);
    //sigma = sqrt(sigma / calValue.size());
    return std_vector(calValue);
}

int CountIF(float *values, int num, bool flag, float v)
{
    int count = 0;
    for (int i = 0; i < num; i++)
    {
        if (flag)
        {
            if (values[i] >= v)
                count++;
        }
        else
        {
            if (values[i] <= v)
                count++;
        }
    }
    return count;
}

pair<int, int> findValue(vector<float> valueVector, float v)
{
    /// valueVector is already sorted by ascent.
    float minVdiff = -1.f * MISSINGFLOAT, maxVdiff = MISSINGFLOAT;
    int minIdx = -1, maxIdx = -1;
    for (int i = 0; i < valueVector.size(); i++)
    {
        float tempv = v - valueVector[i];
        if (tempv <= minVdiff && tempv > 0.f && minIdx < i)
        {
            minVdiff = tempv;
            minIdx = i;
        }
        if (tempv >= maxVdiff && tempv < 0.f && maxIdx < i)
        {
            maxVdiff = tempv;
            maxIdx = i;
        }
    }
    return make_pair(minIdx, maxIdx);
}

float calNash(vector<float> &x, vector<float> &y, float *params)
{
	vector<float> newy(x.size());
	float a = params[0];
	float sigma1 = params[1];
	float sigma2 = params[2];
	float delta = params[3];
	float tmp = delta / sqrt(2*PI);
	for (int i = 0; i < x.size(); i++)
	{
		if(x[i] < a)
			newy[i] = tmp * exp(-1.f*(x[i] - a)*(x[i] - a)/(2.f*sigma1*sigma1));
		else
			newy[i] = tmp * exp(-1.f*(x[i] - a)*(x[i] - a)/(2.f*sigma2*sigma2));
	}
	float rss = 0.f, fit_mean = 0.f, rss2 = 0.f;
	fit_mean = accumulate(newy.begin(), newy.end(), 0.f) / newy.size();
	for (int i = 0; i < y.size(); i++)
	{
		rss += (y[i] - newy[i]) * (y[i] -  newy[i]);
		rss2 += (y[i] - fit_mean) * (y[i] - fit_mean);
	}
	float nash = 1.f - rss / rss2;
	return nash;
}
