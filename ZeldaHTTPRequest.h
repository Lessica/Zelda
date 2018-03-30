//
// Created by Zheng Wu on 2018/3/16.
//

#import "ZeldaProtocol.h"

class ZeldaHTTPRequest: public ZeldaProtocol {
public:
    ZeldaHTTPRequest();
    ~ZeldaHTTPRequest() override;
    void processChuck(char **inOut, size_t len, size_t *newLen) override;
};
