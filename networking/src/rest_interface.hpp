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
                        case http::verb::get:  return handle_get(req);
                        case http::verb::post: return handle_post(req);
                        case http::verb::put: return handle_put(req);
                        case http::verb::delete_: return handle_delete(req);
                        case http::verb::connect: return handle_connect(req);
                        case http::verb::options: return handle_options(req);
                        case http::verb::trace: return handle_trace(req);
                        case http::verb::patch: return handle_patch(req);
                        case http::verb::unknown: return handle_unknown(req);
                        case http::verb::acl: return handle_acl(req);
                        case http::verb::bind: return handle_bind(req);
                        case http::verb::copy: return handle_copy(req);
                        case http::verb::link: return handle_link(req);
                        case http::verb::lock: return handle_lock(req);
                        case http::verb::merge: return handle_merge(req);
                        case http::verb::mkactivity: return handle_mkactivity(req);
                        case http::verb::mkcalendar: return handle_mkcalendar(req);
                        case http::verb::mkcol: return handle_mkcol(req);
                        case http::verb::move: return handle_move(req);
                        case http::verb::propfind: return handle_propfind(req);
                        case http::verb::proppatch: return handle_proppatch(req);
                        case http::verb::rebind: return handle_rebind(req);
                        case http::verb::report: return handle_report(req);
                        case http::verb::search: return handle_search(req);
                        case http::verb::subscribe: return handle_subscribe(req);
                        case http::verb::unbind: return handle_unbind(req);
                        case http::verb::unlink: return handle_unlink(req);
                        case http::verb::unlock: return handle_unlock(req);
                        case http::verb::unsubscribe: return handle_unsubscribe(req);
                        case http::verb::head: return handle_head(req);
                        case http::verb::checkout: return handle_checkout(req);
                        case http::verb::msearch: return handle_msearch(req);
                        case http::verb::notify: return handle_notify(req);
                        case http::verb::purge: return handle_purge(req);
                    }
                    return http::response<http::dynamic_body>
                                (http::status::method_not_allowed, req.version());
                }

            protected:

                // Standard HTTP
                virtual http::response<http::dynamic_body> handle_get(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_post(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_put(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_delete(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_connect(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_options(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_trace(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_patch(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_head(http::request<http::dynamic_body> const& req) = 0;

                // Misc / special verbs
                virtual http::response<http::dynamic_body> handle_unknown(http::request<http::dynamic_body> const& req) = 0;

                // WebDAV / extension methods
                virtual http::response<http::dynamic_body> handle_acl(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_bind(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_copy(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_link(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_lock(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_merge(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_mkactivity(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_mkcalendar(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_mkcol(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_move(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_propfind(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_proppatch(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_rebind(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_report(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_search(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_subscribe(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_unbind(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_unlink(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_unlock(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_unsubscribe(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_checkout(http::request<http::dynamic_body> const& req) = 0;

                // Extra / uncommon verbs
                virtual http::response<http::dynamic_body> handle_msearch(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_notify(http::request<http::dynamic_body> const& req) = 0;
                virtual http::response<http::dynamic_body> handle_purge(http::request<http::dynamic_body> const& req) = 0;


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
