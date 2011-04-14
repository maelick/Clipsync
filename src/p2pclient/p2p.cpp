#include "p2p.h"

using namespace std;

P2PClient::P2PClient(string &confFile):
    conf(confFile),
    manager(this->conf),
    b(this->conf, this->manager)
{
}

Config& P2PClient::getConfig()
{
    return this->conf;
}

void P2PClient::start()
{
    this->manager.start();
    this->b.start();
}
