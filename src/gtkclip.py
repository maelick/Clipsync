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

import pygtk
pygtk.require('2.0')
import gtk, gobject
from twisted.internet import gtk2reactor
gtk2reactor.install()

import sys
import clipdata
from twisted.internet.defer import Deferred
from twisted.internet import reactor
from clipsync import ClipboardManager

class GTKClip(ClipboardManager):
    """GTKClip extends the ClipboardManager to binds Clipsync's clipboard
    to the GTK clipboard."""

    def __init__(self, config_file):
        ClipboardManager.__init__(self, config_file)
        self.clip = gtk.clipboard_get()
        self.set_gtkclip(None)
        self.fetch_clip()

    def fetch_clip(self):
        """Fetch the GTK clipboard's content and sends it to other peers.
        This function calls itself repeatedly."""
        if self.clip.wait_is_text_available():
            set_text = lambda clipboard, text, data: self.set_text(text)
            self.clip.request_text(set_text)
        if self.clip.wait_is_image_available():
            set_image = lambda clipboard, image, data: self.set_image(image)
            self.clip.request_image(set_image)
        reactor.callLater(0.5, self.fetch_clip)

    def set_gtkclip(self, data):
        """Updates the GTK clipboard."""
        if data is not None:
            data = data.get_data()
            if type(data) is str:
                self.clip.set_text(data, len(data))
            elif isinstance(data, gtk.gdk.Pixbuf):
                self.clip.set_img(data)
        d = Deferred()
        d.addCallback(self.set_gtkclip)
        self.deferred = d

def main():
    if len(sys.argv) > 1:
        config_filename = sys.argv[1]
    else:
        config_filename = '~/.clipman'

    clipman = GTKClip(config_filename)

    reactor.run()

if __name__ == '__main__':
    main()
