
#include "busi.h"
#include "loguru.hpp"

void BusiProc::operator()() const { 
    // do something in a thread
    cout << "hello in a thread" << endl;
    LOG_F(INFO, "hello from thread!");
}

