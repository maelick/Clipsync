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

#include "peer_handler.h"

#include <Poco/RegularExpression.h>
#include <vector>
#include <iostream>

#include "clipboard_manager.h"

using Poco::TimerCallback;
using Poco::RegularExpression;
using namespace Poco::Net;
using namespace std;

PeerHandler::PeerHandler(Config *conf, Poco::ThreadPool *pool,
                         ClipboardManager *manager, DialogSocket *sock,
                         bool initiator):
    conf(conf),
    pool(pool),
    manager(manager),
    sock(sock),
    peerName(""),
    challenge(conf->getChallenge()),
    t1(conf->getKeepaliveDelay(),
       conf->getKeepaliveDelay()),
    t2(0, conf->getKeepaliveInterval()),
    initiator(initiator),
    verbose(conf->getVerbosePeer())
{
}

PeerHandler::~PeerHandler()
{
    delete this->sock;
}

void PeerHandler::run()
{
    this->acceptSent = false;
    this->acceptVerified = false;
    this->isRunning = true;
    this->sendJoin();
    while(this->isRunning) {
        string s = "";
        if(this->sock->receiveMessage(s)) {
            this->treatMsg(this->conf->decrypt(s));
        } else {
            this->close();
        }
    }
    delete this;
}

void PeerHandler::sendMsg(string msg)
{
    this->sock->sendMessage(this->conf->encrypt(msg));
}

void PeerHandler::sendClose(int reason)
{
    this->sendKo(reason);
    this->close();
}

void PeerHandler::sendJoin()
{
    ostringstream oss;
    oss << "JOIN " << this->conf->getPeerName()
        << " " << this->challenge;
    this->t1.start(TimerCallback<PeerHandler>(*this, &PeerHandler::onTimer1),
                  *this->pool);
    this->sendMsg(oss.str());
}

void PeerHandler::sendKo(int error)
{
    ostringstream oss;
    oss << "KO " << error;
    this->sendMsg(oss.str());
}

void PeerHandler::close()
{
    if(this->verbose) {
        cout << "Closing connection with " << this->peerName << " on address "
             << this->sock->peerAddress().toString() << endl;
    }
    this->isRunning = false;
    this->t1.restart(0);
    this->t2.restart(0);
    this->manager->removePeer(this, this->peerName);
}

string PeerHandler::getInitiatorName()
{
    return this->initiator ? this->conf->getPeerName() :
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
    RegularExpression joinMsg("^JOIN ([A-Za-z0-9]+) (-?[0-9]+).*",
                              RegularExpression::RE_DOTALL);
    RegularExpression acceptMsg("^ACCEPT (-?[0-9]+).*",
                                RegularExpression::RE_DOTALL);
    RegularExpression okMsg("^OK.*",
                            RegularExpression::RE_DOTALL);
    RegularExpression koMsg("^KO ([0-9]+).*",
                            RegularExpression::RE_DOTALL);
    RegularExpression dataMsg("^DATA ([0-9]+) ([01]) ([0-9]+) (.*)",
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
    } else if(dataMsg.match(msg) && this->acceptSent && this->acceptVerified) {
        dataMsg.split(msg, v);
        this->treatData(getInt(v[1]), getInt(v[2]), getInt(v[3]), v[4]);
    } else if(this->verbose) {
        cout << "Unknown message received from peer " << this->peerName
             << " on address " << this->sock->peerAddress().toString()
             << endl << msg << endl;;
    }
}

void PeerHandler::treatData(int type, int hasMore, int length, string data)
{
    if(type != 0) {
        if(this->verbose) {
            cout << "Unable to treat datatype " << type << endl;
        }
        this->sendKo(3);
        return;
    }

    this->buf << data.substr(0, length);
    if(!hasMore) {
        this->manager->syncClipboard(this->buf.str());
        this->buf.str("");
    }
}

void split(string data, vector<string> &v)
{
    int n = data.size();
    int i = 0;
    do {
        v.push_back(data.substr(i, 4096));
        i += 4096;
    } while(i < n);
}

void PeerHandler::sendData(string data)
{
    vector<string> v;
    split(data, v);

    for(int i = 0; i < v.size(); i++) {
        ostringstream oss;
        oss << "DATA 0 " << (i < v.size() - 1 ) << " " << v[i].size()
            << " " << v[i];
        this->sendMsg(oss.str());
    }
}

void PeerHandler::treatOk()
{
    if(this->acceptVerified) {
        this->t1.restart();
    }
}

void PeerHandler::treatKo(int error)
{
    switch(error){
    case 0: // Close
        if(this->verbose) {
            cout << "Peer " << this->peerName << " closed the connection."
                 << endl;
        }
        break;
        this->close();
    case 1: // Timeout
        if(this->verbose) {
            cout << "Peer " << this->peerName << " closed the connection "
                 << "due to a timeout." << endl;;
        }
        this->close();
        break;
    case 2: // Invalid accept
        if(this->verbose) {
            cout << "Peer " << this->peerName << " closed the connection "
                 << "due to an invalid accept." << endl;;
        }
        this->close();
        break;
    case 3: // Unsupported type
        if(this->verbose) {
            cout << "Peer " << this->peerName << " cannot handle datatype."
                 << endl;
        }
        break;
    }
}

void PeerHandler::addPeer()
{
    if(this->acceptSent && this->acceptVerified) {
        if(this->verbose) {
            cout << "Peer " << this->peerName << " verified." << endl;
        }
        this->manager->addPeer(this, this->peerName);
    }
}

void PeerHandler::verifyAccept(int nbr)
{
    if(nbr == this->challenge + 1) {
        if(this->verbose) {
            cout << "Accept from peer " << this->peerName << " verified"
                 << endl;
        }
        this->t1.restart();
        this->acceptVerified = true;
        this->addPeer();
    } else {
        if(this->verbose) {
            cout << "Accept from peer " << this->peerName << " not verified"
                 << endl;
        }
        this->sendClose(2);
    }
}

void PeerHandler::treatJoin(string peerName, int nbr)
{
    if(this->verbose) {
        cout << "Accept sent to peer " << this->peerName << endl;
    }
    this->peerName = peerName;
    ostringstream oss;
    oss << "ACCEPT " << nbr + 1;
    this->sendMsg(oss.str());
    this->acceptSent = true;
    this->t2.start(TimerCallback<PeerHandler>(*this, &PeerHandler::onTimer2),
                   *this->pool);
    this->addPeer();
}

void PeerHandler::onTimer1(Poco::Timer &timer)
{
    if(this->verbose) {
        cout << "Timeout for peer " << this->peerName << endl;
    }
    this->sendClose(1);
    this->t1.restart(0);
}

void PeerHandler::onTimer2(Poco::Timer &timer)
{
    this->sendMsg("OK");
}
