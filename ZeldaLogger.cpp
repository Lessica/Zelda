//
// Created by Zheng on 05/03/2018.
//

#import <iostream>
#import <ctime>

#import <stdio.h>
#import <string.h>
#import <stdlib.h>

#import "ZeldaLogger.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_REDB    "\x1b[91m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

ZeldaLogLevel ZeldaLogger::LogLevelFromLevelString(const std::string &level) {
    if (level == ZELDA_LOG_FATAL) {
        return ZeldaLogLevelFatal;
    } else if (level == ZELDA_LOG_ERROR) {
        return ZeldaLogLevelError;
    } else if (level == ZELDA_LOG_WARNING) {
        return ZeldaLogLevelWarning;
    } else if (level == ZELDA_LOG_INFO) {
        return ZeldaLogLevelInfo;
    } else if (level == ZELDA_LOG_DEBUG) {
        return ZeldaLogLevelDebug;
    }
    return ZeldaLogLevelInfo;
};

void ZeldaLogger::SetLogLevel(const std::string &level) {
    _level = ZeldaLogger::LogLevelFromLevelString(level);
}

void ZeldaLogger::Fatal(const std::string &msg) {
    if (_level >= ZeldaLogLevelFatal) {
        ZeldaLogger::Err(ZELDA_LOG_FATAL, msg);
    }
    exit(1); // fatal exit
}

void ZeldaLogger::Error(const std::string &msg) {
    if (_level >= ZeldaLogLevelError) {
        ZeldaLogger::Err(ZELDA_LOG_ERROR, msg);
    }
}

void ZeldaLogger::Warning(const std::string &msg) {
    if (_level >= ZeldaLogLevelWarning) {
        ZeldaLogger::Log(ZELDA_LOG_WARNING, msg);
    }
}

void ZeldaLogger::Info(const std::string &msg) {
    if (_level >= ZeldaLogLevelInfo) {
        ZeldaLogger::Log(ZELDA_LOG_INFO, msg);
    }
}

void ZeldaLogger::Debug(const std::string &msg) {
    if (_level >= ZeldaLogLevelDebug) {
        ZeldaLogger::Log(ZELDA_LOG_DEBUG, msg);
    }
}

void ZeldaLogger::Flush() {
    std::flush(std::cout);
}

void ZeldaLogger::Log(const std::string &level, const std::string &msg) {
    std::cout << GetCurrentTimeString() << ColorStringFromLevel(level) << " [" << level << "] " <<  msg << ColorStringFromLevel("") << std::endl;
}

void ZeldaLogger::Err(const std::string &level, const std::string &msg) {
    std::cerr << GetCurrentTimeString() << ColorStringFromLevel(level) << " [" << level << "] " <<  msg << ColorStringFromLevel("") << std::endl;
}

std::string ZeldaLogger::GetCurrentTimeString() {
    time_t t;
    time(&t);
    struct tm *now = localtime(&t);
    auto *p = (char *)malloc(32);
    strftime(p, 32, "%x %X", now);
    return ANSI_COLOR_CYAN + std::string(p) + ANSI_COLOR_RESET;
}

std::string ZeldaLogger::ColorStringFromLevel(const std::string &level) {
    if (!isColoredTerminal) return "";
    if (level == ZELDA_LOG_ERROR) {
        return ANSI_COLOR_RED;
    } else if (level == ZELDA_LOG_WARNING) {
        return ANSI_COLOR_YELLOW;
    } else if (level == ZELDA_LOG_FATAL) {
        return ANSI_COLOR_REDB;
    } else if (level == ZELDA_LOG_INFO) {
        return ANSI_COLOR_BLUE;
    } else if (level == ZELDA_LOG_DEBUG) {
        return ANSI_COLOR_WHITE;
    }
    return ANSI_COLOR_RESET;
}

std::string ZeldaLogger::S() {
    return std::string();
}

ZeldaLogger::ZeldaLogger(const std::string &level) {
    _level = ZeldaLogger::LogLevelFromLevelString(level);
    const char *term = getenv("TERM");
    if (term != nullptr) isColoredTerminal = (strstr(term, "xterm") != nullptr);
}

ZeldaLogger::ZeldaLogger() : ZeldaLogger("info") {

}

void ZeldaLogger::Fatal(const char *msg) {
    Fatal(S() + msg);
}

void ZeldaLogger::Error(const char *msg) {
    Error(S() + msg);
}

void ZeldaLogger::Warning(const char *msg) {
    Warning(S() + msg);
}

void ZeldaLogger::Info(const char *msg) {
    Info(S() + msg);
}

void ZeldaLogger::Debug(const char *msg) {
    Debug(S() + msg);
}
