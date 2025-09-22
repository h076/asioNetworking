#ifndef REST_INTERFACE_H_
#define REST_INTERFACE_H_

#include <rest_listener.hpp>
#include <thread>
#include <boost/asio/signal_set.hpp>


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;


namespace hjw {

    namespace rest {

        // To be used as a base class for custom REST API wrapper
        // Child classes should implement all HTTP methods
        // GET, POST, PUT, DELETE
        class Interface {

            public:

                Interface(std::string address, uint16_t port)
                    : m_ioc(),
                      m_ctxGuard(net::make_work_guard(m_ioc)),
                      m_Signals(m_ioc, SIGINT, SIGTERM),
                      m_address(address), m_port(port)
                {}

                ~Interface() {
                    // join thread
                    if (m_ctxThread.joinable())
                        m_ctxThread.join();
                }

            protected:

                void start() {
                    // Initialize the listener with the handler
                    tcp::endpoint endpoint(net::ip::make_address(m_address), m_port);
                    m_Listener = std::make_shared<Listener>(m_ioc, endpoint,
                                    [this](http::request<http::dynamic_body> const& req) {
                                        return this->handler(req);
                                    });
                    m_Listener->start();

                    // Handle CRTL+C to gracefully stop the ioc and call destructor
                    // Signal set listening for crtl+c
                    m_Signals.async_wait([this](beast::error_code const&, int) {
                            std::cout << "Stopping REST service .... " << std::endl;
                            m_ctxGuard.reset();
                            m_ioc.stop();
                        });

                    // Launch dedicated thread for io_context
                    m_ctxThread = std::thread([this]() {
                        try {
                            m_ioc.run();
                        } catch (std::exception& e) {
                            std::cerr << "IO context exception: " << e.what() << std::endl;
                        }
                    });
                }

            protected:

                // General handler
                // Will call specific HTTP method handler
                http::response<http::dynamic_body> handler(http::request<http::dynamic_body> const& req) {
                    switch (req.method()) {
                        case http::verb::get: return handle_get(req);
                        case http::verb::post: return handle_post(req);
                        case http::verb::put: return handle_put(req);
                        case http::verb::delete_: return handle_delete(req);
                        defaut: return http::response<http::dynamic_body>
                                (http::status::method_not_allowed, req.version());
                    }
                }

            protected:

                // GET
                virtual http::response<http::dynamic_body> handle_get(http::request<http::dynamic_body> const& req) = 0;

                // POST
                virtual http::response<http::dynamic_body> handle_post(http::request<http::dynamic_body> const& req) = 0;

                // PUT
                virtual http::response<http::dynamic_body> handle_put(http::request<http::dynamic_body> const& req) = 0;

                // DELETE
                virtual http::response<http::dynamic_body> handle_delete(http::request<http::dynamic_body> const& req) = 0;

            private:

                // Asio context
                net::io_context m_ioc;

                // Dedicated io thread
                std::thread m_ctxThread;

                // Signal set for gracefull exit
                net::signal_set m_Signals;

                // work guard to keep Listener alive
                net::executor_work_guard<net::io_context::executor_type> m_ctxGuard;

                // Endpoint config
                std::string m_address;
                int m_port;

                // REST Listener
                std::shared_ptr<Listener> m_Listener;
        };

    }
}

#endif // REST_INTERFACE_H_
