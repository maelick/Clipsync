#include "p2p.h"

using namespace std;

P2PClient::P2PClient(string &confFile):
    conf(confFile)
{
    this->b = new Broadcaster(this->conf);
}

P2PClient::~P2PClient()
{
    delete this->b;
}

Config& P2PClient::getConfig()
{
    return this->conf;
}

void P2PClient::start(Poco::ThreadPool &pool)
{
    this->b->start(pool);
}
