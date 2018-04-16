//
// Created by Zheng Wu on 2018/3/16.
//

#import "ZeldaProtocol.h"

class ZeldaHTTPResponse: public ZeldaProtocol {
public:
    ZeldaHTTPResponse();
    ~ZeldaHTTPResponse() override;
    void processChuck(char **inOut, size_t *len) override;
    std::string description() override;

    int httpStatus = 0;

private:
    void processResponseHeader(char **inOut, size_t *len);
};
