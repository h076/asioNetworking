#include "net_message.hpp"
#include <chrono>
#include <iostream>

#include <hjw_net.hpp>
#include <system_error>

enum class CustomMsgTypes : uint32_t // so each type id is 4 bytes
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage
};

class CustomClient : public hjw::net::client_interface<CustomMsgTypes> {
    public:
        void PingServer() {
            hjw::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::ServerPing;

            // grab time from client
            // only using this as we know server and client are on same machine
            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
            msg << timeNow;
            Send(msg);
        }

        void MessageAll() {
            hjw::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::MessageAll;
            Send(msg);
        }
};

int main() {

    /*
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::FireBullet;

    int a = 7;
    char b = 'h';
    bool f = false;

    struct {
        float x;
        float y;
    }xy[5];

    msg << a << b << f << xy;

    std::cout << msg << std::endl;

    a = 100;
    b = 'y';
    f = true;

    msg >> xy >> f >> b >> a;

    std::cout << a << std::endl;
    */

    // easier to use client interface to connect and send
    CustomClient c;
    c.Connect("127.0.0.1", 60000);

    asio::io_service ioService;
    asio::posix::stream_descriptor stream(ioService, STDIN_FILENO);

    char buf[1] = {};
    bool bQuit = false;

    //bool keys[3] = {false, false, false};
    //bool oldKeys[3] = {false, false, false};

    std::function<void(std::error_code, size_t)> read_handler;
    read_handler = [&](std::error_code ec, size_t length) {
                if(!ec) {
                    if(length == 1 && buf[0] != '\n') {
                        //std::cout << "key : " << buf[0] << "\n";
                        if(buf[0] == 'q') {
                            bQuit = true;
                            return;
                        }else if(buf[0] == '1') {
                            c.PingServer();
                        }else if(buf[0] == '2') {
                            c.MessageAll();
                        }else if(buf[0] == '3') {
                            bQuit = true;
                            return;
                        }
                    }
                    asio::async_read(stream, asio::buffer(buf, 1), read_handler);
                }else {
                    std::cerr << "exit with " << ec.message() << "\n";
                }
             };

    asio::async_read(stream, asio::buffer(buf, 1), read_handler);

    std::thread ioThread = std::thread([&](){ioService.run();});

    while(!bQuit) {
        if(c.IsConnected()) {
            if(!c.Incoming().empty()) {
                auto msg = c.Incoming().pop_front().msg;

                switch(msg.header.id) {
                    case CustomMsgTypes::ServerAccept:
                    {
                        std::cout << "Client has been validated by the server\n";
                    }
                    break;

                    case CustomMsgTypes::ServerPing:
                    {
                        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                        std::chrono::system_clock::time_point timeThen;

                        msg >> timeThen;
                        std::cout << "Ping : " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
                    }
                    break;

                    case CustomMsgTypes::ServerMessage:
                    {
                        // Server has responded to a ping requet
                        uint32_t ClientID;
                        msg >> ClientID;
                        std::cout << "Hello from [" << ClientID << "]\n";
                    }
                    break;

                }
            }
        }else {
            std::cout << "server down ... \n";
            bQuit = true;
        }
    }

    std::cout << "end\n";

    if(ioThread.joinable())
        ioThread.join();

    return 0;
}
