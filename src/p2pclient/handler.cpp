#include "handler.h"

#include <Poco/RegularExpression.h>
#include <vector>
#include <iostream>
#include <sstream>

using Poco::TimerCallback;
using Poco::RegularExpression;
using namespace Poco::Net;
using namespace std;

PeerManager::PeerManager(Config *conf):
    conf(conf),
    ssock(conf->getAddress()),
    pool(),
    verbose(conf->getBool("p2p_client.verbose"))
{
}

void PeerManager::start()
{
    this->pool.start(*this);
}

void PeerManager::run()
{
    while(true) {
        SocketAddress addr;
        StreamSocket sock = this->ssock.acceptConnection(addr);
        PeerHandler *handler =  new PeerHandler(this->conf, &this->pool,
                                                this, sock, false);
        this->mutex.lock();
        this->handlers.push_back(handler);
        this->mutex.unlock();
        if(this->verbose) {
            cout << "Peer connected" << endl;
        }
        this->pool.start(*handler);
    }
}

void PeerManager::contact(SocketAddress &addr, string peerName)
{
    if(!this->peers.count(peerName)) {
        StreamSocket sock(addr);
        PeerHandler *handler = new PeerHandler(this->conf, &this->pool,
                                               this, sock, true);
        this->mutex.lock();
        this->handlers.push_back(handler);
        this->mutex.unlock();
        if(this->verbose) {
            cout << "Contacting " << peerName << " on address "
                 << addr.toString() << endl;
        }
        this->pool.start(*handler);
    }
}

void PeerManager::removePeer(PeerHandler *handler, string &peerName)
{
    this->mutex.lock();
    vector<PeerHandler*>::iterator it =
        find(this->handlers.begin(), this->handlers.end(), handler);
    if(it < this->handlers.end()) {
        this->handlers.erase(it);
    }
    if(this->peers.count(peerName) && this->peers[peerName] == handler) {
        this->peers.erase(this->peers.find(peerName));
    }
    this->mutex.unlock();
}

bool PeerManager::addPeer(PeerHandler *handler, string &peerName)
{
    this->mutex.lock();
    bool result = true;
    if(this->peers.count(peerName)) {
        if(handler->compare(this->peers[peerName])) {
            this->peers[peerName]->sendClose();
            this->peers[peerName] = handler;
        } else {
            handler->sendClose();
            result = false;
        }
    } else {
        this->peers[peerName] = handler;
    }
    this->mutex.unlock();
    return result;
}

PeerHandler::PeerHandler(Config *conf, Poco::ThreadPool *pool,
                         PeerManager *manager, StreamSocket &sock,
                         bool initiator):
    conf(conf),
    pool(pool),
    manager(manager),
    sock(sock),
    peerName(""),
    challenge(conf->getChallenge()),
    t1(conf->getInt("p2p_client.keepalive_delay")),
    t2(conf->getInt("p2p_client.keepalive_interval")),
    initiator(initiator),
    verbose(conf->getBool("p2p_client.verbose"))
{
}

void PeerHandler::run()
{
    this->isRunning = true;
    this->sendJoin();
    while(this->isRunning) {
        char buffer[1024];
        int n = this->sock.receiveBytes(buffer, sizeof(buffer));
        if(n > 0) {
            string s(buffer);
            this->treatMsg(s);
        } else {
            this->isRunning = false;
        }
    }
}

void PeerHandler::sendMsg(string msg)
{
    if(this->verbose) {
        cout << "Message: " << msg << " sent to peer " << this->peerName
             << " on address " << this->sock.peerAddress().toString()
             << endl;
    }
    this->sock.sendBytes(msg.c_str(), msg.size());
}

void PeerHandler::sendClose()
{
    this->sendKo(0);
    this->close();
}

void PeerHandler::sendJoin()
{
    ostringstream oss;
    oss << "JOIN " << this->challenge << endl;
    this->sendMsg(oss.str());
    this->t1.start(TimerCallback<PeerHandler>(*this, &PeerHandler::onTimer1),
                  *this->pool);
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
             << this->sock.peerAddress().toString();
    }
    this->manager->removePeer(this, this->peerName);
    this->isRunning = false;
    this->t1.stop();
    this->t2.stop();
}

string PeerHandler::getInitiatorName()
{
    return this->initiator ? this->conf->getString("p2p_client.peer_name") :
        this->peerName;
}

bool PeerHandler::compare(PeerHandler *other)
{
    return this->getInitiatorName().compare(other->getInitiatorName()) < 0;
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
    if(this->verbose) {
        cout << "Message received from peer " << this->peerName
             << " on address " << this->sock.peerAddress().toString()
             << endl;
    }
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

void PeerHandler::addPeer()
{
    if(this->acceptSent && this->acceptVerified) {
        this->manager->addPeer(this, this->peerName);
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

void PeerHandler::treatJoin(string peerName, int nbr)
{
    this->peerName = peerName;
    ostringstream oss;
    oss << "ACCEPT " << nbr + 1 << endl;
    this->sendMsg(oss.str());
    this->acceptSent = true;
    this->t2.start(TimerCallback<PeerHandler>(*this, &PeerHandler::onTimer2),
                   *this->pool);
    this->addPeer();
}

void PeerHandler::onTimer1(Poco::Timer &timer)
{
    this->sendClose();
}

void PeerHandler::onTimer2(Poco::Timer &timer)
{
    this->sendMsg("OK\n");
}
