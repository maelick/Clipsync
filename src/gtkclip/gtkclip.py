#!/usr/bin/env python

import socket, sys
import pygtk
pygtk.require('2.0')
import gtk, gobject

class GTKClipSync:
    def __init__(self, port):
        self.clip = gtk.clipboard_get(gtk.gdk.SELECTION_CLIPBOARD)
        self.last_text = ""
        self.socket = socket.socket()
        self.socket.connect(("localhost", port))
        self.clip_init()
        gobject.timeout_add(1500, self.fetch_clipboard)
        gobject.io_add_watch(self.socket, gobject.IO_IN, self.get_data)

    def clip_text_recv(self, clipboard, text, data):
        if self.last_text != text:
            self.last_text = text
            self.send(text)

    def get_data(self, fd=None, cb_condition=None):
        data = self.socket.recv(1024).split(" ")
        length = int(data[1])
        data = " ".join(data[2:])
        while(len(data) < length):
            data += self.socket.recv(1024)
        text = data[:length]
        if self.last_text != text:
            self.last_text = text
            self.clip.set_text(text, len(text))
        return True

    def clip_init(self):
        self.socket.send("GET\n")
        self.get_data()

    def send(self, data):
        self.socket.send("DATA {0} {1}\n".format(len(data), data))

    def fetch_clipboard(self):
        self.clip.request_text(self.clip_text_recv)
        return True

if __name__ == '__main__':
    if len(sys.argv) > 1:
        clip = GTKClipSync(int(sys.argv[1]))
        gtk.main()
    else:
        sys.stderr.write("Please specify as argument the port on which to " +
                         "send the clipbopard.")
