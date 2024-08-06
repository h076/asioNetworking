#ifndef NET_CONNECTION_H_
#define NET_CONNECTION_H_

#include "net_utils.hpp"
#include "net_tsQueue.hpp"
#include "net_message.hpp"
#include <memory>

namespace olc {

    namespace net {

        // enable_shared_from_this allows us to create a shared ptr from within this object,
        // provides a shared pointer to this keyword essentially
        template <typename T>
        class connection : public std::enable_shared_from_this<connection<T>> {
            public:
                connection() {}

                virtual ~connection() {}

            public:
                // called by only clients
                bool ConnectToServer();

                // called by both clients and server
                bool Disconnect();

                // is the connection open
                bool IsConnected() const;

            public:
                // send message over connection
                bool Send(const message<T>& msg);

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

        };
    }
}

#endif // NET_CONNECTION_H_
