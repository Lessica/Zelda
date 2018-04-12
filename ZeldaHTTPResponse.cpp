//
// Created by Zheng Wu on 2018/3/16.
//

#import "ZeldaHTTPResponse.h"
#include "ZeldaDefines.h"
#include "ZeldaHTTPHelper.h"

ZeldaHTTPResponse::ZeldaHTTPResponse() : ZeldaProtocol() {

}

ZeldaHTTPResponse::~ZeldaHTTPResponse() = default;

void ZeldaHTTPResponse::processChuck(char **inOut, size_t *len) {
    if (inOut == nullptr || len == nullptr)
        return;
    this->inBytes += *len;
    ZeldaProtocol::processChuck(inOut, len);
    if (this->packetCount == 0) {
        this->processResponseHeader(inOut, len);
    }
    this->packetCount++;
    this->outBytes += *len;
}

void ZeldaHTTPResponse::processResponseHeader(char **inOut, size_t *len) {
    auto *buffer = static_cast<const char *>(*inOut);

    if (buffer == nullptr)
        return;

    auto totalLen = *len;

    size_t headerLen = 0;
    for (size_t i = 0; i < totalLen - 3; ++i) {
        if (strncmp(buffer + i,"\r\n\r\n", 4) == 0) {
            headerLen = i + 4;
            break;
        }
    }

    size_t bodyPos = headerLen;
    size_t bodyLen = totalLen - bodyPos;

    auto hmap = ZeldaHTTPHelper::headerMapFromHeaderData(buffer, headerLen);
    hmap["X-Proxy-Agent"] = std::string(ZELDA_NAME) + "/" + ZELDA_VERSION;
    Log->Debug(this->description() + hmap["_"]);

    // transform connection field to lower case
    std::string connectionField = hmap["Connection"];
    std::transform(connectionField.begin(), connectionField.end(), connectionField.begin(), ::tolower);
    if (connectionField == "keep-alive") {
        keepAlive = true;
        active = true;
    } else if (connectionField == "close") {
        keepAlive = false;
        active = false;
    }

    char *newHeaderBuf = nullptr;
    size_t newHeaderLen = 0;
    ZeldaHTTPHelper::copyHeaderDataFromHeaderMap(&newHeaderBuf, &newHeaderLen, hmap);

    size_t newLen = newHeaderLen + bodyLen;
    auto *newBuf = (char *)malloc(newLen);
    memcpy(newBuf, newHeaderBuf, newHeaderLen);
    memcpy(newBuf + newHeaderLen, buffer + bodyPos, bodyLen);

    delete(newHeaderBuf);
    delete(*inOut);

    *inOut = newBuf;
    *len = newLen;
}

std::string ZeldaHTTPResponse::description() {
    return std::string("[<] ");
}
