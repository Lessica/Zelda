//
// Created by Zheng Wu on 2018/3/16.
//

#import <csignal>
#import <algorithm>

#import <stdio.h>
#import <string.h>
#import <stdlib.h>
#import <unistd.h>
#import <sys/types.h>

#import "ZeldaHTTPRequest.h"
#import "ZeldaHTTPHelper.h"

ZeldaHTTPRequest::ZeldaHTTPRequest() : ZeldaProtocol() {

}

ZeldaHTTPRequest::~ZeldaHTTPRequest() = default;

void ZeldaHTTPRequest::processChuck(char **inOut, size_t *len)
{

    if (inOut == nullptr || len == nullptr)
        return;

    this->inBytes += *len;
    ZeldaProtocol::processChuck(inOut, len);

    if (this->packetCount == 0)
        this->processRequestHeader(inOut, len);

    this->packetCount++;
    this->outBytes += *len;

}

void ZeldaHTTPRequest::processRequestHeader(char **inOut, size_t *len)
{

    auto *buffer = static_cast<const char *>(*inOut);

    if (buffer == nullptr)
        return;

    auto totalLen = *len;

    auto hmap = ZeldaHTTPHelper::headerMapFromHeaderData(buffer, totalLen);
    this->processRequestHost(hmap["Host"]);
    hmap["Connection"] = hmap["Proxy-Connection"];
    hmap.erase("Proxy-Connection"); // fix/rename connection field
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

    httpMethod = ZeldaHTTPHelper::methodStringFromHeaderMap(hmap);

}

void ZeldaHTTPRequest::processRequestHost(std::string hostString) {

    const char *host = hostString.c_str();
    size_t hostLen = hostString.size();
    char hostPointer[hostLen];
    memcpy(hostPointer, host, hostLen);
    hostPointer[hostLen] = '\0';
    const char *portPointer = nullptr;
    char *mid = strstr(hostPointer, ":");
    if (mid)
    {
        *mid = '\0';
        portPointer = mid + 1; size_t sz;
        _requestPort = std::stoi(portPointer, &sz);
    }
    _requestAddress = std::string(hostPointer);

}

std::string ZeldaHTTPRequest::GetRemoteAddress() {
    return _requestAddress;
}

int ZeldaHTTPRequest::GetRemotePort() {
    return _requestPort;
}

std::string ZeldaHTTPRequest::description() {
    return std::string("[>] ");
}
