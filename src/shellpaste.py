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

import sys
import clipdata
from clipsync import ClipboardManager

def main():
    if len(sys.argv) > 1:
        config_filename = sys.argv[1]
    else:
        config_filename = '~/.clipman'
    clipman = ClipboardManager((config_filename))

    d = clipman.get_clipboard()

    from twisted.internet import reactor
    def print_clipboard(clipboard):
        if clipboard.is_text():
            print clipboard.get_data()
        else:
            sys.stderr.write('Clipboard contains no text data.\n')
        reactor.stop()
    d.addCallback(print_clipboard)
    reactor.run()

if __name__ == '__main__':
    main()
