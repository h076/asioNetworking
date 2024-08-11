add_executable(SimpleClient src/SimpleClient.cpp)

target_include_directories(SimpleClient PRIVATE
        /opt/homebrew/cellar/asio/1.30.2/include)

target_include_directories(SimpleClient PRIVATE networking)
