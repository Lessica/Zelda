//
// Created by Zheng on 2018/4/15.
//

#import "ZeldaHTTPRequest.h"

class ZeldaHTTPTunnel: public ZeldaHTTPRequest {
public:
    ZeldaHTTPTunnel();
    ~ZeldaHTTPTunnel() override;
    void processChuck(char **inOut, size_t *len) override;

    std::string description() override;
};
