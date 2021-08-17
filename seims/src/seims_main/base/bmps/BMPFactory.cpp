#include "BMPFactory.h"

using namespace bmps;

BMPFactory::BMPFactory(const int scenario_id, const int bmp_id, const int sub_scenario,
                       const int bmp_type, const int bmp_priority, vector<string>& distribution,
                       const string& collection, const string& location, bool effectivenessChangeable,
                       int changeFrequency, int variableTimes) :
    m_scenarioId(scenario_id), m_bmpId(bmp_id), m_subScenarioId(sub_scenario), m_bmpType(bmp_type),
    m_bmpPriority(bmp_priority),
    m_distribution(distribution), m_bmpCollection(collection), m_location(location), m_effectivenessChangeable(effectivenessChangeable),
    m_changeFrequency(changeFrequency), m_changeTimes(variableTimes) {
    /// Do nothing.
}
