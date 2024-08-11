add_executable(SimpleServer src/SimpleServer.cpp)

target_include_directories(SimpleServer PRIVATE
        /opt/homebrew/cellar/asio/1.30.2/include)

target_include_directories(SimpleServer PRIVATE networking)
