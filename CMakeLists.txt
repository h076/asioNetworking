cmake_minimum_required(VERSION 3.17)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_CXX_STANDARD 20)

project("asioNetworking")

find_package(Boost 1.85.0 COMPONENTS program_options REQUIRED)

find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

include_directories(networking "${PROJECT_SOURCE_DIR}/networking/src")
include_directories(${Boost_INCLUDE_DIR})
add_subdirectory(netClient)
add_subdirectory(netServer)
add_subdirectory(websocketClient)

set(HEADER_FILES)

set(SOURCE_FILES
    src/example.cpp)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        /opt/homebrew/cellar/asio/1.30.2/include)
