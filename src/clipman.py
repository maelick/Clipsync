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
        self.deferred = []

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

    def add_peer(self, name, protocol):
        """Adds a peer to the manager and gives a mapping from
        peer's name to the associated PeerProtocol."""
        can_add_peer = not self.has_peer(name)
        if can_add_peer:
            self.peers[name] = protocol
        return can_add_peer

    def remove_peer(self, name):
        """Removes a peer from the manager.
        The associated connection should be closed before doing this."""
        if self.has_peer(name):
            del self.peers[name]

    def has_peer(self, name):
        """Tells if there is an already opened connection with the given
        peer."""
        return self.peers.has_key(name)

    def contact_peer(self, name, host, port):
        """Contacts a peer on a specified host and port.
        A new SSL connection is started with the peer."""
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
        if self.clipboard != data:
            self.clipboard = data
            for d in self.deferred:
                d.callback(self.clipboard)
            self.deferred = []
            print "Clipboard set to: {0}".format(data)
            self.send_clipboard(clip_sender)

    def set_text(self, text):
        """Updates the clipboard with a raw string and sends it to
        every peers."""
        data = clipdata.Text(text)
        return self.set_data(data)

    def set_data(self, data):
        """Updates the clipboard with a basic Data (Text or Image),
        encapsulates it into ChecksumedData and/or CompressedData
        depending of config and sends it to every peers."""
        if self.checksum_type:
            data = clipdata.ChecksumedData(self.checksum_type, data, False)
        if self.use_compression and \
           self.use_compression <= len(data.get_data()):
            data = clipdata.CompressedData(self.compression_type, data,
                                           False)
        self.set_clipboard(data, self)

        d = Deferred()
        from twisted.internet import reactor
        reactor.callLater(1, d.callback, None)
        return d

    def get_clipboard(self):
        """Returns a deferred that will returns the clipboard
        when there will be data in it."""
        d = Deferred()
        if self.clipboard:
            d.callback(self.clipboard)
        else:
            self.deferred.append(d)
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
        config_filename = '~/.clipman'
    clipman = ClipboardManager((config_filename))
    from twisted.internet import reactor
    reactor.run()

if __name__ == '__main__':
    main()
