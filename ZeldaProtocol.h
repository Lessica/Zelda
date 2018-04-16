//
// Created by Zheng on 18/03/2018.
//

#import "ZeldaLogger.h"

class ZeldaProtocol {

public:
    ZeldaProtocol();
    virtual ~ZeldaProtocol();

    virtual void processChuck(char **inOut, size_t *len);
    virtual std::string description();

    bool isActive();
    bool isHandled();
    bool shouldKeepAlive();

#pragma mark - Loggers

    ZeldaLogger *GetLogger();
    void SetLogger(ZeldaLogger *logger);

#pragma mark - Remote

    virtual std::string GetRemoteAddress();
    virtual int GetRemotePort();

protected:

#pragma mark - Logger

    ZeldaLogger *Log = nullptr;

#pragma mark - Buffer

    size_t packetCount = 0;
    size_t inBytes = 0;
    size_t outBytes = 0;

#pragma mark - Flags

    bool active = true;
    bool keepAlive = false;
    bool handled = false;

private:

    void processStartLine(char **inOut, size_t *len);

};
