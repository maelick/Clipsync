#ifndef DEF_HANDLER_H
#define DEF_HANDLER_H

#include <map>
#include <Poco/Mutex.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/ThreadPool.h>
#include <Poco/Timer.h>
#include "config.h"

using Poco::Net::StreamSocket;
using Poco::Net::TCPServer;
using Poco::Net::TCPServerConnection;
using Poco::Net::TCPServerConnectionFactory;

class PeerManager;

class PeerFactory: public TCPServerConnectionFactory
{
public:
    PeerFactory(Config &conf, Poco::ThreadPool &pool, PeerManager &manager);
    TCPServerConnection* createConnection(const StreamSocket &socket);
private:
    Config &conf;
    Poco::ThreadPool &pool;
    PeerManager &manager;
};

class PeerHandler: public TCPServerConnection
{
public:
    PeerHandler(const StreamSocket &socket, Config &conf,
                Poco::ThreadPool &pool, PeerManager &manager);
    void run();
    bool compare(PeerHandler *other);
    void sendClose();
private:
    void onTimer1(Poco::Timer &timer);
    void onTimer2(Poco::Timer &timer);
    void sendMsg(std::string msg);
    void treatMsg(std::string msg);
    void treatOk();
    void treatKo(int error);
    void sendJoin();
    void addPeer();
    void verifyAccept(int nbr);
    void treatJoin(std::string peerName, int nbr);
    void sendKo(int error);
    void close();

    Config &conf;
    Poco::ThreadPool &pool;
    PeerManager &manager;
    Poco::Timer t1, t2;
    std::string peerName;
    int challenge;
    bool isRunning;
    bool acceptVerified;
    bool acceptSent;
};

class PeerManager: public TCPServer
{
public:
    PeerManager(Config &conf);
    void contact(SocketAddress &addr, std::string peerName);
    void removePeer(PeerHandler &peer, std::string &peerName);
    bool addPeer(PeerHandler &peer, std::string &peerName);
private:
    Config &conf;
    Poco::ThreadPool pool;
    std::map<std::string,PeerHandler*> peers;
    Poco::Mutex lock;
};

#endif
