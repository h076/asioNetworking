cmake_minimum_required(VERSION 3.17)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set (CMAKE_CXX_STANDARD 14)

project("asioNetworking")

set(HEADER_FILES)

set(SOURCE_FILES
    src/example.cpp)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        /opt/homebrew/cellar/asio/1.30.2/include)
