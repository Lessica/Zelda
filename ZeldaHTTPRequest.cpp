//
// Created by Zheng Wu on 2018/3/16.
//

#import "ZeldaHTTPRequest.h"

ZeldaHTTPRequest::ZeldaHTTPRequest() : ZeldaProtocol() {

}

ZeldaHTTPRequest::~ZeldaHTTPRequest() = default;

void ZeldaHTTPRequest::processChuck(char **inOut, size_t len, size_t *newLen) {
    ZeldaProtocol::processChuck(inOut, len, newLen);
    this->packetCount++;
}
