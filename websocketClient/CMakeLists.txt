add_executable(SimpleWSSClient src/SimpleWSSClient.cpp)

find_package(Boost 1.85.0 COMPONENTS program_options REQUIRED)

set(OPENSSL_ROOT_DIR /opt/homebrew/Cellar/openssl@3/3.1.0)
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

target_include_directories(SimpleWSSClient PRIVATE
        /opt/homebrew/cellar/asio/1.30.2/include)

target_link_libraries(SimpleWSSClient Boost::program_options ${OPENSSL_LIBRARIES})

target_include_directories(SimpleWSSClient PRIVATE networking)
