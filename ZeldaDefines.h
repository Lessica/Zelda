//
// Created by Zheng on 05/03/2018.
//

#ifndef ZELDA_ZELDADEFINES_H
#define ZELDA_ZELDADEFINES_H

#define ZELDA_NAME "ZeldaProxy"
#define ZELDA_VERSION "1.0"
#define ZELDA_ADDRESS "127.0.0.1"
#define ZELDA_PORT "1087"

#define ZELDA_BUF_SIZE 16384

#if defined(__linux__)

#define ZELDA_USE_SPLICE 1

#endif

#endif //ZELDA_ZELDADEFINES_H
