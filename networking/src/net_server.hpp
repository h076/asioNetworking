#ifndef NET_SERVER_H_
#define NET_SERVER_H_

#include "net_utils.hpp"
#include "net_message.hpp"
#include "net_tsQueue.hpp"
#include "net_connection.hpp"
#include <exception>
#include <memory>
#include <system_error>

namespace hjw {

    namespace net {

        /**
         * Server working with asio ...
         *
         * The server will contain an asio context.
         * This context will be primed, waiting to accept or reject a connection
         *
         * If a connection is rejected then the asio context is re-primed; if the connection
         * is accepted then a shared pointer to a connection is stored within the server.
         *
         * An open connection also has corresponds to a socket.
         *
         * Once connections are established they will allocate work to the asio context,
         * to read a header.
         *
         * If a message header has been filled by a clinet, a request to the server, then the read header
         * task expires and a read body task will be allocated.
         *
         * Once the read body task has expired we will add the message to the server's incoming message queue.
         *
         */

        template <typename T>
        class server_interface {
            public:
                server_interface(uint16_t port)
                    // address passed to accepter is the one the server will listen to conections on
                    : m_oAsioAcceptor(m_oContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
                {

                }

                virtual ~server_interface() {
                    Stop();
                }

                bool Start() {
                    try {
                        WaitForClientConnection();
                        // start context thread
                        m_tContextThread = std::thread([this]() {m_oContext.run(); } );
                    }catch(std::exception& e) {
                        std::cerr << "[SERVER] Exception: " << e.what() << "\n";
                        return false;
                    }

                    std::cout << "[SERVER] Started. \n";
                    return true;
                }

                void Stop() {
                    // request context to close
                    m_oContext.stop();

                    // join context thread
                    if(m_tContextThread.joinable())
                        m_tContextThread.join();

                    std::cout << "[SERVER] Stopped.\n";
                }

                // ASYNC - instruct asio to wait for connections
                void WaitForClientConnection() {
                    // lambda function fired when connection is to be made
                    m_oAsioAcceptor.async_accept(
                        [this](std::error_code ec, asio::ip::tcp::socket socket)
                        {
                            if(!ec) {
                                // Succesfull connection, print ip of connection
                                std::cout << "[SERVER] New connection:" << socket.remote_endpoint() << "\n";

                                // tell new connection it is owned by the server
                                std::shared_ptr<connection<T>> newConnection =
                                    std::make_shared<connection<T>>(connection<T>::owner::server,
                                                    m_oContext, std::move(socket), m_qMessagesIn);

                                // Chance for user server to deny connection
                                if(OnClientConnection(newConnection)) {
                                    // connection allowedm push to connection container
                                    m_dqConnections.push_back(std::move(newConnection));

                                    m_dqConnections.back()->ConnectToClient(this, nIDCounter++);

                                    std::cout << "[" << m_dqConnections.back()->GetID() << "] Approved connection\n";

                                }else {
                                    std::cout << "Connection denied.\n";
                                }

                            }else {
                                // Error during acceptance
                                std::cerr << "[SERVER] Connection error: " << ec.message() << "\n";
                            }

                            // Prime asio function with more work
                            WaitForClientConnection();
                        });
                }

                // Send message to specific client
                void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg) {
                    if(client && client->IsConnected()) {
                        client->Send(msg);
                    }else {
                        OnClientDisconnect(client);
                        client.reset(); // call connection object destructor
                        m_dqConnections.erease(
                            std::remove(m_dqConnections.begin(), m_dqConnections.end(), client), m_dqConnections.end());
                    }
                }

                // Send message to all clients
                void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr) {
                    bool bInvalidConnectionsExist = false;

                    for(auto& client : m_dqConnections) {
                        if(client && client->IsConnected()) {
                            if(client != pIgnoreClient)
                                client->Send(msg);
                        }else {
                            // client cannot be conntacted so must be disconnected
                            OnClientDisconnect(client);
                            client.reset();
                            bInvalidConnectionsExist = true;
                        }
                    }

                    if(bInvalidConnectionsExist)
                        m_dqConnections.erase(
                            std::remove(m_dqConnections.begin(), m_dqConnections.end(), nullptr), m_dqConnections.end());
                }

            protected:

                // Called to explicitly process messages in the server queue
                virtual void update(size_t maxMesssages = -1, bool bWait = false) {
                    // so that we do not occupy the CPU while doing nothing
                    // we can tell the server to rest
                    if (bWait) m_qMessagesIn.wait();

                    for(size_t messageCount = 0; messageCount < maxMesssages && !m_qMessagesIn.empty(); messageCount++) {
                        // grab the front message
                        auto msg = m_qMessagesIn.pop_front();

                        // pass to message handler
                        OnMessage(msg.remote, msg.msg); // remote is a shared pointer to the connection of the client
                    }
                }

            protected:
                // Called when a client connects, you can reject the connection by returning false
                virtual bool OnClientConnection(std::shared_ptr<connection<T>> client) {
                    return false;
                }

                // Called when client has disconnected
                virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client) {

                }

                // Called to deal with message from specific client
                virtual void OnMessage(std::shared_ptr<connection<T>> client, const message<T>& msg) {

                }

            public:
                // Called when a client has been validated
                virtual void OnClientValidated(std::shared_ptr<connection<T>> client) {

                }

            protected:
                // thread safe queue for incoming message packets
                tsqueue<owned_message<T>> m_qMessagesIn;

                // Container of active connections
                std::deque<std::shared_ptr<connection<T>>> m_dqConnections;

                // order of declaration here is also order of initialisation
                asio::io_context m_oContext; // this context is shared to the connections
                std::thread m_tContextThread;

                // Used to get the sockets of the clients
                asio::ip::tcp::acceptor m_oAsioAcceptor;

                // unique identifier for clients
                uint32_t nIDCounter = 10000;


        };
    }
}

#endif // NET_SERVER_H_
