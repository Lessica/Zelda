//
// Created by Zheng on 2018/4/16.
//

#import <algorithm>
#import <fstream>
#import <sstream>

#import <stdio.h>
#import <string.h>
#import <stdlib.h>
#import "base64.hpp"

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

std::list<std::string> ZeldaAuthenticationAgent::authenticationListAtPath(const char *path) {
    std::ifstream infile(path);
    if (!infile.is_open()) {
        return std::list<std::string>();
    }
    auto list = std::list<std::string>();
    std::string line;
    std::string username;
    std::string password;
    while (infile >> username >> password) {
        std::string out;
        std::string newLine = username;
        newLine += ":";
        newLine += password;
        Base64::Encode(newLine, &out);
        list.push_back(out);
    }
    infile.close();
    return list;
}
