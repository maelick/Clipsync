#include <iostream>
#include <Poco/RegularExpression.h>
#include "broadcaster.h"

using namespace std;
using Poco::TimerCallback;

Broadcaster::Broadcaster(Config *conf, ClipboardManager *manager):
    conf(conf),
    manager(manager),
    bcastAddr(conf->getBroadcastAddress()),
    srcAddr(conf->getAddress()),
    s1(this->bcastAddr, true), s2(this->srcAddr, true),
    pool(),
    t(1, conf->getBcastInterval()),
    verbose(conf->getVerboseBcast())
{
    this->s2.setBroadcast(true);
}

void Broadcaster::start()
{
    this->pool.start(*this);
    this->t.start(TimerCallback<Broadcaster>(*this, &Broadcaster::onTimer),
                  pool);
    this->pool.joinAll();
}

void Broadcaster::run()
{
    if(this->verbose) {
        cout << "Listening for broadcast messages on address "
             << this->bcastAddr.toString() << endl;
    }

    while(true) {
        SocketAddress src;
        char buf[1024];
        this->s1.receiveFrom(buf, sizeof(buf), src);

        if(src.toString() != this->srcAddr.toString()) {
            if(this->verbose) {
                cout << src.toString() << ": " << buf << endl;
            }
            this->treatMsg(src, string(buf));
        }
    }
}

void Broadcaster::treatMsg(SocketAddress &src, string msg)
{
    Poco::RegularExpression re("^JOIN ([a-z0-9]+) ([a-z0-9]+).*",
                               Poco::RegularExpression::RE_DOTALL);
    vector<string> v;
    if(re.match(msg)) {
        re.split(msg, v);
        if(this->conf->getGroup() == v[2]) {
            this->manager->contact(src, v[1]);
        }
    }
}

void Broadcaster::onTimer(Poco::Timer &timer)
{
    if(this->verbose) {
        cout << "Sending broadcast msg to address "
             << this->bcastAddr.toString() << " from address "
             << this->srcAddr.toString() << endl;
    }

    std::string msg =
        "JOIN " + this->conf->getPeerName() +
        " " + this->conf->getGroup();
    this->s2.sendTo(msg.data(), msg.size(), bcastAddr);
}
