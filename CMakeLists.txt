cmake_minimum_required(VERSION 3.5)
project(Zelda)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Zelda.cpp Zelda.h cxxopts.hpp base64.hpp ZeldaDefines.h ZeldaLogger.cpp ZeldaLogger.h ZeldaHTTPResponse.cpp ZeldaHTTPResponse.h ZeldaHTTPRequest.cpp ZeldaHTTPRequest.h ZeldaProtocol.cpp ZeldaProtocol.h ZeldaHTTPHelper.cpp ZeldaHTTPHelper.h ZeldaHTTPTunnel.cpp ZeldaHTTPTunnel.h ZeldaAuthenticationAgent.cpp ZeldaAuthenticationAgent.h)
add_executable(Zelda ${SOURCE_FILES})
