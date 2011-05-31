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

import random, sys, commands, time

def get_random_string(n):
    """
    Returns a random string of a given length
    """
    s = ""
    for i in xrange(n):
        s += chr(random.randrange(26) + 97)
    return s

def output(port):
    s = get_random_string(random.randrange(10, 1000))
    out = commands.getoutput('echo "{0}" | ./shellcopy.py {1}'.format(port, s))

def loop(port):
    while True:
        output(port)
        time.sleep(random.randrange(1, 10))

if __name__ == "__main__":
    if len(sys.argv) > 1:
        loop(sys.argv[1])
    else:
        sys.stderr.write("Please specify as argument the port on which to " +
                         "send the clipboard.\n")
