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

import sys
from twisted.internet.protocol import DatagramProtocol
from twisted.internet.defer import Deferred
from twisted.internet.task import LoopingCall

class PeerDiscoverer(DatagramProtocol):
    """A PeerDiscoverer broadcasts messages using IPv4 multicast
    so others peers may discoverer this one."""

    def __init__(self, clipman, name, group, addr, port, ttl, interval):
        self.clipman = clipman
        self.name = name
        self.group = group
        self.addr = addr
        self.port = port
        self.interval = interval
        self.ttl = ttl

    def startProtocol(self):
        """Starts the broadcasting of messages."""
        self.transport.setTTL(self.ttl)
        self.transport.joinGroup(self.addr)
        # self.transport.setOutgoingInterface('192.168.2.5')
        LoopingCall(self.send_msg).start(self.interval)
        print "Protocol started"

    def send_msg(self):
        """Sends a single multicast message."""
        msg = 'JOIN {0} {1} {2}'.format(self.name, self.group,
                                        self.clipman.port)
        self.transport.write(msg, (self.addr, self.port))

    def datagramReceived(self, data, (host, port)):
        """Treats incoming datagram received."""
        msg, data = data.split(' ', 1)
        if msg == 'JOIN':
            name, group, port = data.split(' ')
            if group == self.group and name != self.name and \
                   not self.clipman.has_peer(name):
                print "Peer {0} from {1} on port {2}".format(name, host, port)
                self.clipman.contact_peer(name, host, int(port))
