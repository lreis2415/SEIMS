/*!
 * \file StringList.h
 * \ingroup util
 * \brief String list related class
 * \author Junzhi Liu
 * \version 1.1
 * \date Jul. 2010
 *
 * 
 */

#pragma once

#include <string>
#include <vector>

using namespace std;

/*!
 * \ingroup Util
 * \class StringList
 *
 * \brief Common operation of string vector
 *
 *
 *
 */
class StringList
{
private:
    vector<string> m_List;

public:
    //! Constructor (void)
    StringList(void);

    //! Destructor, clear the string \a vector
    ~StringList(void);

    //! Assignment operator overload
    StringList &operator=(const StringList &obj);

    //! Add an item to the list
    void Add(string item);

    //! Get the item in the list at the given position
    string At(int postion);

    //! Append an item to the end of the list
    void Append(string item);

    //! Clear the list
    void Clear(void);

    //! Return a flag indicating if the given string is in the list
    bool Contains(string item);

    //! Return the number of items in the list
    int Count(void);

    //! Insert an item into the list at the given position
    void Insert(int index, string item);

    //! Resize the list to the given size with the given default values
    void Resize(int newsize, string value);
};

