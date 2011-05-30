/*
  Clipsync, clipboard synchronizer
  Copyright (C) 2011 Maëlick Claes (himself [at] maelick [dot] net)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "clipboard_manager.h"

#include <iostream>

using namespace Poco::Net;
using namespace std;

ClipboardManager::ClipboardManager(Config *conf):
    conf(conf),
    ssock(conf->getAddress()),
    pool(),
    localManager(conf, &this->pool, this),
    verbose(conf->getVerboseNet())
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
}

bool ClipboardManager::addPeer(PeerHandler *handler, string &peerName)
{
    this->peerMutex.lock();
    bool result = true;
    if(this->peers.count(peerName) > 0) {
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
    if(this->verbose) {
        cout << "Clipboard modified from distant peer:" << data << endl;
    }
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
    if(this->verbose) {
        cout << "Clipboard modified from local: " << data << endl;
    }
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
