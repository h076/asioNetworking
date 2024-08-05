#include <iostream>

#include <olc_net.hpp>

enum class CustomMsgTypes : uint32_t // so each type id is 4 bytes
{
    FireBullet,
    MovePlayer
};

int main() {

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

    return 0;
}
