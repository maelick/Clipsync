#ifndef DEF_BROADCASTER_H
#define DEF_BROADCASTER_H

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>
#include <Poco/Timer.h>
#include "config.h"
#include "clipboard_manager.h"

using Poco::Net::SocketAddress;
using Poco::Net::DatagramSocket;

class Broadcaster: public Poco::Runnable
{
public:
    Broadcaster(Config *conf, ClipboardManager *manager);
    void start();
    void onTimer(Poco::Timer &timer);
    void run();

private:
    void treatMsg(SocketAddress &src, std::string msg);

    Config *conf;
    ClipboardManager *manager;
    SocketAddress bcastAddr, srcAddr;
    DatagramSocket s1;
    DatagramSocket s2;
    Poco::ThreadPool pool;
    Poco::Timer t;
    bool verbose;
};

#endif
