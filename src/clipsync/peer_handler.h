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

#ifndef DEF_PEER_HANDLER_H
#define DEF_PEER_HANDLER_H

#include <Poco/Net/DialogSocket.h>
#include <Poco/Runnable.h>
#include <Poco/ThreadPool.h>
#include <Poco/Timer.h>
#include <sstream>

#include "config.h"

using Poco::Net::DialogSocket;

class ClipboardManager;

/*!
 * A PeerHandler is an object used to handle a TCP connection with
 * a peer.
 */
class PeerHandler: public Poco::Runnable
{
public:
    /*!
     * Initializes a PeerHandler with a Config, a thread pool, a
     * ClipboardManager, a socket and a boolean saying if it is this process
     * who initiates the connection.
     */
    PeerHandler(Config *conf, Poco::ThreadPool *pool,
                ClipboardManager *manager, DialogSocket *sock, bool initiator);
    ~PeerHandler();

    /*!
     * Main methods of the PeerHandler called when starting a new thread.
     */
    void run();

    /*!
     * Sends a KO message with a given reason to the peer and closes the
     * connection.
     */
    void sendClose(int reason);

    /*!
     * Compares this PeerHandler to the other one.
     * Returns true if the other handler needs to be close, i.e. if this
     * handler's initiator peer name is ordered before the other handler's
     * initiator peer name (using lexicographic order).
     */
    bool compare(PeerHandler *other);

    /*!
     * Sends the content of the clipboard splits in DATA message where
     * the content as a maximum size of 4096 bytes.
     */
    void sendData(std::string data);

private:
    /*!
     * Sends a message to the peer.
     */
    void sendMsg(std::string msg);

    /*!
     * Sends a JOIN message to the peer.
     */
    void sendJoin();

    /*!
     * Sends a KO message with a given error number.
     */
    void sendKo(int error);

    /*!
     * Closes the connection with the peer.
     */
    void close();

    /*!
     * Returns the name of the peer who initiate the connection.
     */
    std::string getInitiatorName();

    /*!
     * Treats a message.
     */
    void treatMsg(std::string msg);

    /*!
     * Treats a DATA message and gets more bytes from the socket if length
     * is lesser than the length of the already sent datas.
     */
    void treatData(int type, int hasMore, int length, std::string data);

    /*!
     * Treat a OK message, i.e. reset the timer.
     */
    void treatOk();

    /*!
     * Treats a KO message.
     */
    void treatKo(int error);

    /*!
     * Adds the peer to the ClipboardManager if it has been authenticates.
     */
    void addPeer();

    /*!
     * Verifies if the accept message received is valid.
     */
    void verifyAccept(int nbr);

    /*!
     * Treats a JOIN message and sends an ACCEPT message as reply.
     */
    void treatJoin(std::string peerName, int nbr);

    /*!
     * Closes the connection if the peer did not authenticate on time.
     * Used by timer1.
     */
    void onTimer1(Poco::Timer &timer);

    /*!
     * Sends a OK message as keep alive. Used by timer2.
     */
    void onTimer2(Poco::Timer &timer);

    Config *conf; //!< Config object used by Clipsync.
    Poco::ThreadPool *pool; //!< Pool used for creating new threads.
    ClipboardManager *manager; //!< Clipsync's manager.
    DialogSocket *sock; //!< Socket used for the TCP Connection.
    std::string peerName; //!< Name of the peer connected to.
    int challenge; //!< Challenge to use for authentication.
    Poco::Timer t1, t2; //!< Timers used.
    std::ostringstream buf; //!< Buffer used to store data.
    bool isRunning; //!< Boolean saying if the handler is running or not.
    bool initiator; //!< Boolean saying if this peer is the initiator one.
    bool acceptSent, acceptVerified; //!< Boolean used for authentication.
    bool verbose; //!< Boolean used to set verbose mode or not.
};

#endif
