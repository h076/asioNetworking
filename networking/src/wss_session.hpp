#ifndef WSS_SESSION_H_
#define WSS_SESSION_H_

/**
 * This class is creates a "session" with a websocket, allowing messages to be sent
 * and recieved from a socket. Using the Boost Beast library to allow a seperate client class
 * wss_client_interface to connect to websockets and read various data from the socket
 */

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <memory>
#include <system_error>

#define ASIO_STANDALONE
#include <asio/strand.hpp>
#include <asio/signal_set.hpp>
#include <asio.hpp>
#include <thread>

#include <iostream>

namespace beast = boost::beast; // from boost/beast.hpp
namespace http = beast::http; // from boost/beast/http.hpp
namespace websocket = beast::websocket; // from boost/beast/websocket.hpp
namespace ssl = boost::asio::ssl; // from boost/asio/ssl.hpp
using udp = asio::ip::udp; // form asio/ip/udp.hpp
typedef asio::io_context::executor_type executor_type;
typedef asio::strand<executor_type> strand;

namespace hjw {

    namespace wss {

        // General fcuntion to report failure
        void fail(beast::error_code ec, char const* what) {
            std::cerr << what << ec.message() << "\n";
        }

        class session : public std::enable_shared_from_this<session> {

        };

    }
    
}

#endif // WSS_SESSION_H_
