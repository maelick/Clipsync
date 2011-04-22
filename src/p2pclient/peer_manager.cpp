#include "peer_manager.h"

#include <iostream>

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
