#include <iostream>
#include <Poco/Exception.h>
#include "p2p.h"

using namespace std;

int start(string conf)
{
    P2PClient *p2p;

    try {
        p2p = new P2PClient(conf);
    } catch(Poco::FileNotFoundException) {
        cerr << "Configuration file " << conf << " doesn't exist." << endl;
        return 1;
    }
    Poco::ThreadPool pool;

    p2p->start(pool);
    pool.joinAll();

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
