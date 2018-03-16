//
// Created by Zheng Wu on 2018/3/14.
//

#ifndef ZELDA_ZELDAPROTOCOL_H
#define ZELDA_ZELDAPROTOCOL_H


#include <string>
#include "ZeldaProtocol.h"

class ZeldaResponse: ZeldaProtocol {

public:

    std::string Description();
    void PrintDescription();

};


#endif //ZELDA_ZELDAPROTOCOL_H
