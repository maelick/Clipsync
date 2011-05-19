#!/usr/bin/env python

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
        self.socket.send("DATA {0} {1}\n".format(len(data), data))

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
                         "send the clipbopard.")
