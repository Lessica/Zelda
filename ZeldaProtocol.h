//
// Created by Zheng on 18/03/2018.
//

#import "ZeldaLogger.h"

class ZeldaProtocol {

public:
    ZeldaProtocol();
    virtual ~ZeldaProtocol();

    virtual void processChuck(char **inOut, size_t len, size_t *newLen);
    virtual std::string description();

#pragma mark - Loggers

    ZeldaLogger *GetLogger();
    void SetLogger(ZeldaLogger *logger);

protected:

#pragma mark - Logger

    ZeldaLogger *Log = nullptr;

#pragma mark - Buffer

    size_t packetCount = 0;

private:

    void processStartLine(char **inOut, size_t len, size_t *newLen);

};
