#include "local_frontend.h"

#include <Poco/RegularExpression.h>
#include <vector>
#include <iostream>
#include <sstream>

#include "clipboard_manager.h"

using Poco::RegularExpression;
using namespace std;

LocalManager::LocalManager(Config *conf, Poco::ThreadPool *pool,
                           ClipboardManager *manager):
    conf(conf),
    ssock(conf->getLocalAddress()),
    pool(pool),
    manager(manager),
    verbose(conf->getVerboseLocal())
{
}

void LocalManager::start()
{
    this->pool->start(*this);
}

void LocalManager::run()
{
    while(true) {
        SocketAddress addr;
        StreamSocket sock = this->ssock.acceptConnection(addr);
        LocalHandler *handler =  new LocalHandler(this->conf, this->manager,
                                                  sock);
        this->manager->addLocal(handler);
        if(this->verbose) {
            cout << "Local client connected" << endl;
        }
        this->pool->start(*handler);
    }
}

LocalHandler::LocalHandler(Config *conf, ClipboardManager *manager,
                           StreamSocket &sock):
    conf(conf),
    manager(manager),
    sock(sock),
    verbose(conf->getVerboseLocal())
{
}

void LocalHandler::run()
{
    this->isRunning = true;
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

void LocalHandler::sendMsg(string msg)
{
    if(this->verbose) {
        cout << "Message: " << msg << " sent to local client on address "
             << this->sock.peerAddress().toString() << endl;
    }
    this->sock.sendBytes(msg.c_str(), msg.size());
}

void LocalHandler::close()
{
    if(this->verbose) {
        cout << "Closing connection with local client on address "
             << this->sock.peerAddress().toString();
    }
    this->isRunning = false;
    this->manager->removeLocal(this);
}

int getInt(string s);
// {
//     istringstream iss(s);
//     int n;
//     iss >> n;
//     return n;
// }

void LocalHandler::treatMsg(string msg)
{
    RegularExpression getMsg("^GET.*",
                              RegularExpression::RE_DOTALL);
    RegularExpression dataMsg("^DATA ([0-9]+) (.*)",
                              RegularExpression::RE_DOTALL);

    vector<string> v;

    if(getMsg.match(msg)) {
        getMsg.split(msg, v);
        this->sendClipboard();
    } else if(dataMsg.match(msg)) {
        dataMsg.split(msg, v);
        this->treatData(getInt(v[1]), v[2]);
    } else if(this->verbose) {
        cout << "Unknown message received from local client"
             << " on address " << this->sock.peerAddress().toString()
             << endl;
    }
}

void LocalHandler::treatData(int length, string data)
{
    ostringstream buf;
    buf << data;

    while(buf.str().size() < length) {
        char buffer[1024];
        int n = this->sock.receiveBytes(buffer, sizeof(buffer));
        if(n > 0) {
            data = buffer;
            buf << data;
        } else {
            this->isRunning = false;
            this->close();
        }
    }

    this->manager->setClipboard(this, buf.str().substr(0, length));
}

void LocalHandler::sendClipboard()
{
    this->sendClipboard(this->manager->getClipboard());
}

void LocalHandler::sendClipboard(string clipboard)
{
    ostringstream oss;
    oss << "DATA " << clipboard.size()
        << " " << clipboard << endl;
    this->sendMsg(oss.str());
}
