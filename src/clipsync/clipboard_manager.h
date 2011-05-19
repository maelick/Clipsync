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

#ifndef DEF_CLIPBOARD_MANAGER_H
#define DEF_CLIPBOARD_MANAGER_H

#include <map>
#include <Poco/Mutex.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>
#include <vector>

#include "config.h"
#include "peer_handler.h"
#include "local_frontend.h"

using Poco::Net::ServerSocket;

/*!
 * The ClipboardManager is the main class for managing the Clipsync's
 * clipboard. It listen for incoming TCP connections and connects to
 * peers when a broadcast message is received by the Broadcaster.
 * Then it authentificates the peer and treats all incoming messages.
 */
class ClipboardManager: public Poco::Runnable
{
public:
    /*!
     * Initializes the ClipboardManager with the Clipsync's Config.
     */
    ClipboardManager(Config *conf);

    /*!
     * Starts the ClipboardManager in a new thread.
     * It will also starts the LocalManager.
     */
    void start();

    /*!
     * Main method called when starting ClipboardManager in a new thread.
     */
    void run();

    /*!
     * Contacts a peer with a given address and a given name. Called by
     * Broadcaster.
     */
    void contact(SocketAddress &addr, std::string peerName);

    /*!
     * Removes a LocalHandler from the list of handlers.
     */
    void removeLocal(LocalHandler *handler);

    /*!
     * Adds a LocalHandler to the list of handlers.
     */
    void addLocal(LocalHandler *handler);

    /*!
     * Removes a PeerHandler from the list and map of peers.
     */
    void removePeer(PeerHandler *handler, std::string &peerName);

    /*!
     * Adds a PeerHandler to the list and map of peers.
     */
    bool addPeer(PeerHandler *handler, std::string &peerName);

    /*!
     * Returns the content of the clipboard.
     */
    std::string getClipboard();

    /*!
     * Synchronizes the clipboard received from a PeerHandler with
     * LocalHandlers.
     */
    void syncClipboard(std::string data);

    /*!
     * Synchronizes the clipboard received from a LocalHandler with the others
     * LocalHandlers and the PeerHandlers.
     */
    void setClipboard(LocalHandler *handler, std::string data);

private:
    Config *conf; //!< Config object used by Clipsync.
    ServerSocket ssock; //!< Server socket used to communicate with peers.
    std::vector<PeerHandler*> handlers; //!< List of PeerHandlers.
    std::vector<LocalHandler*> localHandlers; //!< List of LocalHandlers.
    std::map<std::string,PeerHandler*> peers; //!< Map of PeerHandlers.
    Poco::ThreadPool pool; //<! Pool used to start new threads.
    LocalManager localManager; //<! LocalManager used to manage local clients.
    Poco::Mutex peerMutex; //!< Mutex used for PeerHandlers list & map.
    Poco::Mutex localMutex; //!< Mutex used for LocalHandlers list.
    Poco::Mutex clipMutex; //!< Mutex used for clipboard.
    std::string clipboard; //!< Clipboard content.
    bool verbose; //!< Boolean used to set verbose mode or not
};

#endif
