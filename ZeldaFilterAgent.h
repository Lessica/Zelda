//
// Created by Zheng on 2018/4/17.
//

#import <string>
#import <list>

class ZeldaFilterAgent {

public:
    ZeldaFilterAgent();
    ~ZeldaFilterAgent();

    std::list<std::string> blacklist = std::list<std::string>();
    std::list<std::string> whitelist = std::list<std::string>();

    bool blacklistMode = true;
    bool isHostAccepted(std::string host);

    static std::list<std::string> listAtPath(const char *path);

};
