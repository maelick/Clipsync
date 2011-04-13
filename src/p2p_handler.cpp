#include "p2p_handler.h"

#include <Poco/RegularExpression.h>
#include <vector>
#include <sstream>

using Poco::RegularExpression;
using namespace Poco::Net;
using namespace std;

PeerFactory::PeerFactory(Config &conf):
    conf(conf)
{
}

TCPServerConnection* PeerFactory::createConnection(const StreamSocket &socket)
{
    return new PeerHandler(socket, this->conf);
}

PeerHandler::PeerHandler(const StreamSocket &socket, Config &conf):
    TCPServerConnection(socket),
    conf(conf),
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
    // reset timer
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
    //démarrer le timer
}

void PeerHandler::verifyAccept(int nbr)
{
    if(nbr == this->challenge + 1) {
        this->ready = true;
    } else {
        this->sendClose();
    }
}

void PeerHandler::treatJoin(string peerName, int nbr)
{
    ostringstream oss;
    oss << "ACCEPT " << nbr + 1 << endl;
    this->sendMsg(oss.str());
}

void PeerHandler::sendClose()
{
    this->sendMsg("KO 0\n");
    this->close();
}

void PeerHandler::close()
{
    this->isRunning = false;
}

PeerManager::PeerManager(Config &conf):
    TCPServer(new PeerFactory(conf),
              conf.getAddress()),
    conf(conf)
{
}
