cmake_minimum_required(VERSION 3.7)
project(Zelda)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Zelda.cpp Zelda.h cxxopts.hpp ZeldaDefines.h ZeldaLogger.cpp ZeldaLogger.h ZeldaHTTPResponse.cpp ZeldaHTTPResponse.h ZeldaHTTPRequest.cpp ZeldaHTTPRequest.h ZeldaProtocol.cpp ZeldaProtocol.h ZeldaHTTPHelper.cpp ZeldaHTTPHelper.h)
add_executable(Zelda ${SOURCE_FILES})
