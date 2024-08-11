#include "net_message.hpp"
#include <iostream>

#include <hjw_net.hpp>

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

    return 0;
}
