/*!
 * \brief 
 * \author Junzhi Liu
 * \version 1.0
 * \date June 2010
 */
#include "clsSimpleTxtData.h"
#include "utils.h"
#include <fstream>
//#include "util.h"
#include "ModelException.h"
//#include <string>

clsSimpleTxtData::clsSimpleTxtData(string fileName)
{
    m_row = 0;
    m_data = NULL;

    utils util;
    if (!util.FileExists(fileName)) throw ModelException("clsSimpleTxtData", "ReadFile", "The file " + fileName +
                                                                                         " does not exist or has not read permission.");

    //StatusMessage(("read " + fileName + "...").c_str());

    ifstream myfile;
    myfile.open(fileName.c_str(), ifstream::in);
    string line;
    utils utl;

    //get number of lines
    if (myfile.is_open())
    {
        vector<float> data;

        while (!myfile.eof())
        {
            if (myfile.good())
            {
                getline(myfile, line);
                utl.TrimSpaces(line);
                if ((line.size() > 0) && (line[0] != '#')) // ignore comments and empty lines
                {
                    vector<string> tokens = utl.SplitString(line, '|');
                    if (tokens.size() > 0)
                    {
                        utl.TrimSpaces(tokens[0]);
                        data.push_back(float(atof(tokens[0].c_str())));//add data
                    }
                }
            }
        }
        myfile.close();

        m_row = int(data.size());
        if (m_row > 0)
        {
            m_data = new float[m_row];
            vector<float>::iterator it;
            int i = 0;
            for (it = data.begin(); it < data.end(); it++)
            {
                m_data[i] = *it;
                i++;
            }
        }

    }
}

clsSimpleTxtData::~clsSimpleTxtData()
{
    if (m_data != NULL) delete[] m_data;
}

void clsSimpleTxtData::dump(ostream *fs)
{
    if (fs == NULL) return;
    if (m_data == NULL) return;
    for (int i = 0; i < m_row; i++)
    {
        *fs << m_data[i] << endl;
    }
}

void clsSimpleTxtData::getData(int *nRow, float **data)
{
    *nRow = m_row;
    *data = m_data;
}
