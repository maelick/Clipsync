#include <iostream>
#include "broadcaster.h"

using namespace std;
using Poco::TimerCallback;

Broadcaster::Broadcaster(Config &conf):
    conf(conf),
    bcastAddr(conf.getBroadcastAddress()),
    srcAddr(conf.getAddress()),
    s1(this->bcastAddr, true), s2(this->srcAddr, true),
    t(1, conf.getInt("p2p_client.bcast_delay")),
    verbose(conf.getBool("p2p_client.verbose"))
{
    this->s2.setBroadcast(true);
}

void Broadcaster::start(Poco::ThreadPool &pool)
{
    pool.start(*this);
    this->t.start(TimerCallback<Broadcaster>(*this, &Broadcaster::onTimer),
                  pool);
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
        int n = this->s1.receiveFrom(buf, sizeof(buf)-1, src);
        buf[n] = '\0';

        if(src.toString() != this->srcAddr.toString()) {
            if(this->verbose) {
                cout << src.toString() << ": " << buf << endl;
            }
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
        "JOIN " + this->conf.getString("p2p_client.peer_name") +
        " " + this->conf.getString("p2p_client.group");
    this->s2.sendTo(msg.data(), msg.size(), bcastAddr);
}
