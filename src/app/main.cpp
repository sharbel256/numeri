#include "trading.h"
#include "readers.h"
#include "thread"


int main(int argc, char *argv[])
{
    Trading trading;
    trading.login();
    trading.liveFunction();

    readers::readerThreadFunction(trading.getOrderbooks()["BTC-USD"]);

    // handle graceful shutdown
    std::this_thread::sleep_for(std::chrono::seconds(10));
    trading.stop();
    

    return 0;
}

