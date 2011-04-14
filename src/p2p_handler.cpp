#include "p2p_handler.h"

#include <Poco/RegularExpression.h>
#include <vector>
#include <sstream>

using Poco::TimerCallback;
using Poco::RegularExpression;
using namespace Poco::Net;
using namespace std;

PeerFactory::PeerFactory(Config &conf, Poco::ThreadPool &pool):
    conf(conf),
    pool(pool)
{
}

TCPServerConnection* PeerFactory::createConnection(const StreamSocket &socket)
{
    return new PeerHandler(socket, this->conf, this->pool);
}

PeerHandler::PeerHandler(const StreamSocket &socket, Config &conf,
                         Poco::ThreadPool &pool):
    TCPServerConnection(socket),
    conf(conf),
    pool(pool),
    t1(conf.getInt("p2p_client.keepalive_delay")),
    t2(conf.getInt("p2p_client.keepalive_interval")),
    challenge(conf.getChallenge()),
    isRunning(false),
    ready(false)
{
}

void PeerHandler::run()
{
    this->isRunning = true;
    StreamSocket &socket = this->socket();
    char buffer[1024];
    int n = socket.receiveBytes(buffer, sizeof(buffer));
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
    RegularExpression acceptMsg("^ACCEPT ([0-9]+).*",
                                RegularExpression::RE_DOTALL);
    RegularExpression okMsg("^OK.*",
                            RegularExpression::RE_DOTALL);
    RegularExpression koMsg("^KO (-?[0-9]+).*",
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

void PeerHandler::verifyAccept(int nbr)
{
    if(nbr == this->challenge + 1) {
        this->ready = true;
        this->t1.restart();
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
    ostringstream oss;
    oss << "ACCEPT " << nbr + 1 << endl;
    this->sendMsg(oss.str());
    this->t2.start(TimerCallback<PeerHandler>(*this, &PeerHandler::onTimer2),
                   this->pool);
}

void PeerHandler::sendKo(int error)
{
    ostringstream oss;
    oss << "KO " << error << endl;
    this->sendMsg(oss.str());
}

void PeerHandler::sendClose()
{
    this->sendKo(0);
    this->close();
}

void PeerHandler::close()
{
    this->isRunning = false;
    this->t1.stop();
    this->t2.stop();
}

PeerManager::PeerManager(Config &conf, Poco::ThreadPool &pool):
    TCPServer(new PeerFactory(conf, pool),
              conf.getAddress()),
    conf(conf),
    pool(pool)
{
}
