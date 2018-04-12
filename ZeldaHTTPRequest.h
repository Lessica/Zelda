//
// Created by Zheng Wu on 2018/3/16.
//

#import "ZeldaProtocol.h"

class ZeldaHTTPRequest: public ZeldaProtocol {
public:
    ZeldaHTTPRequest();
    ~ZeldaHTTPRequest() override;
    void processChuck(char **inOut, size_t *len) override;

    std::string description() override;

    std::string GetRemoteAddress() override;
    int GetRemotePort() override;

private:
    std::string _requestAddress;
    int _requestPort = 80;

    void processRequestHeader(char **inOut, size_t *len);
    void processRequestHost(std::string host);
};
