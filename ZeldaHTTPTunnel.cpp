//
// Created by Zheng on 2018/4/15.
//

#import <map>
#import <sstream>
#import <algorithm>

#import <stdio.h>
#import <string.h>
#import <stdlib.h>

#import "ZeldaHTTPTunnel.h"
#import "ZeldaDefines.h"
#import "ZeldaHTTPHelper.h"

ZeldaHTTPTunnel::ZeldaHTTPTunnel() : ZeldaHTTPRequest() {

}

ZeldaHTTPTunnel::~ZeldaHTTPTunnel() = default;

void ZeldaHTTPTunnel::processChuck(char **inOut, size_t *len) {
    ZeldaHTTPRequest::processChuck(inOut, len);

    // TODO: Proxy Authentication
    // 401 Unauthorized ?
    // 403 Forbidden
    // 407 Proxy Authentication Required
    // RESP: Proxy-Authenticate: Basic
    // REQU: Proxy-Authorization: Basic YWxhZGRpbjpvcGVuc2VzYW1l

    if (packetCount != 1) return;

    auto *buffer = static_cast<const char *>(*inOut);

    if (buffer == nullptr)
        return;

    auto totalLen = *len;
    auto newmap = std::map<std::string, std::string>();

    if (!newmap.count("_"))
    {
        if (authenticationAgent != nullptr)
        {
            auto hmap = ZeldaHTTPHelper::headerMapFromHeaderData(buffer, totalLen);
            if (!hmap.count("Proxy-Authorization"))
            {
                newmap["_"] = "HTTP/1.1 407 Proxy Authentication Required";
                newmap["Proxy-Authenticate"] = "Basic";
                newmap["Connection"] = "Close";
                active = false;
            }
            else
            {
                std::string method;
                std::string cipher;
                std::string authField = hmap["Proxy-Authorization"];
                std::istringstream iss(authField);
                iss >> method >> cipher;
                std::transform(method.begin(), method.end(), method.begin(), ::tolower);

                bool accepted = false;
                if (method == "basic")
                {
                    accepted = authenticationAgent->isCipherAccepted(cipher);
                }
                if (!accepted)
                {
                    newmap["_"] = "HTTP/1.1 403 Forbidden";
                    newmap["Connection"] = "Close";
                    active = false;

                    Log->Warning(this->description() + "Proxy authentication failed");
                }
            }
        }
    }

    if (!newmap.count("_"))
    {
        if (httpMethod == "CONNECT")
        {
            newmap["_"] = "HTTP/1.1 200 Connection Established";
            active = true;
        }
    }

    if (newmap.count("_")) {
        Log->Debug(this->description() + newmap["_"]);
        newmap["Proxy-Agent"] = std::string(ZELDA_NAME) + "/" + ZELDA_VERSION;

        size_t headerLen = 0;
        for (size_t i = 0; i < totalLen - 3; ++i) {
            if (strncmp(buffer + i,"\r\n\r\n", 4) == 0) {
                headerLen = i + 4;
                break;
            }
        }

        size_t bodyPos = headerLen;
        size_t bodyLen = totalLen - bodyPos;

        char *newHeaderBuf = nullptr;
        size_t newHeaderLen = 0;
        ZeldaHTTPHelper::copyHeaderDataFromHeaderMap(&newHeaderBuf, &newHeaderLen, newmap);

        size_t newLen = newHeaderLen + bodyLen;
        auto *newBuf = (char *)malloc(newLen);
        memcpy(newBuf, newHeaderBuf, newHeaderLen);
        memcpy(newBuf + newHeaderLen, buffer + bodyPos, bodyLen);

        free(newHeaderBuf);
        free(*inOut);

        *inOut = newBuf;
        *len = newLen;

        handled = true;
    }

}

std::string ZeldaHTTPTunnel::description() {
    return std::string("[@] ");
}

#pragma mark - Setters

void ZeldaHTTPTunnel::SetAuthenticationAgent(ZeldaAuthenticationAgent *agent) {
    authenticationAgent = agent;
}
