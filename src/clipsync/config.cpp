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

#include "config.h"

#include <Poco/Crypto/CipherFactory.h>
#include <Poco/String.h>

using Poco::Crypto::CipherFactory;
using namespace std;

const string DEFAULT_INTERFACE = "eth0";
const bool DEFAULT_USE_IPv6 = false;
const short DEFAULT_PORT = 2525;
const short DEFAULT_BCAST_PORT = 4242;
const short DEFAULT_LOCAL_PORT = 1111;
const int DEFAULT_BCAST_INTERVAL = 5000;
const string DEFAULT_PEER_NAME = "unknown";
const string DEFAULT_GROUP = "clipboard";
const int DEFAULT_KEEPALIVE_DELAY = 1000;
const int DEFAULT_KEEPALIVE_INTERVAL = 200;

Config::Config(string &confFile):
    conf(new XMLConfiguration(confFile)),
    gen()
{
    this->gen.seed();
    this->initConfigFile(confFile);
    CipherFactory &cf = CipherFactory::defaultFactory();
    this->cipherKey =  new CipherKey("aes-256-ofb",
                                     this->passphrase, this->salt);
    this->cipher = cf.createCipher(*this->cipherKey);
}

Config::~Config()
{
    delete this->cipher;
    delete this->cipherKey;
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
    return NetworkInterface::forName(this->getString("net_frontend.interface"),
                                     this->getBool("net_frontend.use_ipv6"));
}

SocketAddress Config::getAddress()
{
    return SocketAddress(this->getInterface().address(),
                         this->getInt("net_frontend.port"));
}

SocketAddress Config::getBroadcastAddress()
{
    return SocketAddress(this->getInterface().broadcastAddress(),
                         this->getInt("net_frontend.bcast_port"));
}

int Config::getBcastInterval()
{
    return this->conf->getInt("net_frontend.bcast_interval");
}

string Config::getPeerName()
{
    return this->conf->getString("net_frontend.peer_name");
}

string Config::getGroup()
{
    return this->conf->getString("net_frontend.group");
}

int Config::getKeepaliveDelay()
{
    return this->conf->getInt("net_frontend.keepalive_delay");
}

int Config::getKeepaliveInterval()
{
    return this->conf->getInt("net_frontend.keepalive_interval");
}

bool Config::getVerboseNet()
{
    return this->conf->getBool("net_frontend.verbose");
}

bool Config::getVerboseBcast()
{
    return this->conf->getBool("net_frontend.verbose_bcast");
}

bool Config::getVerbosePeer()
{
    return this->conf->getBool("net_frontend.verbose_peer");
}

SocketAddress Config::getLocalAddress()
{
    return SocketAddress("localhost",
                         this->getInt("local_frontend.local_port"));
}

bool Config::getVerboseLocal()
{
    return this->conf->getBool("local_frontend.verbose");
}

int Config::getChallenge()
{
    return this->gen.next();
}

string Config::encrypt(string s)
{
    s = this->cipher->encryptString(s, Cipher::ENC_BASE64);
    s = Poco::translate(s, "\r\n", "");
    s = Poco::translate(s, "\n", "");
    return s;
}

string Config::decrypt(string s)
{
    s = Poco::translate(s, "\r\n", "");
    s = Poco::translate(s, "\n", "");
    s = this->cipher->decryptString(s, Cipher::ENC_BASE64);
    return s;
}

string Config::getRandomString()
{
    char s[16];
    for(int i = 0; i < 16; i++) {
        s[i] = this->gen.next() % 94 + 33;
    }

    return string(s, 16);
}

void Config::initConfigFile(string &confFile) {
    if(!this->conf->hasProperty("net_frontend.interface")) {
        this->conf->setString("net_frontend.interface", DEFAULT_INTERFACE);
    }
    if(!this->conf->hasProperty("net_frontend.use_ipv6")) {
        this->conf->setBool("net_frontend.use_ipv6", DEFAULT_USE_IPv6);
    }
    if(!this->conf->hasProperty("net_frontend.port")) {
        this->conf->setInt("net_frontend.port", DEFAULT_PORT);
    }
    if(!this->conf->hasProperty("net_frontend.bcast_port")) {
        this->conf->setInt("net_frontend.bcast_port", DEFAULT_BCAST_PORT);
    }
    if(!this->conf->hasProperty("net_frontend.bcast_interval")) {
        this->conf->setInt("net_frontend.bcast_interval",
                           DEFAULT_BCAST_INTERVAL);
    }

    if(!this->conf->hasProperty("net_frontend.peer_name")) {
        this->conf->setString("net_frontend.peer_name", DEFAULT_PEER_NAME);
    }
    if(!this->conf->hasProperty("net_frontend.group")) {
        this->conf->setString("net_frontend.group", DEFAULT_GROUP);
    }

    if(!this->conf->hasProperty("net_frontend.passphrase")) {
        this->conf->setString("net_frontend.passphrase",
                              this->getRandomString());
    }
    this->passphrase = this->conf->getString("net_frontend.passphrase");
    if(!this->conf->hasProperty("net_frontend.salt")) {
        this->conf->setString("net_frontend.salt", this->getRandomString());
    }
    this->salt = this->conf->getString("net_frontend.salt");

    if(!this->conf->hasProperty("net_frontend.keepalive_delay")) {
        this->conf->setInt("net_frontend.keepalive_delay",
                           DEFAULT_KEEPALIVE_DELAY);
    }
    if(!this->conf->hasProperty("net_frontend.keepalive_interval")) {
        this->conf->setInt("net_frontend.keepalive_interval",
                           DEFAULT_KEEPALIVE_INTERVAL);
    }

    if(!this->conf->hasProperty("net_frontend.verbose")) {
        this->conf->setBool("net_frontend.verbose", false);
    }
    if(!this->conf->hasProperty("net_frontend.verbose_bcast")) {
        this->conf->setBool("net_frontend.verbose_bcast", false);
    }
    if(!this->conf->hasProperty("net_frontend.verbose_peer")) {
        this->conf->setBool("net_frontend.verbose_peer", false);
    }

    if(!this->conf->hasProperty("local_frontend.local_port")) {
        this->conf->setInt("local_frontend.local_port", DEFAULT_LOCAL_PORT);
    }
    if(!this->conf->hasProperty("local_frontend.verbose")) {
        this->conf->setBool("local_frontend.verbose", false);
    }

    this->conf->save(confFile);
}
