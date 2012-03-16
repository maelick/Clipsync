import pygtk
pygtk.require('2.0')
import gtk, gobject

class Clip:
    def __init__(self):
        self.clip = gtk.clipboard_get()
        gobject.timeout_add(500, self.fetch_clip)

    def fetch_clip(self):
        if self.clip.wait_is_text_available():
            self.clip.request_text(self.get_text)
        if self.clip.wait_is_image_available():
            self.clip.request_image(self.get_img)
        gobject.timeout_add(500, self.fetch_clip)

    def get_text(self, clip, text, data):
        print text

    def get_img(self, clip, img, data):
        has_alpha = img.get_has_alpha()
        bits_per_sample = img.get_bits_per_sample()
        width = img.get_width()
        height = img.get_height()
        rowstride = img.get_rowstride()
        pixels = img.get_pixels()
        print has_alpha, bits_per_sample, width, height, rowstride
        print self.get_pixbuf(pixels, has_alpha, bits_per_sample,
                              width, height, rowstride)

    def get_pixbuf(self, pixels, has_alpha, bits_per_sample,
                   width, height, rowstride):
        return gtk.gdk.pixbuf_new_from_data(pixels, gtk.gdk.COLORSPACE_RGB,
                                            has_alpha, bits_per_sample, width,
                                            height, rowstride)

    def set_clip(self, data):
        if type(data) is str:
            self.clip.set_text(data, len(data))
        elif isinstance(data, gtk.gdk.Pixbuf):
            self.clip.set_img(data)

clip = Clip()
gtk.main()
