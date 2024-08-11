#include "net_message.hpp"
#include "net_server.hpp"
#include <iostream>
#include <hjw_net.hpp>
#include <memory>

enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage
};

class CustomServer : public hjw::net::server_interface<CustomMsgTypes> {
    public:
        CustomServer(uint16_t nPort) : hjw::net::server_interface<CustomMsgTypes>(nPort) {

        }

    protected:
        virtual bool OnClientConnection(std::shared_ptr<hjw::net::connection<CustomMsgTypes>> client) {
            return true;
        }

        virtual void OnClientDisconnect(std::shared_ptr<hjw::net::connection<CustomMsgTypes>> client) {

        }

        virtual void OnMessage(std::shared_ptr<hjw::net::connection<CustomMsgTypes>> client) {

        }
};

int main() {
    CustomServer s(60000);
    s.Start();

    while(1)
        s.update();

    return 0;
}
