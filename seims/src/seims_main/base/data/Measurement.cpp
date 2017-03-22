#include "Measurement.h"

Measurement::Measurement(mongoc_client_t *conn, string hydroDBName, string sitesList, string siteType, time_t startTime,
                         time_t endTime)
    : m_conn(conn), m_hydroDBName(hydroDBName), m_type(siteType),
      m_startTime(startTime), m_endTime(endTime), pData(NULL) {
    m_siteIDList = SplitStringForInt(sitesList, ',');
    sort(m_siteIDList.begin(), m_siteIDList.end());
    //vector<string> vecSites = SplitString(sitesList, ',');
    //int nSites = vecSites.size();
    ////convert from string to int and sort
    //for (int iSite = 0; iSite < nSites; iSite++)
    //    m_siteIDList.push_back(atoi(vecSites[iSite].c_str()));
    //sort(m_siteIDList.begin(), m_siteIDList.end());
    pData = new float[m_siteIDList.size()];
}

Measurement::~Measurement(void) {
    if (pData != NULL) {
        Release1DArray(pData);
    }
}
