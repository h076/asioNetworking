#include <wss_client_interface.hpp>

int main(int argc, char** argv)
{
    hjw::wss::client_interface ws;
    ws.SetHost("marketdata.tradermade.com");
    ws.SetPort("443");
    ws.SetRequest("{\"userKey\":\"wswfy1upa1xLq2MnfIYg\", \"symbol\":\"GBPUSD\", \"fmt\":\"SSV\"}");
    ws.SetEndpoint("/feedadv");
    ws.Open();

    return 0;
}
