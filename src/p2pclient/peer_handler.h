#ifndef DEF_PEER_HANDLER_H
#define DEF_PEER_HANDLER_H

#include <Poco/Net/StreamSocket.h>
#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>
#include <Poco/Timer.h>

#include "config.h"

using Poco::Net::StreamSocket;

class ClipboardManager;

class PeerHandler: public Poco::Runnable
{
public:
    PeerHandler(Config *conf, Poco::ThreadPool *pool, ClipboardManager *manager,
                StreamSocket &sock, bool initiator);
    void run();
    void sendClose(int reason);
    bool compare(PeerHandler *other);
    void sendData(std::string data);

private:
    void sendMsg(std::string msg);
    void sendJoin();
    void sendKo(int error);
    void close();
    std::string getInitiatorName();
    void treatMsg(std::string msg);
    void treatData(int length, std::string data);
    void treatOk();
    void treatKo(int error);
    void addPeer();
    void verifyAccept(int nbr);
    void treatJoin(std::string peerName, int nbr);
    void onTimer1(Poco::Timer &timer);
    void onTimer2(Poco::Timer &timer);

    Config *conf;
    Poco::ThreadPool *pool;
    ClipboardManager *manager;
    StreamSocket sock;
    std::string peerName;
    int challenge;
    Poco::Timer t1, t2;
    bool isRunning;
    bool initiator;
    bool acceptSent, acceptVerified;
    bool verbose;
};

#endif
