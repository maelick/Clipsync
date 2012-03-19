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

import zlib, bz2

class Data:
    """This class is abstract and subclasses
    represents different data types that can be used """

    def __eq__(self, other):
        """Tells if the other data is equal to this one.
        Must be overriden."""
        pass

    def __str__(self):
        """Returns the valid Clipsync DATA message's data (with type).
        Must be overriden."""
        pass

    def is_text(self):
        """Returns True if this data contains only raw text."""
        pass

    def get_data(self):
        """Returns the raw data contained in the object.
        This method returns one of the following objects:
        * string for text
        * GTK pixbuf for images
        Must be overriden."""
        pass

class Text:
    """Common raw text."""

    def __init__(self, data):
        self.text = data
        self.str = None

    def __eq__(self, other):
        return isinstance(other, Text) and other.text == self.text

    def __str__(self):
        if not self.str:
            self.str = 'text {0}'.format(str(self.text))
        return self.str

    def is_text(self):
        return True

    def get_data(self):
        return self.text

class Image:
    """This class represent an image.
    Basically it is a GTK+ pixbuf but it can work with any other image
    as long as there is values for the different parameters:
    * has_alpha
    * bits_per_sample
    * width & height
    * rowstride
    * pixels raw data
    See GTK+ for more informations."""

    def __init__(self, has_alpha, bits_per_sample, width, height, rowstride,
                 pixels):
        self.has_alpha = has_alpha
        self.bits_per_sample = bits_per_sample
        self.width = width
        self.height = height
        self.rowstride = rowstride
        self.pixels = pixels
        self.pixbuf = None
        self.str = None

    def __eq__(self, other):
        return isinstance(other, Image) and \
               self.has_alpha == other.has_alpha and \
               self.bits_per_sample == other.bits_per_sample and \
               self.width == other.width and \
               self.height == other.height and \
               self.rowstride == other.rowstride and \
               self.pixels == other.pixels

    def __str__(self):
        if not self.str:
            s = 'image {0} {1} {2} {3} {4} {5}'
            self.str = s.format(int(self.has_alpha), self.bits_per_sample,
                                self.width, self.height, self.rowstride,
                                self.pixels)
        return self.str

    def is_text(self):
        return False

    def get_data(self):
        """Returns the GTK+ pixbuf object of this image.
        PyGTK & GTK+ (2.0) are required for this to work."""
        import pygtk
        pygtk.require('2.0')
        import gtk, gobject
        if not self.pixbuf:
            get_pixbuf = gtk.gdk.pixbuf_new_from_data
            self.pixbuf = get_pixbuf(self.pixels, gtk.gdk.COLORSPACE_RGB,
                                     self.has_alpha, self.bits_per_sample,
                                     self.width, self.height, self.rowstride)
        return self.pixbuf

class CompressedData:
    """A CompressedData contains another Data compressed with one of the
    following algorithms:
    * gz
    * bz2
    """

    def __init__(self, type, data, compressed=True):
        """Initializes the CompressedData using a given compression
        type/algorithm.
        If compressed is True, the data is assumed to be already in a
        compressed form.
        Otherwise it is assumed to be uncompressed (and so to be a
        Data object).
        Raises ValueError if type is unknown or data is invalid."""
        self.type = type
        if compressed:
            self.str = 'compressed {0} {1}'.format(type, data)
            data = self.decompress(data)
            data_type, data = data.split(' ', 1)
            self.data = get_data(data_type, data)
        else:
            self.str = 'compressed {0} {1}'.format(type, self.compress(data))
            self.data = data

    def decompress(self, data):
        """Decompresses a given data.
        Raises ValueError if type is unknown or data is invalid."""
        if self.type == 'gz':
            try:
                return zlib.decompress(data)
            except zlib.error:
                raise ValueError
        elif self.type == 'bz2':
            try:
                return bz2.decompress(data)
            except IOError:
                raise ValueError
        else:
            raise ValueError

    def compress(self, data):
        """Compresses a given data.
        Raises ValueError if type is unknown."""
        if self.type == 'gz':
            return zlib.compress(str(data))
        elif self.type == 'bz2':
            return bz2.compress(str(data))
        else:
            raise ValueError

    def __eq__(self, other):
        return isinstance(other, CompressedData) and \
               self.type == other.type and self.data == other.data

    def __str__(self):
        return self.str

    def is_text(self):
        return self.data.is_text()

    def get_data(self):
        return self.data.get_data()

class ChecksumedData:
    """A ChecksumedData contains another Data and its checksum
    computed with one the following algorithms:
    * alder32
    * crc32
    """

    def __init__(self, type, data, computed=True):
        """Initializes the ChecksumedData using a given checksum
        type/algorithm.
        If computed is True, the data's checksum is assumed to be already
        computed.
        Otherwise it is assumed not to be (and so data is a Data object).
        Raises ValueError if type is unknown or data is invalid."""
        self.type = type
        if type == 'adler32':
            compute_checksum = zlib.adler32
        elif type == 'crc32':
            compute_checksum = zlib.crc32
        else:
            raise ValueError

        if computed:
            checksum, data_type, data = data.split(' ', 2)
            self.checksum = int(checksum)
            self.data = get_data(data_type, data)
            if compute_checksum(str(self.data)) != self.checksum:
                raise ValueError
        else:
            self.checksum = compute_checksum(str(data))
            self.data = data

        self.str = 'checksum {0} {1} {2}'.format(type, self.checksum,
                                                 str(self.data))

    def __eq__(self, other):
        return isinstance(other, ChecksumedData) and \
               self.type == other.type and self.data == other.data

    def __str__(self):
        return self.str

    def is_text(self):
        return self.data.is_text()

    def get_data(self):
        return self.data.get_data()

def get_data(type, data):
    """Returns a Data object corresponding to a type and data of a
    DATA string message.
    Raises ValueError if type is unknown."""
    if type == 'text':
        return Text(data)
    elif type == 'image':
        has_alpha, bits_per_sample, width, height, rowstride, pixels = \
                   data.split(' ', 5)
        return Image(bool(int(has_alpha)), int(bits_per_sample),
                     int(width), int(height), int(rowstride), pixels)
    elif type == 'compressed':
        type, data = data.split(' ', 1)
        return CompressedData(type, data)
    elif type == 'checksum':
        type, data = data.split(' ', 1)
        return ChecksumedData(type, data)
    else:
        raise ValueError
