#include <iostream>
#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>
#include "p2p.h"

using namespace std;

int start(string conf)
{
    P2PClient *p2p;

    try {
        p2p = new P2PClient(conf);
    } catch(Poco::FileNotFoundException e) {
        cerr << "Configuration file " << conf << " doesn't exist." << endl;
        return 1;
    } catch(Poco::Net::InterfaceNotFoundException e) {
        cerr << "Unable to find interface " << e.message() << "." << endl;
        return 1;
    }

    p2p->start();

    delete p2p;

    return 0;
}

int main(int argc, char **argv) {
    if(argc <  2) {
        cerr << "Please enter a configuration file name." << endl;;
        return 1;
    } else {
        return start(argv[1]);
    }
}
