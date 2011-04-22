#ifndef DEF_HANDLER_H
#define DEF_HANDLER_H

#include <map>
#include <Poco/Runnable.h>
#include <Poco/Mutex.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/ThreadPool.h>
#include <Poco/Timer.h>
#include <vector>
#include "config.h"

using Poco::Net::StreamSocket;
using Poco::Net::ServerSocket;

class PeerHandler;

class PeerManager: public Poco::Runnable
{
public:
    PeerManager(Config *conf);
    void start();
    void run();
    void contact(SocketAddress &addr, std::string peerName);
    void removePeer(PeerHandler *handler, std::string &peerName);
    bool addPeer(PeerHandler *handler, std::string &peerName);

private:
    Config *conf;
    ServerSocket ssock;
    std::vector<PeerHandler*> handlers;
    std::map<std::string,PeerHandler*> peers;
    Poco::ThreadPool pool;
    Poco::Mutex mutex;
    bool verbose;
};

class PeerHandler: public Poco::Runnable
{
public:
    PeerHandler(Config *conf, Poco::ThreadPool *pool, PeerManager *manager,
                StreamSocket &sock, bool initiator);
    void run();
    void sendClose();
    bool compare(PeerHandler *other);

private:
    void sendMsg(std::string msg);
    void sendJoin();
    void sendKo(int error);
    void close();
    std::string getInitiatorName();
    void treatMsg(std::string msg);
    void treatOk();
    void treatKo(int error);
    void addPeer();
    void verifyAccept(int nbr);
    void treatJoin(std::string peerName, int nbr);
    void onTimer1(Poco::Timer &timer);
    void onTimer2(Poco::Timer &timer);

    Config *conf;
    Poco::ThreadPool *pool;
    PeerManager *manager;
    StreamSocket sock;
    std::string peerName;
    int challenge;
    Poco::Timer t1, t2;
    bool isRunning;
    bool initiator;
    bool acceptSent, acceptVerified;
    bool verbose;
};

/* class PeerFactory: public TCPServerConnectionFactory */
/* { */
/* public: */
/*     PeerFactory(Config *conf, Poco::ThreadPool *pool, PeerManager *manager); */
/*     TCPServerConnection* createConnection(const StreamSocket &socket); */
/* private: */
/*     Config *conf; */
/*     Poco::ThreadPool *pool; */
/*     PeerManager *manager; */
/* }; */

/* class PeerConnection: public TCPServerConnection */
/* { */
/* public: */
/*     PeerConnection(const StreamSocket &socket, Config *conf, */
/*                    Poco::ThreadPool *pool, PeerManager *manager); */
/*     void run(); */

/* private: */
/*     PeerHandler handler; */
/* }; */

/* class PeerHandler */
/* { */
/* public: */
/*     PeerHandler(const StreamSocket *socket, Config *conf, */
/*                 Poco::ThreadPool *pool, PeerManager *manager, */
/*                 bool initiator=false); */
/*     /\* bool compare(PeerHandler *other); *\/ */
/*     void sendClose(); */

/* private: */
/*     std::string getInitiatorName(); */
/*     /\* void onTimer1(Poco::Timer &timer); *\/ */
/*     /\* void onTimer2(Poco::Timer &timer); *\/ */
/*     void sendMsg(std::string msg); */
/*     /\* void treatMsg(std::string msg); *\/ */
/*     /\* void treatOk(); *\/ */
/*     void treatKo(int error); */
/*     void sendJoin(); */
/*     /\* void addPeer(); *\/ */
/*     void verifyAccept(int nbr); */
/*     void treatJoin(std::string peerName, int nbr); */
/*     void sendKo(int error); */
/*     void close(); */

/*     StreamSocket socket; */
/*     SocketAddress addr; */
/*     Config *conf; */
/*     Poco::ThreadPool *pool; */
/*     PeerManager *manager; */
/*     /\* Poco::Timer t1, t2; *\/ */
/*     std::string peerName; */
/*     int challenge; */
/*     bool initiator; */
/*     bool isRunning; */
/*     /\* bool acceptVerified; *\/ */
/*     /\* bool acceptSent; *\/ */
/*     bool verbose; */
/* }; */

/* class PeerManager: public TCPServer */
/* { */
/* public: */
/*     PeerManager(Config *conf); */
/*     void contact(SocketAddress &addr, std::string peerName); */
/*     /\* void removePeer(PeerHandler &peer, std::string &peerName); *\/ */
/*     /\* bool addPeer(PeerHandler &peer, std::string &peerName); *\/ */
/* private: */
/*     Config *conf; */
/*     Poco::ThreadPool pool; */
/*     std::map<std::string,PeerHandler*> peers; */
/*     /\* Poco::Mutex lock; *\/ */
/* }; */

#endif
