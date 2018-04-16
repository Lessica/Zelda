//
// Created by Zheng on 2018/4/15.
//

#import <map>
#import "ZeldaHTTPTunnel.h"
#import "ZeldaDefines.h"
#import "ZeldaHTTPHelper.h"

ZeldaHTTPTunnel::ZeldaHTTPTunnel() : ZeldaHTTPRequest() {

}

ZeldaHTTPTunnel::~ZeldaHTTPTunnel() = default;

void ZeldaHTTPTunnel::processChuck(char **inOut, size_t *len) {
    ZeldaHTTPRequest::processChuck(inOut, len);

    // TODO: Proxy Authentication
    // 401 Unauthorized
    // 407 Proxy Authentication Required

    if (httpMethod == "CONNECT")
    {

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

        auto hmap = std::map<std::string, std::string>();
        hmap["_"] = "HTTP/1.1 200 Connection Established";
        hmap["Proxy-Agent"] = std::string(ZELDA_NAME) + "/" + ZELDA_VERSION;
        Log->Debug(this->description() + hmap["_"]);

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

        handled = true;

    }

}

std::string ZeldaHTTPTunnel::description() {
    return std::string("[@] ");
}
