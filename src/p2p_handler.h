#ifndef DEF_P2P_HANDLER_H
#define DEF_P2P_HANDLER_H

#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include "config.h"

using Poco::Net::StreamSocket;
using Poco::Net::TCPServer;
using Poco::Net::TCPServerConnection;
using Poco::Net::TCPServerConnectionFactory;

class PeerFactory: public TCPServerConnectionFactory
{
public:
    PeerFactory(Config &conf);
    TCPServerConnection* createConnection(const StreamSocket &socket);
private:
    Config &conf;
};

class PeerHandler: public TCPServerConnection
{
public:
    PeerHandler(const StreamSocket &socket, Config &conf);
    void run();
private:
    void sendMsg(std::string msg);
    void treatMsg(std::string msg);
    void treatOk();
    void treatKo(int error);
    void sendJoin();
    void verifyAccept(int nbr);
    void treatJoin(std::string peerName, int nbr);
    void sendClose();
    void close();
    Config &conf;
    int challenge;
    bool isRunning;
    bool ready;
};

class PeerManager: public TCPServer
{
public:
    PeerManager(Config &conf);
private:
    Config &conf;
};

#endif
