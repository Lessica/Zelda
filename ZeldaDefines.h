//
// Created by Zheng on 05/03/2018.
//

#define ZELDA_NAME "ZeldaProxy"
#define ZELDA_VERSION "1.0"
#define ZELDA_ADDRESS "127.0.0.1"
#define ZELDA_PORT "10087"

#define ZELDA_BUF_SIZE 16384

#if defined(__linux__)

#define ZELDA_USE_SPLICE 1

#endif
