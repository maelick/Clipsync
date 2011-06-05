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

import random, sys

def get_random_string(n):
    """
    Returns a random string of a given length
    """
    s = ""
    for i in xrange(n):
        s += chr(random.randrange(94) + 33)
    return s

def replace_special_char(s):
    """
    Replace all special XML characters (<, > and &).
    """
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

def print_file(peer_name, group_name, interface, port="2525",
               bcast_port="4242", local_port="1111", passphrase_len=16,
               salt_len=16):
    """
    Prints to stdout the generated configuration file
    """
    peer_name = replace_special_char(peer_name)
    group_name = replace_special_char(group_name)
    interface = replace_special_char(interface)
    port = replace_special_char(port)
    bcast_port = replace_special_char(bcast_port)
    local_port = replace_special_char(local_port)
    passphrase = replace_special_char(get_random_string(int(passphrase_len)))
    salt = replace_special_char(get_random_string(int(salt_len)))
    print ("<clipsync>\n" +
           "	<net_frontend>\n" +
           "		<interface>{2}</interface>\n" +
           "		<use_ipv6>false</use_ipv6>\n" +
           "		<port>{3}</port>\n" +
           "		<bcast_port>{4}</bcast_port>\n" +
           "		<bcast_interval>5000</bcast_interval>\n" +
           "		<peer_name>{0}</peer_name>\n" +
           "		<group>{1}</group>\n" +
           "		<passphrase>{6}</passphrase>\n" +
           "		<salt>{7}</salt>\n" +
           "		<keepalive_delay>10000</keepalive_delay>\n" +
           "		<keepalive_interval>2000</keepalive_interval>\n" +
           "		<verbose>false</verbose>\n" +
           "		<verbose_bcast>false</verbose_bcast>\n" +
           "		<verbose_peer>false</verbose_peer>\n" +
           "	</net_frontend>\n" +
           "	<local_frontend>\n" +
           "		<local_port>{5}</local_port>\n" +
           "		<verbose>false</verbose>\n" +
           "	</local_frontend>\n" +
           "</clipsync>").format(peer_name, group_name, interface,
                                 port, bcast_port, local_port,
                                 passphrase, salt)

if __name__ == "__main__":
    if len(sys.argv) > 3 and len(sys.argv):
        print_file(*sys.argv[1:])
    else:
        sys.stderr.write("Please specify as argument at least the peer " +
                         "name, the group name and the interface to use.\n")
        sys.stderr.write("Optional arguments are port, broadcast port, " +
                         "local port, passphrase length and salt length.\n")
