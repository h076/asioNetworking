#ifndef WSS_CLIENT_INTERFACE_H_
#define WSS_CLIENT_INTERFACE_H_

/**
 * This class will use the Boost Beast library to allow a seperate client class
 * wss_client to connect to websockets and read various data from the socket
 */

#include "wss_session.hpp"
#include <memory>

namespace hjw {

    namespace wss {

        class client_interface {
            public:

                client_interface()
                    : m_sHost("null"), m_sPort("443"),
                      m_sRequest("null"), m_nThreadCount(3),
                      m_sEndpoint("null")
                {}

                ~client_interface() {}

                void SetHost(std::string h) {m_sHost = h;}
                void PrintHost() {std::cout << "Host : " << m_sHost << std::endl;}

                void SetPort(std::string p) {m_sPort = p;}
                void PrintPort() {std::cout << "Port : " << m_sPort << std::endl;}

                void SetRequest(std::string r) {m_sRequest = r;}
                void printRequest() {std::cout << "Request : " << m_sRequest << std::endl;}

                void SetEndpoint(std::string e) {m_sEndpoint = e;}
                void PrintEndpoint() {std::cout << "Endpoint : " << m_sEndpoint << std::endl;}

                void SetThreadCount(int tc) {m_nThreadCount = tc;}
                void PrintThreadCount() {std::cout << "Thread count : " << m_nThreadCount << std::endl;}

                void Open() {
                    // Launch the asynchronous operation
                    // Create instance of sessiona and return shared pointer
                    m_pSession = std::make_shared<session>(m_oAsioContext, m_oSSlContext);
                    m_pSession->Run(m_sHost.c_str(), m_sPort.c_str(), m_sRequest.c_str(), m_sEndpoint.c_str());

                    // Run the IO service
                    m_tContextThread = std::thread([this](){m_oAsioContext.run();});
                }

                void Close() {
                    m_oAsioContext.stop();
                    if(m_tContextThread.joinable())
                        m_tContextThread.join();

                    std::cout << "Socket connection closed\n";
                }

            private:
                asio::io_context m_oAsioContext;
                ssl::context m_oSSlContext{ssl::context::tlsv12_client};

                std::thread m_tContextThread;

                std::shared_ptr<session> m_pSession;

                std::string m_sHost;
                std::string m_sPort;
                std::string m_sRequest;
                std::string m_sEndpoint;

                int m_nThreadCount;
        };

    }
}

#endif // WSS_CLIENT_INTERFACE_H_
