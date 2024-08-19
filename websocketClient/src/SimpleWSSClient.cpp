#include <wss_client_interface.hpp>

int main(int argc, char** argv)
{
    hjw::wss::client_interface ws;
    ws.SetHost("marketdata.tradermade.com");
    ws.SetPort("443");
    ws.SetRequest("{\"userKey\":\"wswfy1upa1xLq2MnfIYg\", \"symbol\":\"GBPUSD\", \"fmt\":\"SSV\"}");
    ws.SetEndpoint("/feedadv");
    if(!ws.isOpen())
        ws.Open();

    asio::io_service ioService;
    asio::posix::stream_descriptor stream(ioService, STDIN_FILENO);

    char buf[1] = {};
    bool bQuit = false;

    std::function<void(std::error_code, size_t)> read_handler;
    read_handler = [&](std::error_code ec, size_t length) {
                if(!ec) {
                    if(length == 1 && buf[0] != '\n') {
                        //std::cout << "key : " << buf[0] << "\n";
                        if(buf[0] == 'q') {
                            bQuit = true;
                            return;
                        }
                    }
                    asio::async_read(stream, asio::buffer(buf, 1), read_handler);
                }else {
                    std::cerr << "exit with " << ec.message() << "\n";
                }
             };

    asio::async_read(stream, asio::buffer(buf, 1), read_handler);

    std::thread ioThread = std::thread([&]() {ioService.run();});

    while(!bQuit) {
        continue;
    }

    if(ws.isOpen())
        ws.Close();
    ioThread.join();

    return 0;
}
