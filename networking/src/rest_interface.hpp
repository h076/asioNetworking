#ifndef REST_INTERFACE_H_
#define REST_INTERFACE_H_

#include <rest_listener.hpp>
#include <thread>

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
                    : m_ioc(), m_ctxGuard(net::make_work_guard(m_ioc))
                {
                    // Initialize the listener with the handler
                    tcp::endpoint endpoint(net::ip::make_address(address), port);
                    m_Listener = std::make_shared<Listener>(m_ioc, endpoint,
                                    [this](http::request<http::string_body> const& req) {
                                        return this->handler(req);
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

                ~Interface() {
                    // Stop io_context and join thread
                    m_ctxGuard.reset();
                    m_ioc.stop();
                    if (m_ctxThread.joinable())
                        m_ctxThread.join();
                }

            private:

                // General handler
                // Will call specific HTTP method handler
                http::response<http::string_body> handler(http::request<http::string_body> const& req) {
                    switch (req.method()) {
                        case http::verb::get: return handle_get(req);
                        case http::verb::post: return handle_post(req);
                        case http::verb::put: return handle_put(req);
                        case http::verb::delete_: return handle_delete(req);
                        defaut: return http::response<http::string_body>
                                (http::status::method_not_allowed, req.version());
                    }
                }

            protected:

                // GET
                virtual http::response<http::string_body> handle_get(http::request<http::string_body> const& req) = 0;

                // POST
                virtual http::response<http::string_body> handle_post(http::request<http::string_body> const& req) = 0;

                // PUT
                virtual http::response<http::string_body> handle_put(http::request<http::string_body> const& req) = 0;

                // DELETE
                virtual http::response<http::string_body> handle_delete(http::request<http::string_body> const& req) = 0;

            private:

                // Asio context
                net::io_context m_ioc;

                // Dedicated io thread
                std::thread m_ctxThread;

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
