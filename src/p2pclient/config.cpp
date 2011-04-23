#include "config.h"

using namespace std;

const string DEFAULT_INTERFACE = "eth0";
const bool DEFAULT_USE_IPv6 = false;
const short DEFAULT_PORT = 2525;
const short DEFAULT_BCAST_PORT = 4242;
const short DEFAULT_LOCAL_PORT = 1111;
const int DEFAULT_BCAST_INTERVAL = 5000;
const string DEFAULT_PEER_NAME = "unknown";
const string DEFAULT_GROUP = "clipboard";
const string DEFAULT_KEY = "passphrase";
const string DEFAULT_SALT = "mysalt";
const int DEFAULT_KEEPALIVE_DELAY = 1000;
const int DEFAULT_KEEPALIVE_INTERVAL = 200;

Config::Config(string &confFile):
    conf(new XMLConfiguration(confFile)),
    gen()
{
    this->initConfigFile(confFile);
    this->gen.seed();
}

string Config::getString(string property)
{
    return this->conf->getString(property);
}

int Config::getInt(string property)
{
    return this->conf->getInt(property);
}

bool Config::getBool(string property)
{
    return this->conf->getBool(property);
}

NetworkInterface Config::getInterface()
{
    return NetworkInterface::forName(this->getString("p2p_client.interface"),
                                     this->getBool("p2p_client.use_ipv6"));
}

SocketAddress Config::getAddress()
{
    return SocketAddress(this->getInterface().address(),
                         this->getInt("p2p_client.port"));
}

SocketAddress Config::getBroadcastAddress()
{
    return SocketAddress(this->getInterface().broadcastAddress(),
                         this->getInt("p2p_client.bcast_port"));
}

SocketAddress Config::getLocalAddress()
{
    return SocketAddress("localhost", this->getInt("p2p_client.local_port"));
}

void Config::initConfigFile(string &confFile) {
    if(!this->conf->hasProperty("p2p_client.interface")) {
        this->conf->setString("p2p_client.interface", DEFAULT_INTERFACE);
    }

    if(!this->conf->hasProperty("p2p_client.use_ipv6")) {
        this->conf->setBool("p2p_client.use_ipv6", DEFAULT_USE_IPv6);
    }

    if(!this->conf->hasProperty("p2p_client.port")) {
        this->conf->setInt("p2p_client.port", DEFAULT_PORT);
    }

    if(!this->conf->hasProperty("p2p_client.bcast_port")) {
        this->conf->setInt("p2p_client.bcast_port", DEFAULT_BCAST_PORT);
    }

    if(!this->conf->hasProperty("p2p_client.local_port")) {
        this->conf->setInt("p2p_client.local_port", DEFAULT_LOCAL_PORT);
    }

    if(!this->conf->hasProperty("p2p_client.bcast_interval")) {
        this->conf->setInt("p2p_client.bcast_interval",
                           DEFAULT_BCAST_INTERVAL);
    }

    if(!this->conf->hasProperty("p2p_client.peer_name")) {
        this->conf->setString("p2p_client.peer_name", DEFAULT_PEER_NAME);
    }

    if(!this->conf->hasProperty("p2p_client.group")) {
        this->conf->setString("p2p_client.group", DEFAULT_GROUP);
    }

    if(!this->conf->hasProperty("p2p_client.key")) {
        this->conf->setString("p2p_client.key", DEFAULT_KEY);
    }

    if(!this->conf->hasProperty("p2p_client.salt")) {
        this->conf->setString("p2p_client.salt", DEFAULT_SALT);
    }

    if(!this->conf->hasProperty("p2p_client.keepalive_delay")) {
        this->conf->setInt("p2p_client.keepalive_delay",
                           DEFAULT_KEEPALIVE_DELAY);
    }

    if(!this->conf->hasProperty("p2p_client.keepalive_interval")) {
        this->conf->setInt("p2p_client.keepalive_interval",
                           DEFAULT_KEEPALIVE_INTERVAL);
    }

    if(!this->conf->hasProperty("p2p_client.verbose")) {
        this->conf->setBool("p2p_client.verbose", false);
    }

    this->conf->save(confFile);
}

int Config::getChallenge()
{
    return this->gen.next();
}
