#ifndef NET_CLIENT_H_
#define NET_CLIENT_H_

#include "net_utils.hpp"
#include "net_message.hpp"
#include "net_tsQueue.hpp"
#include "net_connection.hpp"
#include <exception>
#include <string>
#include <type_traits>

namespace hjw {

    namespace net {

        template <typename T>
        class client_interface {
            public:
                client_interface() : m_oSocket(m_oContext) {
                    // set up asio socket with context
                }

                virtual ~client_interface() {
                    Disconnect();
                }

            public:
                // Allow clinet to connect to a server
                bool Connect(const std::string& host, const uint16_t port) {

                    try {
                        // resolve hostname/ip-address into tangiable physical address
                        asio::ip::tcp::resolver resolver(m_oContext);
                        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

                        // create connection
                        m_pConnection = std::make_unique<connection<T>>(
                            connection<T>::owner::client,
                            m_oContext, asio::ip::tcp::socket(m_oContext),
                            m_qMessagesIn);

                        // Connect to the server
                        m_pConnection->ConnectToServer(endpoints);

                        // start context thread
                        m_tContextThread = std::thread([this]() {m_oContext.run();});

                    }catch (std::exception& e) {
                        std::cerr << "Client exception : " << e.what() << "\n";
                        return false;
                    }

                    return true;
                }

                // Disconnect from server
                void Disconnect() {
                    // if connection exists, disconnect
                    if(IsConnected()) {
                        // Disconnect from the server
                        m_pConnection->Disconnect();
                    }

                    // stop asio context and join its thread
                    m_oContext.stop();
                    if(m_tContextThread.joinable())
                        m_tContextThread.join();

                    // release unique pointer for connection object
                    m_pConnection.release();
                }

                // Check if a connection is presnet
                bool IsConnected() {
                    if(m_pConnection)
                        return m_pConnection->IsConnected();
                    else
                        return false;
                }

                // send message to server
                void Send(const message<T>& msg) {
                    if(IsConnected())
                        m_pConnection->Send(msg);
                }

                // Retrieve queue of messages from server
                tsqueue<owned_message<T>>& Incoming() {
                    return m_qMessagesIn;
                }

            protected:
                // Client owns its own asio context to handle data transfer
                asio::io_context m_oContext;

                // context requires a thread to execute its work
                std::thread m_tContextThread;

                // hardware socket that is connected to the server
                asio::ip::tcp::socket m_oSocket;

                // The client has a single instance of connection object, which handles data transfer
                std::unique_ptr<connection<T>> m_pConnection;

                // connection endpoints
                asio::ip::tcp::endpoint m_oEndpoints;

            private:
                // Thread safe queue of incoming messages from the server
                tsqueue<owned_message<T>> m_qMessagesIn;
        };
    }
}

#endif // NET_CLIENT_H_
