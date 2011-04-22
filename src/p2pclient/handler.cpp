#include "handler.h"

#include <Poco/RegularExpression.h>
#include <vector>
#include <iostream>
#include <sstream>

using Poco::TimerCallback;
using Poco::RegularExpression;
using namespace Poco::Net;
using namespace std;

PeerFactory::PeerFactory(Config &conf, Poco::ThreadPool &pool,
                         PeerManager &manager):
    conf(conf),
    pool(pool),
    manager(manager)
{
}

TCPServerConnection* PeerFactory::createConnection(const StreamSocket &socket)
{
    return new PeerHandler(socket, this->conf, this->pool, this->manager);
}

PeerHandler::PeerHandler(const StreamSocket &socket, Config &conf,
                         Poco::ThreadPool &pool, PeerManager &manager,
                         bool initiator):
    TCPServerConnection(socket),
    conf(conf),
    pool(pool),
    manager(manager),
    peerName(""),
    t1(conf.getInt("p2p_client.keepalive_delay")),
    t2(conf.getInt("p2p_client.keepalive_interval")),
    challenge(conf.getChallenge()),
    initiator(initiator),
    isRunning(false),
    acceptVerified(false),
    acceptSent(false),
    verbose(conf.getBool("p2p_client.verbose"))
{
}

void PeerHandler::run()
{
    this->isRunning = true;
    StreamSocket &socket = this->socket();
    char buffer[1024];
    int n = socket.receiveBytes(buffer, sizeof(buffer));
    string s(buffer);
    this->treatMsg(s);
}

bool PeerHandler::compare(PeerHandler *other)
{
    string distantName = this->conf.getString("p2p_client.peer_name");
    int cmpValue = distantName.compare(this->peerName);
    return this->initiator ? cmpValue > 0 : cmpValue < 0;
}

void PeerHandler::sendClose()
{
    this->sendKo(0);
    this->close();
}

void PeerHandler::sendMsg(string msg)
{
    this->socket().sendBytes(msg.c_str(), msg.size());
}

int getInt(string s)
{
    istringstream iss(s);
    int n;
    iss >> n;
    return n;
}

void PeerHandler::treatMsg(string msg)
{
    RegularExpression joinMsg("^JOIN ([a-z0-9]+) (-?[0-9]+).*",
                              RegularExpression::RE_DOTALL);
    RegularExpression acceptMsg("^ACCEPT (-?[0-9]+).*",
                                RegularExpression::RE_DOTALL);
    RegularExpression okMsg("^OK.*",
                            RegularExpression::RE_DOTALL);
    RegularExpression koMsg("^KO ([0-9]+).*",
                            RegularExpression::RE_DOTALL);

    vector<string> v;

    if(joinMsg.match(msg)) {
        joinMsg.split(msg, v);
        this->treatJoin(v[1], getInt(v[2]));
    } else if(acceptMsg.match(msg)) {
        acceptMsg.split(msg, v);
        this->verifyAccept(getInt(v[1]));
    } else if(okMsg.match(msg)) {
        this->treatOk();
    } else if(koMsg.match(msg)) {
        koMsg.split(msg, v);
        this->treatKo(getInt(v[1]));
    }
}

void PeerHandler::treatOk()
{
    this->t1.restart();
}

void PeerHandler::treatKo(int error)
{
    switch(error){
    case 0: // Close
        this->close();
        break;
    }
}

void PeerHandler::sendJoin()
{
    ostringstream oss;
    oss << "JOIN " << this->challenge << endl;
    this->sendMsg(oss.str());
    this->t1.start(TimerCallback<PeerHandler>(*this, &PeerHandler::onTimer1),
                  this->pool);
}

void PeerHandler::onTimer1(Poco::Timer &timer)
{
    this->sendClose();
}

void PeerHandler::addPeer()
{
    if(this->acceptSent && this->acceptVerified) {
        this->manager.addPeer(*this, this->peerName);
    }
}

void PeerHandler::verifyAccept(int nbr)
{
    if(nbr == this->challenge + 1) {
        this->t1.restart();
        this->acceptVerified = true;
        this->addPeer();
    } else {
        this->sendClose();
    }
}

void PeerHandler::onTimer2(Poco::Timer &timer)
{
    this->sendMsg("OK\n");
}

void PeerHandler::treatJoin(string peerName, int nbr)
{
    this->peerName = peerName;
    ostringstream oss;
    oss << "ACCEPT " << nbr + 1 << endl;
    this->sendMsg(oss.str());
    this->acceptSent = true;
    this->t2.start(TimerCallback<PeerHandler>(*this, &PeerHandler::onTimer2),
                   this->pool);
    this->addPeer();
}

void PeerHandler::sendKo(int error)
{
    ostringstream oss;
    oss << "KO " << error << endl;
    this->sendMsg(oss.str());
}

void PeerHandler::close()
{
    if(this->verbose) {
        cout << "Closing connection with " << this->peerName << " on address "
             << this->socket().peerAddress().toString();
    }
    this->manager.removePeer(*this, this->peerName);
    this->isRunning = false;
    this->t1.stop();
    this->t2.stop();
}

PeerManager::PeerManager(Config &conf):
    conf(conf),
    pool(),
    peers(),
    lock(),
    TCPServer(new PeerFactory(conf, this->pool, *this),
              conf.getAddress())
{
}

void PeerManager::contact(SocketAddress &addr, string peerName)
{
    if(!this->peers.count(peerName)) {
        StreamSocket socket(addr);
        PeerHandler handler(socket, this->conf, this->pool, *this, true);
        if(this->conf.getBool("p2p_client.verbose")) {
            cout << "Contacting " << peerName << " on address "
                 << addr.toString() << endl;
        }
        this->pool.start(handler);
    }
}

void PeerManager::removePeer(PeerHandler &handler, string &peerName)
{
    this->lock.lock();
    if(this->peers.count(peerName) && this->peers[peerName] == &handler) {
        this->peers.erase(this->peers.find(peerName));
    }
    this->lock.unlock();
}

bool PeerManager::addPeer(PeerHandler &handler, string &peerName)
{
    this->lock.lock();
    bool result = true;
    if(this->peers.count(peerName)) {
        if(handler.compare(this->peers[peerName])) {
            this->peers[peerName]->sendClose();
            this->peers[peerName] = &handler;
        } else {
            handler.sendClose();
            result = false;
        }
    } else {
        this->peers[peerName] = &handler;
    }
    this->lock.unlock();
    return result;
}
