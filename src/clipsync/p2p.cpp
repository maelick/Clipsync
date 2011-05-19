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

#include "p2p.h"

using namespace std;

P2PClient::P2PClient(string &confFile):
    conf(new Config(confFile))
{
    this->manager = new ClipboardManager(this->conf);
    this->b = new Broadcaster(this->conf, this->manager);
}

P2PClient::~P2PClient()
{
    delete this->conf;
    delete this->manager;
    delete this->b;
}

Config* P2PClient::getConfig()
{
    return this->conf;
}

void P2PClient::start()
{
    this->manager->start();
    this->b->start();
}
