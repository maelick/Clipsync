#include "peer_handler.h"

#include <Poco/RegularExpression.h>
#include <vector>
#include <iostream>
#include <sstream>

#include "peer_manager.h"

using Poco::TimerCallback;
using Poco::RegularExpression;
using namespace Poco::Net;
using namespace std;

PeerHandler::PeerHandler(Config *conf, Poco::ThreadPool *pool,
                         PeerManager *manager, StreamSocket &sock,
                         bool initiator):
    conf(conf),
    pool(pool),
    manager(manager),
    sock(sock),
    peerName(""),
    challenge(conf->getChallenge()),
    t1(conf->getInt("p2p_client.keepalive_delay"),
       conf->getInt("p2p_client.keepalive_delay")),
    t2(0, conf->getInt("p2p_client.keepalive_interval")),
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
            this->close();
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

void PeerHandler::sendClose(int reason)
{
    this->sendKo(reason);
    this->close();
}

void PeerHandler::sendJoin()
{
    ostringstream oss;
    oss << "JOIN " << this->conf->getString("p2p_client.peer_name")
        << " " << this->challenge << endl;
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
    } else if(this->verbose) {
        cout << "Unknown message received from peer " << this->peerName
             << " on address " << this->sock.peerAddress().toString()
             << endl;
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
    case 1: // Timeout
        this->close();
        break;
    case 2: // Invalid accept
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
        this->sendClose(2);
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
    this->t1.stop();
    this->sendClose(1);
}

void PeerHandler::onTimer2(Poco::Timer &timer)
{
    this->sendMsg("OK\n");
}
