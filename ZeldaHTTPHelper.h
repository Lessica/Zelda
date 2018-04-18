//
// Created by Zheng on 2018/4/10.
//


#import <map>
#import <list>

class ZeldaHTTPHelper {

public:

#pragma mark - Header

    static std::map<std::string, std::string> headerMapFromHeaderData(const char *data, size_t len);
    static void copyHeaderDataFromHeaderMap(char **dst, size_t *len, std::map<std::string, std::string> hmap);
    static std::string methodStringFromHeaderMap(std::map<std::string, std::string> hmap);
    static std::string getGMTDateString();

    static std::string forbiddenPage();

};
