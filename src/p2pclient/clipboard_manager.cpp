#include "clipboard_manager.h"

#include <iostream>

using namespace Poco::Net;
using namespace std;

ClipboardManager::ClipboardManager(Config *conf):
    conf(conf),
    ssock(conf->getAddress()),
    pool(),
    localManager(conf, &this->pool, this),
    verbose(conf->getBool("p2p_client.verbose"))
{
}

void ClipboardManager::start()
{
    this->pool.start(this->localManager);
    this->pool.start(*this);
}

void ClipboardManager::run()
{
    while(true) {
        SocketAddress addr;
        StreamSocket sock = this->ssock.acceptConnection(addr);
        PeerHandler *handler =  new PeerHandler(this->conf, &this->pool,
                                                this, sock, false);
        this->peerMutex.lock();
        this->handlers.push_back(handler);
        this->peerMutex.unlock();
        if(this->verbose) {
            cout << "Peer connected" << endl;
        }
        this->pool.start(*handler);
    }
}

void ClipboardManager::contact(SocketAddress &addr, string peerName)
{
    if(!this->peers.count(peerName)) {
        StreamSocket sock(addr);
        PeerHandler *handler = new PeerHandler(this->conf, &this->pool,
                                               this, sock, true);
        this->peerMutex.lock();
        this->handlers.push_back(handler);
        this->peerMutex.unlock();
        if(this->verbose) {
            cout << "Contacting " << peerName << " on address "
                 << addr.toString() << endl;
        }
        this->pool.start(*handler);
    }
}

void ClipboardManager::removeLocal(LocalHandler *handler)
{
    this->localMutex.lock();
    vector<LocalHandler*>::iterator it =
        find(this->localHandlers.begin(), this->localHandlers.end(), handler);
    if(it < this->localHandlers.end()) {
        this->localHandlers.erase(it);
    }
    this->localMutex.unlock();
    delete handler;
}

void ClipboardManager::addLocal(LocalHandler *handler)
{
    this->localMutex.lock();
    this->localHandlers.push_back(handler);
    this->localMutex.unlock();
}

void ClipboardManager::removePeer(PeerHandler *handler, string &peerName)
{
    this->peerMutex.lock();
    vector<PeerHandler*>::iterator it =
        find(this->handlers.begin(), this->handlers.end(), handler);
    if(it < this->handlers.end()) {
        this->handlers.erase(it);
    }
    if(this->peers.count(peerName) && this->peers[peerName] == handler) {
        this->peers.erase(this->peers.find(peerName));
    }
    this->peerMutex.unlock();
    delete handler;
}

bool ClipboardManager::addPeer(PeerHandler *handler, string &peerName)
{
    this->peerMutex.lock();
    bool result = true;
    if(this->peers.count(peerName)) {
        if(handler->compare(this->peers[peerName])) {
            this->peers[peerName]->sendClose(0);
            this->peers[peerName] = handler;
        } else {
            handler->sendClose(0);
            result = false;
        }
    } else {
        this->peers[peerName] = handler;
    }
    this->peerMutex.unlock();
    return result;
}

string ClipboardManager::getClipboard()
{
    this->clipMutex.lock();
    string data = this->clipboard;
    this->clipMutex.unlock();
    return data;
}

void ClipboardManager::syncClipboard(string data)
{
    this->clipMutex.lock();
    this->clipboard = data;
    for(vector<LocalHandler*>::iterator it = this->localHandlers.begin();
        it < this->localHandlers.end(); it++) {
        (*it)->sendClipboard(data);
    }
    this->clipMutex.unlock();
}

void ClipboardManager::setClipboard(LocalHandler *handler, string data)
{
    this->clipMutex.lock();
    this->clipboard = data;
    for(map<string,PeerHandler*>::iterator it = this->peers.begin();
        it != this->peers.end(); it++) {
        it->second->sendData(this->clipboard);
    }
    for(vector<LocalHandler*>::iterator it = this->localHandlers.begin();
        it < this->localHandlers.end(); it++) {
        if((*it) != handler) {
            (*it)->sendClipboard(data);
        }
    }
    this->clipMutex.unlock();
}
