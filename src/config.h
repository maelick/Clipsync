#ifndef DEF_CONFIG_H
#define DEF_CONFIG_H

#include <Poco/Util/XMLConfiguration.h>
#include <Poco/AutoPtr.h>

using Poco::Util::XMLConfiguration;

class Config
{
public:
    Config(std::string &confFile);
    std::string getString(std::string property);
    int getInt(std::string property);
    bool getBool(std::string property);

private:
    void initConfigFile(std::string &confFile);
    Poco::AutoPtr<XMLConfiguration> conf;
};

#endif
