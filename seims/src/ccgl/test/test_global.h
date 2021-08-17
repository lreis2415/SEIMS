#ifndef CCGLTEST_GLOBAL_DEFINITIONS_H
#define CCGLTEST_GLOBAL_DEFINITIONS_H
#include "../src/basic.h"

class GlobalEnvironment : public ::testing::Environment {
public:
    GlobalEnvironment(string host, ccgl::vint16_t port) : mongoHost(std::move(host)), mongoPort(port) {}
    virtual ~GlobalEnvironment() {}

    // Override this to define how to set up the environment.
    void SetUp() override {}

    // Override this to define how to tear down the environment.
    void TearDown() override {}
public:
    string mongoHost;
    ccgl::vint16_t mongoPort;
};

#endif /* CCGLTEST_GLOBAL_DEFINITIONS_H */
