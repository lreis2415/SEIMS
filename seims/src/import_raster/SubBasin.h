#pragma once

class SubBasin
{
public:
    SubBasin(void);

    SubBasin(int xMin, int yMin, int xMax, int yMax)
    {
        this->xMin = xMin;
        this->yMin = yMin;
        this->xMax = xMax;
        this->yMax = yMax;
        this->cellCount = 0;
    };

    ~SubBasin(void);

    int xMin, yMin, xMax, yMax;
    // count of valid cells
    int cellCount;
};

