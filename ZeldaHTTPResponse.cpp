//
// Created by Zheng Wu on 2018/3/16.
//

#import <stdio.h>
#import <string.h>
#import <stdlib.h>
#import <algorithm>

#import "ZeldaHTTPResponse.h"
#import "ZeldaDefines.h"
#import "ZeldaHTTPHelper.h"

ZeldaHTTPResponse::ZeldaHTTPResponse() : ZeldaProtocol() {

}

ZeldaHTTPResponse::~ZeldaHTTPResponse() = default;

void ZeldaHTTPResponse::processChuck(char **inOut, size_t *len)
{

    if (inOut == nullptr || len == nullptr)
        return;

    this->inBytes += *len;
    ZeldaProtocol::processChuck(inOut, len);

    if (this->packetCount == 0)
        this->processResponseHeader(inOut, len);

    this->packetCount++;
    this->outBytes += *len;

}

void ZeldaHTTPResponse::processResponseHeader(char **inOut, size_t *len)
{

    auto *buffer = static_cast<const char *>(*inOut);

    if (buffer == nullptr)
        return;

    auto totalLen = *len;

    size_t headerLen = 0;
    for (size_t i = 0; i < totalLen - 3; ++i)
    {
        if (strncmp(buffer + i,"\r\n\r\n", 4) == 0)
        {
            headerLen = i + 4;
            break;
        }
    }

    size_t bodyPos = headerLen;
    size_t bodyLen = totalLen - bodyPos;

    auto hmap = ZeldaHTTPHelper::headerMapFromHeaderData(buffer, headerLen, false);
    hmap["Proxy-Agent"] = std::string(ZELDA_NAME) + "/" + ZELDA_VERSION;
    Log->Debug(this->description() + hmap["_"]);

    // transform connection field to lower case
    std::string connectionField = hmap["Connection"];
    std::transform(connectionField.begin(), connectionField.end(), connectionField.begin(), ::tolower);
    if (connectionField == "keep-alive")
    {
        keepAlive = true;
        active = true;
    }
    else if (connectionField == "close")
    {
        keepAlive = false;
        active = false;
    }

    hmap.erase("Connection");
    if (keepAlive) {
        hmap["Connection"] = "Keep-Alive";
    } else {
        hmap["Connection"] = "Close";
    }


    char *newHeaderBuf = nullptr;
    size_t newHeaderLen = 0;
    ZeldaHTTPHelper::copyHeaderDataFromHeaderMap(&newHeaderBuf, &newHeaderLen, hmap);

    size_t newLen = newHeaderLen + bodyLen;
    auto *newBuf = (char *)malloc(newLen);
    memcpy(newBuf, newHeaderBuf, newHeaderLen);
    memcpy(newBuf + newHeaderLen, buffer + bodyPos, bodyLen);

    free(newHeaderBuf);
    free(*inOut);

    *inOut = newBuf;
    *len = newLen;

}

std::string ZeldaHTTPResponse::description() {
    return std::string("[<] ");
}
