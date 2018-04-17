//
// Created by Zheng on 18/03/2018.
//

#import <stdio.h>
#import <string.h>
#import <stdlib.h>

#import "ZeldaProtocol.h"

ZeldaProtocol::ZeldaProtocol() = default;
ZeldaProtocol::~ZeldaProtocol() = default;

void ZeldaProtocol::processChuck(char **inOut, size_t *len) {
    if (packetCount == 0) {
        // this->processStartLine(inOut, len);
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

void ZeldaProtocol::processStartLine(char **inOut, size_t *len) {
    if (inOut == nullptr || len == nullptr)
        return;

    auto *buffer = static_cast<const char *>(*inOut);

    if (buffer == nullptr)
        return;

    auto totalLen = *len;

    size_t lineLen = 0;
    for (size_t i = 0; i < totalLen; ++i) {
        if (buffer[i] == '\r') {
            lineLen = i;
            break;
        }
    }

    char line[lineLen + 1]; // + string terminator
    memcpy(line, buffer, lineLen);
    line[lineLen] = '\0';

    Log->Info(line);
}

std::string ZeldaProtocol::GetRemoteAddress() {
    return std::string();
}

int ZeldaProtocol::GetRemotePort() {
    return 0;
}

bool ZeldaProtocol::isActive() {
    return active;
}

bool ZeldaProtocol::isHandled() {
    return handled;
}

bool ZeldaProtocol::shouldKeepAlive() {
    return keepAlive;
}
