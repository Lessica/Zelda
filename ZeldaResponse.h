//
// Created by Zheng Wu on 2018/3/14.
//

#import <string>
#include "ZeldaProtocol.h"

class ZeldaResponse: public ZeldaProtocol {

public:

    std::string Description();
    void PrintDescription();

};
