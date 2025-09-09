#ifndef REST_SESSION_H_
#define REST_SESSION_H_

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <boost/config.hpp>
#include <functional>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace hjw {

    namespace rest {

        // This class will handle induvidual client sessions over HTTP
        // Using async_read and async_write
        // Enable shared is used to prevent dangling operations if pointer destroyed
        class Session : std::enable_shared_from_this<Session> {

            public:

                using Handler = std::function<http::response<http::string_body>
                                              (http::request<http::string_body> const&)>;

                // Socket must be passed through constructor
                // Created by async_accept fucnction that runs session
                // A request handler is also passed
                Session(tcp::socket s, Handler h) : m_Socket(std::move(s)), m_Handler(std::move(h)) {}

                // Run function to start session
                // Should co_spawn run
                net::awaitable<void> run() { co_await co_read(); }

            private:

                // Read
                net::awaitable<void> co_read() {
                    auto self(shared_from_this());
                    co_await http::async_read(m_Socket, m_Buffer, m_Req, net::use_awaitable);
                    http::response<http::string_body> res = m_Handler(m_Req);
                    co_await co_write(res);
                }

                // Write
                // Shoudl always occur is unison with a read
                net::awaitable<void> co_write(http::response<http::string_body> res) {
                    auto self(shared_from_this());
                    auto sp_res = std::make_shared<http::response<http::string_body>>(std::move(res));
                    co_await http::async_write(m_Socket, *sp_res, net::use_awaitable);
                    m_Socket.shutdown(tcp::socket::shutdown_send);
                }


            private:

                // TCP socket used for HTTP messaging
                tcp::socket m_Socket;

                // Buffer for reading data
                beast::flat_buffer m_Buffer;

                // HTTP request object
                http::request<http::string_body> m_Req;

                // Handler for requests
                Handler m_Handler;

        };

    }
}

#endif // REST_SESSION_H_
