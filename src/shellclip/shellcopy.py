#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Clipsync, clipboard synchronizer
# Copyright (C) 2011 Maëlick Claes (himself [at] maelick [dot] net)

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

import socket, sys

def send(port, data):
    """
    Sends some data to a localhost port.
    """
    s = socket.socket()
    s.connect(("localhost", port))
    s.send("DATA 0 {0} {1}\n".format(len(data), data))
    s.close()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        try:
            send(int(sys.argv[1]), sys.stdin.read()[:-1])
        except KeyboardInterrupt:
            sys.exit(0)
        except:
            sys.stderr.write("Unable to connect\n")
            sys.exit(1)
    else:
        sys.stderr.write("Please specify as argument the port on which to " +
                         "send the clipboard as argument\n")
