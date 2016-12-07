/*!
 * \brief Methods for the StringList utility class
 * \author Junzhi Liu
 * \version 1.1
 * \date Jul. 2010
 *
 * 
 */
#include "StringList.h"
#include "util.h"

StringList::StringList(void)
{
}

StringList::~StringList(void)
{
    m_List.clear();
}

StringList &StringList::operator=(const StringList &obj)
{
    m_List.resize(obj.m_List.size());
    for (size_t itm = 0; itm < obj.m_List.size(); itm++)
    {
        m_List[itm] = obj.m_List[itm];
    }
    return *this;
}

void StringList::Add(string item)
{
    m_List.push_back(item);
}

void StringList::Append(string item)
{
    m_List.insert(m_List.end(), item);
}

string StringList::At(int position)
{
    string res = "";

    if (position >= 0 && position < (int) m_List.size())
    {
        res = m_List[position];
    }

    return res;
}


void StringList::Clear(void)
{
    m_List.clear();
}

int StringList::Count(void)
{
    return m_List.size();
}

bool StringList::Contains(string item)
{
    bool bStatus = false;

    if (m_List.size() > 0)
    {
        for (size_t i = 0; i < m_List.size(); i++)
        {
            if (StringMatch(m_List[i], item))
            {
                bStatus = true;
            }
        }
    }

    return bStatus;
}

void StringList::Insert(int index, string item)
{
    m_List[index] = item;
}

void StringList::Resize(int newsize, string value)
{
    m_List.resize(newsize, value);
}