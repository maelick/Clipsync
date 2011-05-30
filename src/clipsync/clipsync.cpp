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

#include "clipsync.h"

#include <iostream>
#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>

using namespace std;

Clipsync::Clipsync(string &confFile):
    conf(new Config(confFile))
{
    this->manager = new ClipboardManager(this->conf);
    this->b = new Broadcaster(this->conf, this->manager);
}

Clipsync::~Clipsync()
{
    delete this->conf;
    delete this->manager;
    delete this->b;
}

Config* Clipsync::getConfig()
{
    return this->conf;
}

void Clipsync::start()
{
    this->manager->start();
    this->b->start();
}

int start(string conf)
{
    Clipsync *p2p;

    try {
        p2p = new Clipsync(conf);
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
