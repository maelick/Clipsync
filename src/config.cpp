#include "config.h"

using namespace std;

const string DEFAULT_ADDR = "localhost:2525";
const string DEFAULT_BCAST_ADDR = "localhost:4242";
const int DEFAULT_BCAST_DELAY = 5000;
const string DEFAULT_PEER_NAME = "unknown";
const string DEFAULT_GROUP = "clipboard";
const string DEFAULT_KEY = "passphrase";
const string DEFAULT_SALT = "mysalt";

Config::Config(string &confFile):
    conf(new XMLConfiguration(confFile))
{
    this->initConfigFile(confFile);
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

void Config::initConfigFile(string &confFile) {
    if(!this->conf->hasProperty("p2p_client.addr")) {
        this->conf->setString("p2p_client.addr", DEFAULT_ADDR);
    }

    if(!this->conf->hasProperty("p2p_client.bcast_addr")) {
        this->conf->setString("p2p_client.bcast_addr", DEFAULT_BCAST_ADDR);
    }

    if(!this->conf->hasProperty("p2p_client.bcast_delay")) {
        this->conf->setInt("p2p_client.bcast_delay", DEFAULT_BCAST_DELAY);
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

    if(!this->conf->hasProperty("p2p_client.verbose")) {
        this->conf->setBool("p2p_client.verbose", false);
    }

    this->conf->save(confFile);
}
