//
// Created by Zheng on 2018/4/10.
//

#import <string>
#import <fstream>
#import <sstream>

#import <stdio.h>
#import <string.h>
#import <stdlib.h>

#import "ZeldaHTTPHelper.h"
#import "base64.hpp"


std::string ZeldaHTTPHelper::methodStringFromHeaderMap(std::map<std::string, std::string> hmap)
{

    std::string hhdrStr = hmap["_"];
    std::string::size_type pos = hhdrStr.find(' ');
    if (pos != std::string::npos)
        return hhdrStr.substr(0, pos);
    return "";

}

void ZeldaHTTPHelper::copyHeaderDataFromHeaderMap(char **dst, size_t *len, std::map<std::string, std::string> hmap)
{

    if (!dst || !len) return;

    std::string hhdrStr = hmap["_"] + "\r\n";
    hmap.erase("_");

    for (auto &it : hmap)
        hhdrStr += it.first + ": " + it.second + "\r\n";

    hhdrStr += "\r\n";

    *len = hhdrStr.size(); // copy without '\0'
    *dst = (char *)malloc(*len);
    memcpy(*dst, hhdrStr.c_str(), *len);

}

std::map<std::string, std::string> ZeldaHTTPHelper::headerMapFromHeaderData(const char *data, size_t len)
{

    auto hmap = std::map<std::string, std::string>();

    size_t lineStart = 0;
    size_t lineEnd = 0;
    size_t sepLoc = 0;
    int isFirstLine = 0;

    for (size_t i = 0; i < len; ++i)
    {

        char ch = *(data + i);

        if (sepLoc == 0 && ch == ' ')
        {
            isFirstLine = 1;
            continue;
        }
        else if (ch == '\r')
        {
            lineEnd = i;
        }
        else if (ch == '\n')
        {
            lineStart = i + 1;
            lineEnd = 0;
            sepLoc = 0;
            isFirstLine = 0;
            continue;
        }

        if (isFirstLine == 0)
        {
            if (sepLoc == 0 && ch == ':')
            {
                sepLoc = i;
            }
        }

        if (lineEnd != 0 && sepLoc != 0 &&
            sepLoc - lineStart > 0 &&
            lineEnd - sepLoc > 0) {

            char hkey[BUFSIZ];
            size_t sepLen = sepLoc - lineStart;
            strncpy(hkey, data + lineStart, sepLen);
            hkey[sepLen] = '\0';

            size_t spaceLoc = sepLoc + 2;
            char hval[BUFSIZ];
            size_t valLen = lineEnd - spaceLoc;
            strncpy(hval, data + spaceLoc, valLen);
            hval[valLen] = '\0';

            std::string hkeyStr(hkey);
            std::string hValStr(hval);

            hmap[hkeyStr] = hValStr;

        }
        else if (lineEnd != 0 && isFirstLine)
        {

            char hhdr[BUFSIZ];
            size_t hdrLen = lineEnd - lineStart;
            strncpy(hhdr, data + lineStart, hdrLen);
            hhdr[hdrLen] = '\0';
            std::string hhdrStr(hhdr);
            hmap["_"] = hhdrStr;

        }

    }

    return hmap;
}

std::list<std::string> ZeldaHTTPHelper::authenticationListAtPath(const char *path) {
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

std::string ZeldaHTTPHelper::getGMTDateString() {
    char buf[BUFSIZ];
    time_t now = time(nullptr);
    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return std::string(buf);
}
