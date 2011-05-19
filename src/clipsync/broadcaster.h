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

#ifndef DEF_BROADCASTER_H
#define DEF_BROADCASTER_H

#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>
#include <Poco/Timer.h>
#include "config.h"
#include "clipboard_manager.h"

using Poco::Net::SocketAddress;
using Poco::Net::DatagramSocket;

/*!
 * A Broadcaster is used to broadcast a network to find peers willing to
 * synchronize their clipboard. It sends message on the broadcast address of
 * an interface to find peers. It also listen for
 * broadcast messages incoming and starts a TCP session when a message
 * is received.
 */
class Broadcaster: public Poco::Runnable
{
public:
    /*!
     * Initializes the Broadcaster with a given ClipboardManager and a
     * Config.
     */
    Broadcaster(Config *conf, ClipboardManager *manager);

    /*!
     * Starts the Broadcaster in a new thread.
     */
    void start();

    /*!
     * This method will be called by the Timer for broadcasting a message.
     */
    void onTimer(Poco::Timer &timer);

    /*!
     * Main method of the broadcaster called when starting in a new thread.
     */
    void run();

private:
    /*!
     * Treats an incoming message.
     */
    void treatMsg(SocketAddress &src, std::string msg);

    Config *conf; //!< Config object used by Clipsync.
    ClipboardManager *manager; //!< ClipboardManager of Clipsync.
    SocketAddress bcastAddr, srcAddr; //!< Broadcast and listening addresses.
    DatagramSocket s1; //!< Socket used for listening.
    DatagramSocket s2; //!< Socket used for broadcasting.
    Poco::ThreadPool pool; //!< Pool used when creating a new thread.
    Poco::Timer t; //!< Timer used for broadcasting messaged.
    bool verbose; //!< Boolean used to set verbose mode or not.
};

#endif
