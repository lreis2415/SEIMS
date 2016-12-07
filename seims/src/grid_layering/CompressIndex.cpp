// Author: Junzhi Liu
// Date: 2013-04-19

int CalCompressedIndex(int n, int *mask, int noDataValue, int *compressedIndex)
{
        int counter = 0;
        for (int i = 0; i < n; ++i)
        {
                if (mask[i] == noDataValue)
                        compressedIndex[i] = -1;
                else
                        compressedIndex[i] = counter++;
        }
        return counter;
}
