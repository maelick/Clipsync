#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Clipsync, clipboard synchronizer
# Copyright (C) 2011-2012 Maëlick Claes (himself [at] maelick [dot] net)

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys, os.path, yaml
import clipdata
from twisted.python import log
from twisted.python.logfile import DailyLogFile
from discoverer import PeerDiscoverer
from twisted.internet.defer import Deferred
from twisted.protocols.basic import NetstringReceiver
from twisted.internet.protocol import ClientFactory, Protocol

class ClipboardManager:
    """The ClipboardManager maintains the synchronized clipboard,
    the list of peers and the opened connections to them."""

    def __init__(self, config_filename):
        self.load_config(config_filename)
        self.clipboard = ''
        self.peers = {}
        self.deferred = None
        self.start_logging()

        self.factory = PeerFactory(self.name, self.group, self)
        self.discoverer = PeerDiscoverer(self, self.name, self.group,
                                         self.multicast_addr,
                                         self.multicast_port,
                                         self.multicast_ttl,
                                         self.multicast_interval)
        from twisted.internet import reactor
        port = reactor.listenTCP(self.port, self.factory)
        self.port = port.getHost().port
        reactor.listenMulticast(self.multicast_port, self.discoverer,
                                listenMultiple=True)

    def start_logging(self):
        """Starts logging to log file or stdout depending on config."""
        if self.use_log:
            if self.log_stdout:
                log.startLogging(sys.stdout)
            else:
                log_file = os.path.expanduser(self.log_file)
                log.startLogging(DailyLogFile.fromFullPath(log_file))

    def load_config(self, filename):
        """Loads configuration file"""
        with open(os.path.expanduser(filename)) as f:
            conf = yaml.load(f)
            self.multicast_addr = conf['multicast_addr']
            self.multicast_port = conf['multicast_port']
            self.multicast_interval = conf['multicast_interval']
            self.multicast_ttl = conf['multicast_ttl']
            self.port = conf['port']
            self.name = conf['name']
            self.group = conf['group']
            self.use_compression = conf['use_compression']
            self.compression_type = conf['compression_type']
            self.checksum_type = conf['checksum_type']
            self.use_log = conf['use_log']
            if self.use_log:
                self.log_stdout = conf['log_stdout']
                if not self.log_stdout:
                    self.log_file = conf['log_file']

    def add_peer(self, name, protocol):
        """Adds a peer to the manager and gives a mapping from
        peer's name to the associated PeerProtocol."""
        can_add_peer = not self.has_peer(name)
        if can_add_peer:
            self.peers[name] = protocol
            log.msg("Peer {0} added to peer table.".format(name))
        return can_add_peer

    def remove_peer(self, name):
        """Removes a peer from the manager.
        The associated connection should be closed before doing this."""
        if self.has_peer(name):
            log.msg("Peer {0} removed from peer table.".format(name))
            del self.peers[name]

    def has_peer(self, name):
        """Tells if there is an already opened connection with the given
        peer."""
        return self.peers.has_key(name)

    def contact_peer(self, name, host, port):
        """Contacts a peer on a specified host and port.
        A new SSL connection is started with the peer."""
        str = "Contacting peer {0} from {1} on port {2}"
        log.msg(str.format(name, host, port))
        from twisted.internet import reactor
        reactor.connectTCP(host, port, self.factory)

    def send_clipboard(self, clip_sender):
        """Sends the content of the clipboard to all peers
        except the one who send the clipboard (clip_sender)."""
        for peer in self.peers:
            if peer != clip_sender:
                self.peers[peer].send_data(self.clipboard)

    def set_clipboard(self, data, clip_sender):
        """Updates the clipboard's data received from clip_sender
        peer.
        This will contacts every peers except the sender."""
        if not self.clipboard == data:
            msg = "Updates clipboard received from {0}"
            msg.log(msg.format(clip_sender))
            self.clipboard = data
            if self.deferred:
                d, self.deferred = self.deferred, None
                d.callback(self.clipboard)
            self.send_clipboard(clip_sender)

    def set_text(self, text):
        """Updates the clipboard with a raw string and sends it to
        every peers.
        Returns a deferred that will be fired when clipboard will be sent
        to peers."""
        data = clipdata.Text(text)
        return self.set_data(data)

    def set_image(self, image):
        """Updates the clipboard with an image (GTK pixbuf) and sends it
        to every peers..
        Returns a deferred that will be fired when clipboard will be sent
        to peers."""
        data = clipdata.Image(image.get_has_alpha(),
                              image.get_bits_per_sample(),
                              image.get_width(), image.get_height(),
                              image.get_rowstride(), image.get_pixels())
        return self.set_data(data)

    def set_data(self, data):
        """Updates the clipboard with a basic Data (Text or Image),
        encapsulates it into ChecksumedData and/or CompressedData
        depending of config and sends it to every peers.
        Returns a deferred that will be fired when clipboard will be sent
        to peers."""
        use_compression = isinstance(data, clipdata.Image)
        if self.checksum_type:
            data = clipdata.ChecksumedData(self.checksum_type, data, False)
        if self.use_compression and \
           (use_compression or \
            self.use_compression <= len(data.get_data())):
            data = clipdata.CompressedData(self.compression_type, data,
                                           False)
        self.set_clipboard(data, self)

        d = Deferred()
        from twisted.internet import reactor
        reactor.callLater(0.5, d.callback, None)
        return d

    def get_clipboard(self):
        """Returns a deferred that will returns the clipboard
        when there will be data in it."""
        d = Deferred()
        if self.clipboard:
            d.callback(self.clipboard)
        else:
            self.deferred = d
        return d

class PeerProtocol(NetstringReceiver):
    """This class represents the protocol (based on netstrings) used
    to communicate with a peer."""

    def connectionMade(self):
        """Sends a JOIN when the connection to the peer has been
        established.
        Also sends a GET message if clipboard is empty."""
        self.name = None
        self.send_join()
        if not self.factory.clipman.clipboard:
            self.send_get()

    def send_join(self):
        """Sends a JOIN message."""
        msg = 'JOIN {0} {1}'.format(self.factory.name, self.factory.group)
        self.sendString(msg)

    def send_get(self):
        """Sends a GET message to request the clipboard."""
        self.sendString('GET clipboard')

    def send_data(self, data):
        """Sends a DATA message with a given data."""
        self.sendString('DATA {0}'.format(data))

    def stringReceived(self, string):
        """Treat an incoming string."""
        msg, data = string.split(' ', 1)
        if msg == 'JOIN':
            self.treat_join(data)
        elif msg == 'GET':
            self.treat_get()
        elif msg == 'DATA':
            self.treat_data(data)

    def treat_join(self, data):
        """Treats a JOIN message."""
        self.name, group = data.split(' ', 1)
        add_peer = self.factory.clipman.add_peer
        if group != self.factory.group or not add_peer(self.name, self):
            self.transport.loseConnection()

    def treat_get(self):
        """Treats a GET message."""
        if self.factory.clipman.clipboard:
            self.send_data(self.factory.clipman.clipboard)

    def treat_data(self, data):
        """Treats an incoming DATA message."""
        type, data = data.split(' ', 1)
        self.factory.clipman.set_clipboard(clipdata.get_data(type, data),
                                           self.name)

    def connectionLost(self, reason):
        """Removes the peer from the manager when this connection
        is lost."""
        self.factory.clipman.remove_peer(self.name)

class PeerFactory(ClientFactory):
    """Factory used to creates connections with other peers."""

    protocol = PeerProtocol

    def __init__(self, name, group, clipman):
        self.name = name
        self.group = group
        self.clipman = clipman

def main():
    if len(sys.argv) > 1:
        config_filename = sys.argv[1]
    else:
        config_filename = '~/.clipman/conf.yml'
    clipman = ClipboardManager((config_filename))
    from twisted.internet import reactor
    reactor.run()

if __name__ == '__main__':
    main()
