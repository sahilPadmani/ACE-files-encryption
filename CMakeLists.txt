cmake_minimum_required(VERSION 4.1)
project(filesEncryption)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenSSL REQUIRED)

add_executable(filesEncryption
        main.cpp
        KeyHandler/KeyHandler.cpp
        encryption/files.cpp
)

target_link_libraries(filesEncryption PRIVATE OpenSSL::Crypto)