#include "Settings.h"

Settings::Settings(vector<string>& str1dvec) {
    SetSettingTagStrings(str1dvec);
}

string Settings::GetValue(const string& tag) {
    string res = "";
    if (!m_Settings.empty()) {
        for (size_t idx = 0; idx < m_Settings.size(); idx++) {
            assert(m_Settings[idx].size() == 2);
            if (StringMatch(m_Settings[idx][0], tag)) {
                res = m_Settings[idx][1];
                break;
            }
        }
    } else {
        cout << "m_Settings is empty, please check and confirm!" << endl;
    }
    return res;
}

void Settings::SetSettingTagStrings(vector<string>& stringvector) {
    assert(!stringvector.empty());
    for (auto iter = stringvector.begin(); iter != stringvector.end(); ++iter) {
        // parse the line into separate items
        vector<string> tokens = SplitString(*iter, '|');
        // is there anything in the token list?
        if (!tokens.empty()) {
            for (size_t i = 0; i < tokens.size(); i++) {
                tokens[i] = trim(tokens[i]);
            }
            // is there anything in the first item?
            if (!tokens[0].empty()) {
                // there is something to add so resize the header list to append it
                size_t sz = m_Settings.size(); // get the current number of rows
                m_Settings.resize(sz + 1);  // resize with one more row
                m_Settings[sz] = tokens;
            } // if there is nothing in the first item of the token list there is nothing to add to the header list
        }
    }
}
