//
// Created by Zheng on 2018/4/17.
//

#import <algorithm>
#import <fstream>
#import <sstream>

#import "ZeldaFilterAgent.h"

ZeldaFilterAgent::ZeldaFilterAgent() = default;

ZeldaFilterAgent::~ZeldaFilterAgent() = default;

bool ZeldaFilterAgent::isHostAccepted(std::string host) {
    if (blacklist.empty() && whitelist.empty()) return false;
    if (blacklistMode) {
        auto findIter = std::find(blacklist.begin(), blacklist.end(), host);
        if (blacklist.end() == findIter) {
            return true;
        }
        return false;
    } else {
        auto findIter = std::find(whitelist.begin(), whitelist.end(), host);
        if (whitelist.end() == findIter) {
            return false;
        }
        return true;
    }
}

std::list<std::string> ZeldaFilterAgent::listAtPath(const char *path) {
    std::ifstream infile(path);
    if (!infile.is_open()) {
        return std::list<std::string>();
    }
    auto list = std::list<std::string>();
    std::string line;
    std::string host;
    while (infile >> host) {
        list.push_back(host);
    }
    infile.close();
    return list;
}
