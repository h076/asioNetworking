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
            hjw::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::ServerAccept;
            client->Send(msg);

            return true;
        }

        virtual void OnClientDisconnect(std::shared_ptr<hjw::net::connection<CustomMsgTypes>> client) {
            std::cout << "Removing clinet [" << client->GetID() << "]\n";
        }

        virtual void OnMessage(std::shared_ptr<hjw::net::connection<CustomMsgTypes>> client,
                               const hjw::net::message<CustomMsgTypes>& msg) {
            switch(msg.header.id) {
                case CustomMsgTypes::ServerAccept:
                {

                }
                break;

                case CustomMsgTypes::ServerPing:
                {
                    std::cout << "[" << client->GetID() << "] Server ping.\n";
                    // bounce message back;
                    client->Send(msg);
                }
                break;

                case CustomMsgTypes::MessageAll:
                {
                    std::cout << "[" << client->GetID() << "] : Message all.\n";
                    hjw::net::message<CustomMsgTypes> msgOut;
                    msgOut.header.id = CustomMsgTypes::ServerMessage;
                    msgOut << client->GetID();
                    MessageAllClients(msgOut, client);
                }
                break;
            }
        }
};

int main() {
    CustomServer s(60000);
    s.Start();

    while(1)
        s.update();

    return 0;
}
