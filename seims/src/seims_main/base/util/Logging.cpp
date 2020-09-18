#include "utils_string.h"
#include "Logging.h"

using namespace ccgl::utils_string;
using std::string;

/*
Levels:
     TRACE
     DEBUG
     INFO
     WARNING
     ERROR
     FATAL
*/

el::base::type::StoragePointer sharedLoggingRepository() {
    return el::Helpers::storage();
}

el::Configurations Logging::gDefaultConf;

void Logging::setFmt(bool timestamps) {
    std::string datetime;
    if (timestamps) {
        datetime = "%datetime{%Y-%M-%dT%H:%m:%s.%g}";
    }
    std::string shortFmt = datetime + " %level [%logger] %msg";
    std::string longFmt = shortFmt + " [%fbase:%line]";

    gDefaultConf.setGlobally(el::ConfigurationType::Format, shortFmt);
    gDefaultConf.set(el::Level::Error, el::ConfigurationType::Format, longFmt);
    gDefaultConf.set(el::Level::Trace, el::ConfigurationType::Format, longFmt);
    gDefaultConf.set(el::Level::Fatal, el::ConfigurationType::Format, longFmt);
    el::Loggers::reconfigureAllLoggers(gDefaultConf);
}

void Logging::init() {
    el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);
    el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
    el::Loggers::addFlag(el::LoggingFlag::MultiLoggerSupport);
    el::Loggers::addFlag(el::LoggingFlag::CreateLoggerAutomatically);

    el::Loggers::getLogger(LOG_TIMESPAN);
    el::Loggers::getLogger(LOG_INIT);
    el::Loggers::getLogger(LOG_OUTPUT);
    el::Loggers::getLogger(LOG_RELEASE);

    gDefaultConf.setToDefault();
    gDefaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
    gDefaultConf.setGlobally(el::ConfigurationType::ToFile, "false");
    gDefaultConf.setGlobally(el::ConfigurationType::LogFlushThreshold, "100");
    setFmt();
}

void Logging::setLoggingToFile(std::string const& filename) {
    gDefaultConf.setGlobally(el::ConfigurationType::ToFile, "true");
    gDefaultConf.setGlobally(el::ConfigurationType::Filename, filename);
    el::Loggers::reconfigureAllLoggers(gDefaultConf);
}

el::Level Logging::getLogLevel(std::string const& partition) {
    el::Logger* logger = el::Loggers::getLogger(partition);
    if (logger->typedConfigurations()->enabled(el::Level::Trace))
        return el::Level::Trace;
    if (logger->typedConfigurations()->enabled(el::Level::Debug))
        return el::Level::Debug;
    if (logger->typedConfigurations()->enabled(el::Level::Info))
        return el::Level::Info;
    if (logger->typedConfigurations()->enabled(el::Level::Warning))
        return el::Level::Warning;
    if (logger->typedConfigurations()->enabled(el::Level::Error))
        return el::Level::Error;
    if (logger->typedConfigurations()->enabled(el::Level::Fatal))
        return el::Level::Fatal;
    return el::Level::Unknown;
}

bool Logging::logDebug(std::string const& partition) {
    auto lev = Logging::getLogLevel(partition);
    return lev == el::Level::Debug || lev == el::Level::Trace;
}

bool Logging::logTrace(std::string const& partition) {
    auto lev = Logging::getLogLevel(partition);
    return lev == el::Level::Trace;
}

// Trace < Debug < Info < Warning < Error < Fatal < Unknown
// Detailly,
//   Trace: Log everything
//   Debug: Log everything except Trace
//   Info: Not include Trace and Debug
//   ...
void Logging::setLogLevel(el::Level level, const char* partition) {
    el::Configurations config = gDefaultConf;

    if (level == el::Level::Debug)
        config.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
    else if (level == el::Level::Info) {
        config.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    } else if (level == el::Level::Warning) {
        config.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
    } else if (level == el::Level::Error) {
        config.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Warning, el::ConfigurationType::Enabled, "false");
    } else if (level == el::Level::Fatal) {
        config.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Warning, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Error, el::ConfigurationType::Enabled, "false");
    } else if (level == el::Level::Unknown) {
        config.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Warning, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Error, el::ConfigurationType::Enabled, "false");
        config.set(el::Level::Fatal, el::ConfigurationType::Enabled, "false");
    }

    if (partition)
        el::Loggers::reconfigureLogger(partition, config);
    else
        el::Loggers::reconfigureAllLoggers(config);
}

std::string Logging::getStringFromLL(el::Level level) {
    switch (level) {
        case el::Level::Global:
            return "Global";
        case el::Level::Trace:
            return "Trace";
        case el::Level::Debug:
            return "Debug";
        case el::Level::Fatal:
            return "Fatal";
        case el::Level::Error:
            return "Error";
        case el::Level::Warning:
            return "Warning";
        case el::Level::Verbose:
            return "Verbose";
        case el::Level::Info:
            return "Info";
        case el::Level::Unknown:
            return "Unknown";
    }
    return "????";
}

// default "info" if unrecognized
el::Level Logging::getLLfromString(std::string const& levelName) {
    if (StringMatch(levelName, "trace")) {
        return el::Level::Trace;
    }

    if (StringMatch(levelName, "debug")) {
        return el::Level::Debug;
    }

    if (StringMatch(levelName, "warning")) {
        return el::Level::Warning;
    }

    if (StringMatch(levelName, "fatal")) {
        return el::Level::Fatal;
    }

    if (StringMatch(levelName, "error")) {
        return el::Level::Error;
    }

    if (StringMatch(levelName, "none")) {
        return el::Level::Unknown;
    }

    return el::Level::Info;
}

void Logging::rotate() {
    el::Loggers::getLogger(LOG_DEFAULT)->reconfigure();
    el::Loggers::getLogger(LOG_TIMESPAN)->reconfigure();
    el::Loggers::getLogger(LOG_INIT)->reconfigure();
    el::Loggers::getLogger(LOG_OUTPUT)->reconfigure();
    el::Loggers::getLogger(LOG_RELEASE)->reconfigure();
}
