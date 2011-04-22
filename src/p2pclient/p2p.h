#ifndef DEF_P2P_H
#define DEF_P2P_H

#include <Poco/ThreadPool.h>
#include "config.h"
#include "broadcaster.h"
#include "clipboard_manager.h"

class P2PClient
{
public:
    P2PClient(std::string &confFile);
    ~P2PClient();
    Config* getConfig();
    void start();

private:
    Config *conf;
    ClipboardManager *manager;
    Broadcaster *b;
};

#endif
