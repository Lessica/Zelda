//
// Created by Zheng on 18/03/2018.
//

#import "ZeldaProtocol.h"

ZeldaProtocol::ZeldaProtocol() = default;
ZeldaProtocol::~ZeldaProtocol() = default;

void ZeldaProtocol::processChuck(char **inOut, size_t len, size_t *newLen) {
    if (packetCount == 0) {
        this->processStartLine(inOut, len, newLen);
    }
}

std::string ZeldaProtocol::description() {
    return std::string();
}

ZeldaLogger *ZeldaProtocol::GetLogger() {
    return Log;
}

void ZeldaProtocol::SetLogger(ZeldaLogger *logger) {
    Log = logger;
}

void ZeldaProtocol::processStartLine(char **inOut, size_t len, size_t *newLen) {
    auto *buffer = static_cast<const char *>(*inOut);

    size_t lineLen = 0;
    for (size_t i = 0; i < len; ++i) {
        if (buffer[i] == '\r') {
            lineLen = i;
            break;
        }
    }

    auto *line = (char *)malloc(lineLen + 1); // + string terminator
    memcpy(line, buffer, lineLen);
    line[lineLen] = '\0';

    Log->Info(line);
    delete(line);
}
