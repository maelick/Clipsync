#ifndef DEF_P2P_H
#define DEF_P2P_H

#include <Poco/ThreadPool.h>
#include "config.h"
#include "broadcaster.h"
#include "handler.h"

class P2PClient
{
public:
    P2PClient(std::string &confFile);
    Config& getConfig();
    void start();

private:
    Config conf;
    PeerManager manager;
    Broadcaster b;
};

#endif
