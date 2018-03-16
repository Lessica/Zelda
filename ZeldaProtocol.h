//
// Created by Zheng Wu on 2018/3/16.
//

#ifndef ZELDA_ZELDAPROTOCOL_H
#define ZELDA_ZELDAPROTOCOL_H


#include <cstdio>

class ZeldaProtocol {

public:

    ZeldaProtocol();

    size_t DataLength();
    void AppendData(const char *data, const int& size);

    int ReadDataToBuffer(char *buffer, size_t offset, size_t length, size_t *actual);

private:

    char *_data;
    size_t _data_len;
    size_t _max_len;

};


#endif //ZELDA_ZELDAPROTOCOL_H
