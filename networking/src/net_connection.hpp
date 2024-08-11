#ifndef NET_CONNECTION_H_
#define NET_CONNECTION_H_

#include "net_utils.hpp"
#include "net_tsQueue.hpp"
#include "net_message.hpp"
#include <memory>

namespace hjw {

    namespace net {

        // enable_shared_from_this allows us to create a shared ptr from within this object,
        // provides a shared pointer to this keyword essentially
        template <typename T>
        class connection : public std::enable_shared_from_this<connection<T>> {
            public:
                enum class owner {client, server};

                // We pass the owner of the connectioon, the asio context owned by the owner, the socket owned by the connection,
                // and the incoming message queue of the owner
                connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
                    : m_oAsioContext(asioContext), m_oSocket(std::move(socket)), m_qMessagesIn(qIn)
                {
                    m_nOwnerType = parent;
                }

                virtual ~connection() {}

                uint32_t GetID() {return id;}

            public:
                // called by only clients
                bool ConnectToServer();

                // called by both clients and server
                bool Disconnect() {
                    if(IsConnected()) {
                        // the asio context will close the socket when it is appropriate
                        asio::post(m_oAsioContext, [this]() {m_oSocket.close();});
                    }
                }

                // is the connection open
                bool IsConnected() const {
                    return m_oSocket.is_open();
                }

                // is called by the server interface when we create a new connection
                void ConnectToClient(uint32_t uid = 0) {
                    if(m_nOwnerType == owner::server) {
                        if(m_oSocket.is_open()) {
                            id = uid;

                            // Register task when we connect to client
                            ReadHeader();
                        }
                    }
                }

            public:
                // send message over connection
                bool Send(const message<T>& msg) {
                    // Send a job to the asio context as a lambda function
                    asio::post(m_oAsioContext,
                        [this, msg]() {
                            // are messages already being written
                            bool bWritingMessages = !m_qMessagesOut.empty();
                            m_qMessagesOut.push_back(msg);

                            // if messages arent being written then we can prime asio with writing header
                            // wanting to avoid multiple write header work load
                            if(!bWritingMessages)
                                WriteHeader();
                        });
                }

            private:
                // ASYNC - Prime context ready to read message header
                void ReadHeader() {
                    // reading from the particular socket associated with the connection
                    // using a asio buffer the size of a message header
                    asio::async_read(m_oSocket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
                        [this](std::error_code ec, std::size_t length) {
                            if(!ec) {
                                // Assuming the full message has been read
                                if(m_msgTemporaryIn.header.size > 0) {
                                    m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                                    ReadBody();
                                }else {
                                    // if no message body then we pass the message to the incoming queue
                                    AddToIncomingMessageQueue();
                                }
                            }else {
                                // if error occurs then we force close the socket
                                // therefore closing the connection
                                std::cout << "[" << id << "] Read header fail.\n";
                                m_oSocket.close();
                            }
                        });
                }

                // ASYNC - Prime context ready to read message body
                void ReadBody() {
                    // now reading form the same socket
                    // using the temporary message body as a ppointer for asio buffer
                    asio::async_read(m_oSocket, asio::buffer(&m_msgTemporaryIn.bod.data(), m_msgTemporaryIn.header.size),
                        [this](std::error_code ec, std::size_t length) {
                            if(!ec) {
                                AddToIncomingMessageQueue();
                            }else {
                                // force close socket if failed to read body
                                std::cout << "[" << id << "] Read body fail.\n";
                                m_oSocket.close();
                            }
                        });
                }

                // ASYNC - Prime context to write message header
                void WriteHeader() {
                    // now writing to the socket, from the asio buffer that is
                    // initialised with a pointer to the first message in the message out queue
                    asio::async_write(m_oSocket, asio::buffer(m_qMessagesOut.front().header, sizeof(message_header<T>)),
                        [this](std::error_code ec, std::size_t length) {
                            if(!ec) {
                                if(m_qMessagesOut.front().body.size() > 0) {
                                    WriteBody();
                                }else {
                                    m_qMessagesOut.pop_front();

                                    if(!m_qMessagesOut.empty())
                                        WriteHeader();
                                }
                            }else {
                                // force close socket if write fails
                                std::cout << "[" << id << "] Write head fail.\n";
                                m_oSocket.close();
                            }
                        });
                }

                // ASYNC - Prime conetxt to write message body
                void WriteBody() {
                    // now writing the body to the socket
                    asio::async_write(m_oSocket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
                        [this](std::error_code ec, std::size_t length) {
                            if(!ec) {
                                // after message is read by async_write we will then pop the message
                                m_qMessagesOut.pop_front();

                                // call WriteHeader if there is another message to be writen
                                if(!m_qMessagesOut.empty())
                                    WriteHeader();
                            }else {
                                // force close socket if write fails
                                std::cout << "[" << id << "] Write body fail.\n";
                                m_oSocket.close();
                            }
                        });
                }

                void AddToIncomingMessageQueue() {
                    // If the connection owner is a server, then we want to transform the
                    // message into an owned message
                    if(m_nOwnerType == owner::server) {
                        // using an initialiser list with a shared pointer to this connection objject
                        // we can push the message as a owned message
                        m_qMessagesIn.push_back({this->shared_from_this(), m_msgTemporaryIn});
                    }else {
                        // if the owner is a client then tagging the message makes no sense
                        // as clients only own one connection
                        m_qMessagesIn.push_back({nullptr, m_msgTemporaryIn});
                    }

                    // always called after we read a message, so prime asio again to read
                    ReadHeader();
                }

            protected:
                // Each connection has a unique socket
                asio::ip::tcp::socket m_oSocket;

                // asio context provided by the client or server interface
                asio::io_context& m_oAsioContext;

                // Queue for all messages to be sent to remote side of this connection
                tsqueue<message<T>> m_qMessagesOut;

                // Queue for all messages recieved from the remote side
                // of this connection. Really a referenec to a queue that
                // is owned by a server or client.
                tsqueue<owned_message<T>>& m_qMessagesIn;
                message<T> m_msgTemporaryIn;

                // is the connection owned by a server or client, determins some behaviour
                owner m_nOwnerType = owner::server;
                uint32_t id = 0;

        };
    }
}

#endif // NET_CONNECTION_H_
