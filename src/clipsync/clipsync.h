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

#ifndef DEF_CLIPSYNC_H
#define DEF_CLIPSYNC_H

#include <Poco/ThreadPool.h>
#include "config.h"
#include "broadcaster.h"
#include "clipboard_manager.h"

/*!
 * Clipsync is the main class used to encapsulate the others and to represent
 * the Clipsync's P2P client.
 */
class Clipsync
{
public:
    /*!
     * Initializes the P2Client with a config file name.
     */
    Clipsync(std::string &confFile);
    ~Clipsync();

    /*!
     * Returns the Config of this client.
     */
    Config* getConfig();

    /*!
     * Starts the Clipsync and waits for it to finish.
     */
    void start();

private:
    Config *conf; //!< Config object used by Clipsync.
    ClipboardManager *manager; //!< ClipboardManager of Clipsync.
    Broadcaster *b; //!< Broadcaster of Clipsync.
};

#endif
