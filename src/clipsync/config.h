#ifndef DEF_CONFIG_H
#define DEF_CONFIG_H

#include <Poco/Util/XMLConfiguration.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/AutoPtr.h>
#include <Poco/Random.h>

using Poco::Util::XMLConfiguration;
using Poco::Net::NetworkInterface;
using Poco::Net::SocketAddress;
using Poco::Random;

class Config
{
public:
    Config(std::string &confFile);
    SocketAddress getAddress();
    SocketAddress getBroadcastAddress();
    int getBcastInterval();
    std::string getPeerName();
    std::string getGroup();
    int getKeepaliveDelay();
    int getKeepaliveInterval();
    bool getVerboseNet();
    bool getVerboseBcast();
    bool getVerbosePeer();
    SocketAddress getLocalAddress();
    bool getVerboseLocal();
    int getChallenge();

private:
    int getInt(std::string property);
    bool getBool(std::string property);
    std::string getString(std::string property);
    NetworkInterface getInterface();
    void initConfigFile(std::string &confFile);

    Poco::AutoPtr<XMLConfiguration> conf;
    Random gen;
};

#endif
