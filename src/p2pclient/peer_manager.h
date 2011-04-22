#ifndef DEF_PEER_MANAGER_H
#define DEF_PEER_MANAGER_H

#include <map>
#include <Poco/Mutex.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>
#include <vector>

#include "config.h"
#include "peer_handler.h"

using Poco::Net::ServerSocket;

class PeerManager: public Poco::Runnable
{
public:
    PeerManager(Config *conf);
    void start();
    void run();
    void contact(SocketAddress &addr, std::string peerName);
    void removePeer(PeerHandler *handler, std::string &peerName);
    bool addPeer(PeerHandler *handler, std::string &peerName);
    void syncClipboard(std::string data);
    void setClipboard(std::string data);

private:
    Config *conf;
    ServerSocket ssock;
    std::vector<PeerHandler*> handlers;
    std::map<std::string,PeerHandler*> peers;
    Poco::ThreadPool pool;
    Poco::Mutex mutex;
    std::string clipboard;
    bool verbose;
};

#endif
