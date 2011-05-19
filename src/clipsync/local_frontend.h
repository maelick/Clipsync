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

#ifndef DEF_LOCAL_FRONTEND_H
#define DEF_LOCAL_FRONTEND_H

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>

#include "config.h"

using Poco::Net::ServerSocket;
using Poco::Net::StreamSocket;

class ClipboardManager;

/*!
 * The LocalManager is an object used to manage connections to local clients.
 */
class LocalManager: public Poco::Runnable
{
public:
    /*!
     * Initialize the LocalManager with a Config, a thread pool and a
     * ClipboardManager.
     */
    LocalManager(Config *conf, Poco::ThreadPool *pool,
                 ClipboardManager *manager);

    /*!
     * Starts the LocalManager in a new thread.
     */
    void start();

    /*!
     * Main method of the manager called when starting in a new thread.
     */
    void run();

private:
    Config *conf; //!< Config object used by Clipsync.
    ServerSocket ssock; //!< Socket used for incoming connections.
    Poco::ThreadPool *pool; //!< Pool used for starting new threads.
    ClipboardManager *manager; //!< ClipboardManager of Clipsync.
    bool verbose; //!< Boolean used to set verbose mode or not.
};

/*!
 * A LocalHandler is used to manage the connection between the peer
 * and a local client.
 */
class LocalHandler: public Poco::Runnable
{
public:
    /*!
     * Initializes the handler with a Config, a ClipboardManager and the
     * socket to use for the TCP connection.
     */
    LocalHandler(Config *conf, ClipboardManager *manager, StreamSocket &sock);

    /*!
     * Main method of the handler callend when started in a new thread.
     * Called by the LocalManager when there is a new incoming connection.
     */
    void run();

    /*!
     * Sends the content of the clipboard to the local client when received
     * from another peer. Called in ClipboardManager.
     */
    void sendClipboard(std::string clipboard);

private:
    /*!
     * Sends a message to the local client.
     */
    void sendMsg(std::string msg);

    /*!
     * Closes the TCP connection with the local client.
     */
    void close();

    /*!
     * Treats an incoming message.
     */
    void treatMsg(std::string msg);

    /*!
     * Treats the data (clipboard) received from the local client.
     */
    void treatData(int type, int length, std::string data);

    /*!
     * Sends the content of the clipboard to the local client when requested
     * by the local client.
     */
    void sendClipboard();

    Config *conf; //!< Config object used by Clipsync.
    ClipboardManager *manager; //!< ClipboardManager of Clipsync.
    StreamSocket sock; //!< Socket used to communicate with the client.
    bool isRunning; //!< Boolean saying if the handler is running or not.
    bool verbose; //!< Boolean used to set verbose mode or not.
};

#endif
