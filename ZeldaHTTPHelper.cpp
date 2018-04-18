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

std::map<std::string, std::string> ZeldaHTTPHelper::headerMapFromHeaderData(const char *data, size_t len, bool is_request)
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

            if (is_request) {
                std::string method;
                std::string url;
                std::string protocol;

                std::istringstream iss(hhdrStr);
                iss >> method >> url >> protocol;

                int count = 0;
                std::string::size_type pos = 0;
                while (count < 3) {
                    pos = url.find('/', pos + 1);
                    if (pos == std::string::npos) {
                        break;
                    }
                    count++;
                }

                if (count == 3) {
                    url = url.substr(pos, url.size() - pos);
                }

                std::string hhdrNewStr;
                hhdrNewStr += method;
                hhdrNewStr += " ";
                hhdrNewStr += url;
                hhdrNewStr += " ";
                hhdrNewStr += protocol;
                hmap["_"] = hhdrNewStr;
            } else {
                hmap["_"] = hhdrStr;
            }

        }

    }

    return hmap;
}

std::string ZeldaHTTPHelper::getGMTDateString() {
    char buf[BUFSIZ];
    time_t now = time(nullptr);
    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S GMT", &tm);
    return std::string(buf);
}

std::string ZeldaHTTPHelper::forbiddenPage() {
    size_t maxlen = BUFSIZ * 16;
    FILE *fp = fopen("403.html", "rb");
    char buffer[maxlen];
    bzero(buffer, maxlen);
    fread(buffer, maxlen, 1, fp);
    fclose(fp);
    return std::string(buffer);
}
