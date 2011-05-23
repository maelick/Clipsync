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

#ifndef DEF_CONFIG_H
#define DEF_CONFIG_H

#include <Poco/Crypto/Cipher.h>
#include <Poco/Crypto/CipherKey.h>
#include <Poco/Util/XMLConfiguration.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/AutoPtr.h>
#include <Poco/Random.h>

using Poco::Crypto::Cipher;
using Poco::Crypto::CipherKey;
using Poco::Util::XMLConfiguration;
using Poco::Net::NetworkInterface;
using Poco::Net::SocketAddress;
using Poco::Random;

/*!
 * Config is a class used to encapsulate the configuration of Clipsync.
 * It reads a XML file and give a set of methods to get different parameters
 * needed by the software such as network configuration informations,
 * timers delays/intervals, verbose mode,...
 */
class Config
{
public:
    /*!
     * Initializes the Config with a XML file name.
     */
    Config(std::string &confFile);

    /*!
     * Destructs the Config.
     */
    ~Config();

    /*!
     * Returns the address to use to listen for incoming TCP connections.
     */
    SocketAddress getAddress();

    /*!
     * Returns the broadcast address to use.
     */
    SocketAddress getBroadcastAddress();

    /*!
     * Returns the interval to use for sending broadcast messages.
     */
    int getBcastInterval();

    /*!
     * Returns the name of this peer.
     */
    std::string getPeerName();

    /*!
     * Returns the group to which this peer belongs to.
     */
    std::string getGroup();

    /*!
     * Returns the delay to use for sending the first keep alive.
     */
    int getKeepaliveDelay();

    /*!
     * Returns the interval to use for sending keep alives.
     */
    int getKeepaliveInterval();

    /*!
     * Returns true if the ClipboardManager needs to be set to verbose mode.
     */
    bool getVerboseNet();

    /*!
     * Returns true if the Broadcaster needs to be set to verbose mode.
     */
    bool getVerboseBcast();

    /*!
     * Returns true if the PeerHandler needs to be set to verbose mode.
     */
    bool getVerbosePeer();

    /*!
     * Returns true if the LocalHandler needs to be set to verbose mode.
     */
    bool getVerboseLocal();

    /*!
     * Returns the address to listen on.
     */
    SocketAddress getLocalAddress();

    /*!
     * Returns a pseudo-random integer to be used as a challenge for peer
     * authentification.
     */
    int getChallenge();

    /*!
     * Encrypts datas using the Config's key and salt.
     */
    std::string encrypt(std::string s);

    /*!
     * Decrypts datas using the Config's key and salt.
     */
    std::string decrypt(std::string s);

private:
    /*!
     * Returns the value of a integer property.
     */
    int getInt(std::string property);

    /*!
     * Returns the value of a boolean property.
     */
    bool getBool(std::string property);

    /*!
     * Returns the value of a string property.
     */
    std::string getString(std::string property);

    /*!
     * Returns the interface to use.
     */
    NetworkInterface getInterface();

    /*!
     * Returns a random string which can be used as a random salt
     * or passphrase.
     */
    std::string getRandomString();

    /*
     * Initializes the config file if not set correctly.
     */
    void initConfigFile(std::string &confFile);

    Poco::AutoPtr<XMLConfiguration> conf; //!< Pointer referencing the file.
    Random gen; //!< Random number generator.
    std::string passphrase; //!< Passphrase used to generate encryption key.
    std::string salt; //!< Salt used to generate encryption key.
    CipherKey *cipherKey; //!< Key used for encrypting/decrypting datas.
    Cipher *cipher; //!< Cipher used to crypt datas.
};

#endif
