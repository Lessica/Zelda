//
// Created by Zheng Wu on 2018/3/16.
//

#include <cstdlib>
#include <cstring>
#include "ZeldaProtocol.h"
#include "ZeldaDefines.h"

ZeldaProtocol::ZeldaProtocol() {
    _data = (char *)malloc(ZELDA_BUF_SIZE);
    _max_len = ZELDA_BUF_SIZE;
    _data_len = 0;
};

size_t ZeldaProtocol::DataLength() {
    return _data_len;
}

void ZeldaProtocol::AppendData(const char *data, const int &len) {
    size_t expand_len = _max_len;
    while (_data_len + len > expand_len) {
        expand_len += ZELDA_BUF_SIZE;
    }
    if (expand_len > _max_len) {
        _data = (char *)realloc(_data, expand_len);
    }
}

int ZeldaProtocol::ReadDataToBuffer(char *buffer, size_t offset, size_t length, size_t *actual) {
    return 0;
}
