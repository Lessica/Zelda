//
// Created by Zheng on 2018/4/10.
//

#include <string>
#include "ZeldaHTTPHelper.h"


std::string ZeldaHTTPHelper::methodStringFromHeaderMap(std::map<std::string, std::string> hmap) {
    std::string hhdrStr = hmap["_"];
    std::string::size_type pos = hhdrStr.find(' ');
    if (pos != std::string::npos) {
        return hhdrStr.substr(0, pos);
    }
    return "";
}

void ZeldaHTTPHelper::copyHeaderDataFromHeaderMap(char **dst, size_t *len, std::map<std::string, std::string> hmap) {
    if (!dst || !len) return;

    std::string hhdrStr = hmap["_"] + "\r\n";
    hmap.erase("_");

    for (auto &it : hmap) {
        hhdrStr += it.first + ": " + it.second + "\r\n";
    }

    hhdrStr += "\r\n";

    *len = hhdrStr.size(); // copy without '\0'
    *dst = (char *)malloc(*len);
    memcpy(*dst, hhdrStr.c_str(), *len);
}

std::map<std::string, std::string> ZeldaHTTPHelper::headerMapFromHeaderData(const char *data, size_t len) {
    auto hmap = std::map<std::string, std::string>();

    size_t lineStart = 0;
    size_t lineEnd = 0;
    size_t sepLoc = 0;
    int isFirstLine = 0;

    for (size_t i = 0; i < len; ++i) {
        char ch = *(data + i);
        if (sepLoc == 0 && ch == ' ') {
            isFirstLine = 1;
            continue;
        } else if (ch == '\r') {
            lineEnd = i;
        } else if (ch == '\n') {
            lineStart = i + 1;
            lineEnd = 0;
            sepLoc = 0;
            isFirstLine = 0;
            continue;
        }
        if (isFirstLine == 0) {
            if (sepLoc == 0 && ch == ':') {
                sepLoc = i;
            }
        }
        if (lineEnd != 0 && sepLoc != 0 &&
            sepLoc - lineStart > 0 &&
            lineEnd - sepLoc > 0) {

            auto *hkey = (char *)malloc(BUFSIZ);
            size_t sepLen = sepLoc - lineStart;
            strncpy(hkey, data + lineStart, sepLen);
            hkey[sepLen] = '\0';

            size_t spaceLoc = sepLoc + 2;
            auto *hval = (char *)malloc(BUFSIZ);
            size_t valLen = lineEnd - spaceLoc;
            strncpy(hval, data + spaceLoc, valLen);
            hval[valLen] = '\0';

            std::string hkeyStr(hkey);
            std::string hValStr(hval);

            hmap[hkeyStr] = hValStr;

            free(hkey);
            free(hval);

        }
        else if (lineEnd != 0 && isFirstLine)
        {

            auto *hhdr = (char *)malloc(BUFSIZ);
            size_t hdrLen = lineEnd - lineStart;
            strncpy(hhdr, data + lineStart, hdrLen);
            hhdr[hdrLen] = '\0';

            std::string hhdrStr(hhdr);

            hmap["_"] = hhdrStr;

            free(hhdr);

        }
    }

    return hmap;
}
