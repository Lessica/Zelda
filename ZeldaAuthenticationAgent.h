//
// Created by Zheng on 2018/4/16.
//

#import <string>
#import <list>

class ZeldaAuthenticationAgent {

public:
    ZeldaAuthenticationAgent();
    ~ZeldaAuthenticationAgent();

    std::list<std::string> authenticationList = std::list<std::string>();
    bool isCipherAccepted(std::string cipher);

};
