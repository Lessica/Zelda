//
// Created by Zheng on 05/03/2018.
//

#import <string>

#define ZELDA_LOG_FATAL "fatal"
#define ZELDA_LOG_ERROR "error"
#define ZELDA_LOG_WARNING "warning"
#define ZELDA_LOG_INFO "info"
#define ZELDA_LOG_DEBUG "debug"

typedef enum {
    ZeldaLogLevelFatal = 0,
    ZeldaLogLevelError,
    ZeldaLogLevelWarning,
    ZeldaLogLevelInfo,
    ZeldaLogLevelDebug
} ZeldaLogLevel;

class ZeldaLogger {

public:

    ZeldaLogger();
    explicit ZeldaLogger(const std::string &level);

    void SetLogLevel(const std::string &level);
    void Fatal(const std::string &msg);
    void Fatal(const char *msg);
    void Error(const std::string &msg);
    void Error(const char *msg);
    void Warning(const std::string &msg);
    void Warning(const char *msg);
    void Info(const std::string &msg);
    void Info(const char *msg);
    void Debug(const std::string &msg);
    void Debug(const char *msg);
    void Flush();

private:

    ZeldaLogLevel _level;
    ZeldaLogLevel LogLevelFromLevelString(const std::string &level);

    void Log(const std::string &level, const std::string &msg);
    void Err(const std::string &level, const std::string &msg);

    std::string GetCurrentTimeString();
    std::string S();

};
