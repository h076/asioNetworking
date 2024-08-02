#include <iostream>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

// relatively large buffer to store returned data
std::vector<char> vBuffer(20 * 1024);

void grabData(asio::ip::tcp::socket& socket) {
    // async_read_some will prime the context with a task
    // the task is provided by our lambda function
    socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
        [&](std::error_code ec, std::size_t length) {
            if(!ec) {
                std::cout << "\n\nRead " << length << " bytes \n\n";

                for(int i = 0; i < length; i++) {
                    std::cout << vBuffer[i];
                }

                // if more data is present then grabData is called again
                grabData(socket);
            }
        }
    );
}

int main() {
    // used to return errors
    asio::error_code ec;

    // unique instance of asio, platfrom specific interface
    asio::io_context context;

    // provide idle tasks for context thread so that the thread does not close prematurly
    asio::io_context::work idlework(context);

    // run context in its own thread to prevent it from blocking the program
    std::thread contextThread = std::thread([&]() { context.run();});

    // conect via tcp to endpoint
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

    // create a network socket
    asio::ip::tcp::socket socket(context);

    // tell socket to try and connect
    socket.connect(endpoint, ec);

    if(!ec) {
        std::cout << "Connected." << std::endl;
    }else {
        std::cout << "Failed to connect to address : " << ec.message() << std::endl;
    }

    if(socket.is_open()) {

        // prime asio context to recived data before we make a request
        grabData(socket);

        std::string sRequest =
            "GET /index.html HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "Connection: close\r\n\r\n";

        // write our request to asio buffer and send to server
        socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(20000ms);

        context.stop();
        if(contextThread.joinable())
            contextThread.join();
    }




    return 0;
}
