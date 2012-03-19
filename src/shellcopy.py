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

    data = sys.stdin.read()[:-1]
    clipman = ClipboardManager((config_filename))

    from twisted.internet import reactor

    def f():
        d = clipman.set_text(data)
        d.addCallback(lambda data: reactor.stop())

    reactor.callLater(0.1, f)
    reactor.run()

if __name__ == '__main__':
    main()
