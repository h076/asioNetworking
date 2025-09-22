#ifndef REST_LISTENER_H_
#define REST_LISTENER_H_

#include <rest_session.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace hjw {

    namespace rest {

        // Accepts incoming connections and spawns sessions for each connection
        class Listener : public std::enable_shared_from_this<Listener> {

            public:

                using Handler = std::function<http::response<http::dynamic_body>
                                              (http::request<http::dynamic_body> const&)>;

                // Pass Handler from interface class, to be passed to each sessiono
                Listener(net::io_context& ioc, tcp::endpoint endpoint, Handler h)
                    : m_ioc(ioc), m_Acceptor(net::make_strand(ioc)), m_Handler(h)
                {
                    beast::error_code ec;

                    // Open the endpoint on port provided
                    m_Acceptor.open(endpoint.protocol(), ec);
                    if (ec) {
                        std::cerr << "Open error: " << ec.message() << std::endl;
                        return;
                    }

                    // Set socket options
                    m_Acceptor.set_option(net::socket_base::reuse_address(true), ec);
                    if (ec) {
                        std::cerr << "Set Option error: " << ec.message() << std::endl;
                        return;
                    }

                    // Bind to port
                    m_Acceptor.bind(endpoint, ec);
                    if (ec) {
                        std::cerr << "Bind error: " << ec.message() << std::endl;
                        return;
                    }

                    // Listen on port
                    m_Acceptor.listen(net::socket_base::max_listen_connections, ec);
                    if (ec) {
                        std::cerr << "Listen error: " << ec.message() << std::endl;
                        return;
                    }
                }

                void start() {
                    // The use of detached means forget about this co_routine
                    // Ignore all exceptions thrown
                    net::co_spawn(m_ioc, co_accept(), net::detached);
                }

            private:

                net::awaitable<void> co_accept() {
                    // For loop will not spin as we use co_await for waiting for connections
                    for (;;) {
                        tcp::socket socket = co_await m_Acceptor.async_accept(net::use_awaitable);
                        // Create session obejct within shared ptr
                        auto session = std::make_shared<Session>(std::move(socket), m_Handler);

                        // spawn co_routine to run session
                        // Passing ptr to lambda to keep session alive
                        net::co_spawn(m_ioc, [session]() -> net::awaitable<void> {
                                co_await session->run();
                            }, net::detached);
                    }
                }

            private:

                // Asio context to spawn sessions on
                net::io_context& m_ioc;

                // TCP acceptor to maage endpoint
                tcp::acceptor m_Acceptor;

                // Request handler passed to each session
                Handler m_Handler;

        };
    }
}

#endif // REST_LISTENER_H_
