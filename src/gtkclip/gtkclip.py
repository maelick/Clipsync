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
import pygtk
pygtk.require('2.0')
import gtk, gobject

class GTKClipSync:
    """
    A GTKClipSync is an object used to synchronize the OS windowing system
    with Clipsync.
    """
    def __init__(self, port):
        """
        Initialize the GTKClipSync using a localhost port to communicate
        with Clipsync. Connects the socket and add a timeout every 500ms
        to check if local clipboard has changed.
        """
        self.clip = gtk.clipboard_get(gtk.gdk.SELECTION_CLIPBOARD)
        self.last_text = ""
        self.socket = socket.socket()
        self.socket.connect(("localhost", port))
        self.clip_init()
        gobject.timeout_add(500, self.fetch_clipboard)
        gobject.io_add_watch(self.socket, gobject.IO_IN, self.get_data)

    def clip_text_recv(self, clipboard, text, data):
        """
        Sends the OS windowing system clipboard content to Clipsync if it
        changed.
        This method is called by the GTK event loop.
        """
        if self.last_text != text:
            self.last_text = text
            self.send(text)

    def get_data(self, fd=None, cb_condition=None):
        """
        Gets some data from Clipsync. This method is called by GTK
        at the end of the timeout defined at the creation of the GTKClipSync
        object.
        """
        data = self.socket.recv(1024).split(" ")
        datatype = int(data[1])
        if datatype != 0:
            print "Unable to use datatype {0}".format(datatype)
            return True
        length = int(data[2])
        data = " ".join(data[3:])
        while(len(data) < length):
            data += self.socket.recv(1024)
        text = data[:length]
        if self.last_text != text:
            self.last_text = text
            self.clip.set_text(text, len(text))
        return True

    def clip_init(self):
        """
        Initialize the clipboard and asking to clipsync to send
        the content of the synchronized clipboard.
        """
        self.socket.send("GET\n")
        self.get_data()

    def send(self, data):
        """
        Sends some data to Clipsync.
        """
        self.socket.send("DATA 0 {0} {1}\n".format(len(data), data))

    def fetch_clipboard(self):
        """
        Fetch the content of the OS windowing system clipboard.
        self.clip_text_recv will be called by the GTK event system.
        """
        self.clip.request_text(self.clip_text_recv)
        return True

if __name__ == '__main__':
    if len(sys.argv) > 1:
        clip = GTKClipSync(int(sys.argv[1]))
        gtk.main()
    else:
        sys.stderr.write("Please specify as argument the port on which to " +
                         "send the clipbopard.\n")
