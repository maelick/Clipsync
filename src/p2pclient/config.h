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
    std::string getString(std::string property);
    int getInt(std::string property);
    bool getBool(std::string property);
    NetworkInterface getInterface();
    SocketAddress getAddress();
    SocketAddress getBroadcastAddress();
    int getChallenge();

private:
    void initConfigFile(std::string &confFile);
    Poco::AutoPtr<XMLConfiguration> conf;
    Random gen;
};

#endif
