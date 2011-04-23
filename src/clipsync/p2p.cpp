#include "p2p.h"

using namespace std;

P2PClient::P2PClient(string &confFile):
    conf(new Config(confFile))
{
    this->manager = new ClipboardManager(this->conf);
    this->b = new Broadcaster(this->conf, this->manager);
}

P2PClient::~P2PClient()
{
    delete this->conf;
    delete this->manager;
    delete this->b;
}

Config* P2PClient::getConfig()
{
    return this->conf;
}

void P2PClient::start()
{
    this->manager->start();
    this->b->start();
}
