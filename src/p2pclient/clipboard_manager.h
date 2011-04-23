#ifndef DEF_CLIPBOARD_MANAGER_H
#define DEF_CLIPBOARD_MANAGER_H

#include <map>
#include <Poco/Mutex.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>
#include <vector>

#include "config.h"
#include "peer_handler.h"
#include "local_frontend.h"

using Poco::Net::ServerSocket;

class ClipboardManager: public Poco::Runnable
{
public:
    ClipboardManager(Config *conf);
    void start();
    void run();
    void contact(SocketAddress &addr, std::string peerName);
    void removeLocal(LocalHandler *handler);
    void addLocal(LocalHandler *handler);
    void removePeer(PeerHandler *handler, std::string &peerName);
    bool addPeer(PeerHandler *handler, std::string &peerName);
    std::string getClipboard();
    void syncClipboard(std::string data);
    void setClipboard(LocalHandler *handler, std::string data);

private:
    Config *conf;
    ServerSocket ssock;
    std::vector<PeerHandler*> handlers;
    std::vector<LocalHandler*> localHandlers;
    std::map<std::string,PeerHandler*> peers;
    Poco::ThreadPool pool;
    LocalManager localManager;
    Poco::Mutex peerMutex;
    Poco::Mutex localMutex;
    Poco::Mutex clipMutex;
    std::string clipboard;
    bool verbose;
};

#endif
