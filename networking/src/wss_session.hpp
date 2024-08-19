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
#include <chrono>
#include <memory>
#include <system_error>

#include <boost/asio/strand.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <thread>

#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <boost/asio/signal_set.hpp>
#include <thread>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast; // from boost/beast.hpp
namespace http = beast::http; // from boost/beast/http.hpp
namespace websocket = beast::websocket; // from boost/beast/websocket.hpp
namespace ssl = boost::asio::ssl; // from boost/asio/ssl.hpp
using tcp = asio::ip::tcp; // form asio/ip/udp.hpp
typedef asio::io_context::executor_type executor_type;
typedef asio::strand<executor_type> strand;

namespace hjw {

    namespace wss {

        // General function to report failure
        void fail(std::error_code ec, char const* what) {
            std::cerr << what << ec.message() << "\n";
        }

        // using enable_shared_from_this o easily initilaise a shared_ptr to a session object
        class session : public std::enable_shared_from_this<session> {
            public:
                // resolver and socket require an io_context
                // use explicit to prevent unwanted type conversions
                explicit
                session(asio::io_context& asio_ioc, ssl::context& ssl_ctx)
                    // resolver looks up domain name
                    : m_oResolver(asio::make_strand(asio_ioc)), // using make_strand to create a dedicated thread for the resolver that dosnt require the use of mutexs
                      m_oWebSocketStream(asio::make_strand(asio_ioc), ssl_ctx), // pass asio context and ssl context to websocket constructor
                      m_oAsioContext(asio_ioc), // reference to io_context created in wss_client_interface
                      m_oAsioStrand(asio_ioc.get_executor()) // Get strand from the io context, when work is submitted to a strand it ensures only one thread is running at a time
                {

                }

                ~session() {

                }

                void Run(char const* host, char const* port, char const* text, char const* endpoint) {
                    // cache host, text and endpoint
                    m_sHost = host;
                    m_sText = text;
                    m_sEndpoint = endpoint;

                    // loop up the domain name
                    m_oResolver.async_resolve(
                        host, port,
                        // bind_front_handler will take a shared pointer of this object and
                        // a function pointer to our reolve function, which is called when we use async_resolve
                        beast::bind_front_handler(&session::OnResolve, shared_from_this()));
                }

                void OnResolve(beast::error_code ec, tcp::resolver::results_type results) {
                    if(ec)
                        return wss::fail(ec, "resolve");

                    // set a timeout for the operation using chrono
                    // get_lowest_layer will retrieve the underlying socket
                    beast::get_lowest_layer(m_oWebSocketStream).expires_after(std::chrono::seconds(15));

                    // make the connection to the IP address we recieved from the look up
                    beast::get_lowest_layer(m_oWebSocketStream).async_connect(
                        results,
                        // Bind the strand to an executor of the type the strand is based on
                        // Bind front handler creates a functor object, calling OnConnect within this instance
                        // Once async connect completes it calls the functor object with an error_code and endpoint_type,
                        // which are required arguments for the OnConnect function
                        asio::bind_executor(m_oAsioStrand, beast::bind_front_handler(&session::OnConnect, shared_from_this())));
                }

                void OnConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
                    if(ec)
                        return wss::fail(ec, "connnect");

                    // Get the socket associated with the web socket and set a timeout
                    // m_oWebSocketStream is defined as wedsocket::stream<beast::ssl_stream<beast::tcp_stream>>
                    // so get_lowest_layer will return a tcp_stream
                    beast::get_lowest_layer(m_oWebSocketStream).expires_after(std::chrono::seconds(15));

                    // Set Server Name Indicator host name (needed for succesfull handshake with host)
                    if(!SSL_set_tlsext_host_name(m_oWebSocketStream.next_layer().native_handle(), m_sHost.c_str())) {
                        ec = beast::error_code(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
                        return fail(ec, "connect");
                    }

                    // upadte host string with port
                    m_sHost += ":" + std::to_string(ep.port());

                    // Preform SSL handshake.
                    // next_layer() will return the websockets stream layer beast::ssl_stream
                    m_oWebSocketStream.next_layer().async_handshake(
                        ssl::stream_base::client,
                        asio::bind_executor(m_oAsioStrand, beast::bind_front_handler(&session::OnSSLHandshake, shared_from_this())));

                }

                void OnSSLHandshake(beast::error_code ec) {
                    if(ec)
                        return fail(ec, "handshake");

                    // turn off the time out on the tcp stream
                    // The websocket stream has its own time out system
                    beast::get_lowest_layer(m_oWebSocketStream).expires_never();

                    // set suggested timeout settings for the socket
                    m_oWebSocketStream.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

                    // set a decorator to change the user-agent of the handshake
                    // change the way the client is recognised by the server
                    m_oWebSocketStream.set_option(websocket::stream_base::decorator(
                            [](websocket::request_type& reqType) {
                                reqType.set(http::field::user_agent,
                                            std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async-ssl");
                            }));

                    // Preform websocket handshake
                    m_oWebSocketStream.async_handshake(m_sHost, m_sEndpoint,
                                                       asio::bind_executor(m_oAsioStrand,
                                                                           beast::bind_front_handler(&session::OnHandshake, shared_from_this())));
                }

                void OnHandshake(beast::error_code ec) {
                    if(ec)
                        return fail(ec, "handshake");

                    // send outv requets to the server
                    m_oWebSocketStream.async_write(asio::buffer(m_sText),
                                                   asio::bind_executor(m_oAsioStrand,
                                                                       beast::bind_front_handler(&session::OnWrite, shared_from_this())));
                }

                void OnWrite(beast::error_code ec, std::size_t bytes_transfered) {
                    // suppress unused variable compiler warning
                    // bytes_transfered contains total bytes that have been read/written to the socket
                    boost::ignore_unused(bytes_transfered);

                    if(ec)
                        return fail(ec, "write");

                    // Read single message from the server
                    m_oWebSocketStream.async_read(m_oBuffer,
                                                  asio::bind_executor(m_oAsioStrand,
                                                                      beast::bind_front_handler(&session::OnRead, shared_from_this())));
                }

                void OnRead(beast::error_code ec, std::size_t bytes_transfered) {
                    boost::ignore_unused(bytes_transfered);

                    if(ec)
                        return fail(ec, "read");

                    // print the data recieved from the websocket
                    // make_printable interprets the bytes as characters
                    std::cout << beast::make_printable(m_oBuffer.data()) << " By thread ID : " << boost::this_thread::get_id() << std::endl;

                    // clean the buffer
                    m_oBuffer.consume(m_oBuffer.size());

                    // re-prime the websocket with another read operation
                    m_oWebSocketStream.async_read(m_oBuffer,
                                                  asio::bind_executor(m_oAsioStrand,
                                                                      beast::bind_front_handler(&session::OnRead, shared_from_this())));
                }

                // how to properly close ?
                void OnClose(beast::error_code ec) {
                    if(ec)
                        fail(ec, "close");

                    // websocket has been closed gracefully
                    std::cout << beast::make_printable(m_oBuffer.data()) << std::endl;
                }

            private:
                asio::io_context& m_oAsioContext;
                tcp::resolver m_oResolver;
                websocket::stream<beast::ssl_stream<beast::tcp_stream>> m_oWebSocketStream;
                beast::flat_buffer m_oBuffer;
                std::string m_sHost;
                std::string m_sText;
                std::string m_sEndpoint;
                strand m_oAsioStrand;

        };

    }
    
}

#endif // WSS_SESSION_H_
