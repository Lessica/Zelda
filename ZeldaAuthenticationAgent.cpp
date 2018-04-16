//
// Created by Zheng on 2018/4/16.
//

#include <algorithm>

#import "ZeldaAuthenticationAgent.h"

ZeldaAuthenticationAgent::ZeldaAuthenticationAgent() = default;
ZeldaAuthenticationAgent::~ZeldaAuthenticationAgent() = default;

bool ZeldaAuthenticationAgent::isCipherAccepted(std::string cipher)
{
    if (authenticationList.empty()) return false;
    auto findIter = std::find(authenticationList.begin(), authenticationList.end(), cipher);
    if (authenticationList.end() == findIter) {
        return false;
    }
    return true;
}
