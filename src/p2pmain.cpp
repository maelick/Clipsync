#include <iostream>
#include "p2p.h"

using namespace std;

void start(string conf)
{
    P2PClient p2p(conf);
    Poco::ThreadPool pool;

    p2p.start(pool);
    pool.joinAll();
}

int main(int argc, char **argv) {
    if(argc <  2) {
        cerr << "Please enter a configuration file name." << endl;;
        return 1;
    } else {
        start(argv[1]);

        return 0;
    }
}
