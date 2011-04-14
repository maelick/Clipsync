#ifndef DEF_HANDLER_H
#define DEF_HANDLER_H

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

class PeerFactory: public TCPServerConnectionFactory
{
public:
    PeerFactory(Config &conf, Poco::ThreadPool &pool);
    TCPServerConnection* createConnection(const StreamSocket &socket);
private:
    Config &conf;
    Poco::ThreadPool &pool;
};

class PeerHandler: public TCPServerConnection
{
public:
    PeerHandler(const StreamSocket &socket, Config &conf,
                Poco::ThreadPool &pool);
    void run();
private:
    void onTimer1(Poco::Timer &timer);
    void onTimer2(Poco::Timer &timer);
    void sendMsg(std::string msg);
    void treatMsg(std::string msg);
    void treatOk();
    void treatKo(int error);
    void sendJoin();
    void verifyAccept(int nbr);
    void treatJoin(std::string peerName, int nbr);
    void sendKo(int error);
    void sendClose();
    void close();

    Config &conf;
    Poco::ThreadPool &pool;
    Poco::Timer t1, t2;
    int challenge;
    bool isRunning;
    bool ready;
};

class PeerManager: public TCPServer
{
public:
    PeerManager(Config &conf);
    void contact(SocketAddress addr);
private:
    Config &conf;
    Poco::ThreadPool pool;
};

#endif
