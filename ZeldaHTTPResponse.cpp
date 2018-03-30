//
// Created by Zheng Wu on 2018/3/16.
//

#import "ZeldaHTTPResponse.h"
#include "ZeldaDefines.h"

ZeldaHTTPResponse::ZeldaHTTPResponse() : ZeldaProtocol() {

}

ZeldaHTTPResponse::~ZeldaHTTPResponse() = default;

void ZeldaHTTPResponse::processChuck(char **inOut, size_t len, size_t *newLen) {
    ZeldaProtocol::processChuck(inOut, len, newLen);
    if (this->packetCount == 0) {
        this->processResponseHeader(inOut, len, newLen);
    }
    this->packetCount++;
}

void ZeldaHTTPResponse::processResponseHeader(char **inOut, size_t len, size_t *newLen) {
    auto *buffer = static_cast<const char *>(*inOut);

    size_t headerLen = 0;
    for (size_t i = 0; i < len - 3; ++i) {
        if (strncmp(buffer + i,"\r\n\r\n", 4) == 0) {
            headerLen = i;
            break;
        }
    }

    std::string addition = this->additionalHeaderFields();
    size_t additionLen = addition.size();

    size_t newPos = 0;
    auto *newBuf = (char *)malloc(len + additionLen + 2);
    memcpy(newBuf + newPos, buffer, headerLen); newPos += headerLen;
    memcpy(newBuf + newPos, "\r\n", 2); newPos += 2;
    memcpy(newBuf + newPos, addition.c_str(), additionLen); newPos += additionLen;
    memcpy(newBuf + newPos, buffer + headerLen, len - headerLen); newPos += len - headerLen;

    delete(*inOut);
    *inOut = newBuf;
    *newLen = newPos;

}

std::string ZeldaHTTPResponse::additionalHeaderFields() {
    return std::string("X-Forwarded-By: ") + ZELDA_NAME + "/" + ZELDA_VERSION;
}
