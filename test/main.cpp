#include "yacht.h"
#include <iostream>

using namespace std::chrono;

int main() {
    auto yacht_thread = MakeHandyThreadPtr();
    yacht_thread
        ->SetConfig(Delay_Config, "1000")
        ->Run([] { printf("原神, "); });
    yacht_thread->Wait();
    yacht_thread->Run([] { printf("启动\n"); });

    std::this_thread::sleep_for(2000ms);
}