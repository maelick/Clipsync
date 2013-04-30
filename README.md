Clypsync
========

Clipsync is a clipboard synchronizer, i.e. it is a software tool which
provide a solution for copy-pasting between two or more computers
across a computer network.

Deprecated README
=================

As the tool is in a state of complete rewriting the following sections
are deprecated and will be rewritten in the future.

Presentation
------------

Clipsync is a software which can synchronize clipboards among computers
over a LAN. It is composed of several tools to do this.
Clipsync is the main tool being the network frontend.
Shellclip and GTKClip are the frontend with the user communicating with
Clipsync. Shellclip is composed of two scripts, Shellcopy and Shellpaste,
that can be used to send and receive the clipboard on stdin/stdout.
GTKClip uses GTK to synchronize the Clipsync's clipboard with the
system's clipboard.

Required softwares
------------------

- POCO >=1.3.6 for Clipsync: http://www.pocoproject.org
- Python 2.6 or 2.7 for Shellclip and GTKClip
- PyGTK >=2.10 for GTKClip: http://www.pygtk.org

Optional software
-----------------

- Doxygen can be used to generate LaTeX and HTML documentation.

Installation
------------

To compile and install do:
make
make install

To compile documentation with Doxygen:
make doc

As this project has been developed as part of a University project, there
is a report of 42 pages in French which can be compiled with LaTeX:
make report

Usage
-----

clipsync <xml_file>
shellcopy <port>
shellpaste <port>
gtkclip <port>

<xml_file> is a XML file which contains the configuration of Clipsync.
<port> is the local port used by Clipsync in the XML file.
For both see below

XML Configuration
-----------------

The XML file can be generated with the script clipsync_genconfig shipped
with Clipsync. The script must have passed as parameter the peer name,
the group name and the network interface. Peer name and group name
must be composed from alphanumerical characters only.

Optional arguments are port, broadcast port, local port and password
and salt length (which are pseudo-random generated string). The file
is printed on stdout and so must be redirected to a file.

The different XML markup used:
* net_frontend
  * interface: network interface used.
  * use_ipv6: boolean which is true if IPv6 should be used. Not working
    for the moment (feature planned).
  * port: TCP port used for incoming connections
  * bcast_port: UDP broadcast port used.
  * bcast_interval: interval in milliseconds used for broadcast sending.
  * peer_name: name for this peer (computer), should be unique.
  * group: name of the group of peers, should be the same for all your
    peers (computers).
  * passphrase and saltsalt: passphrase and salt used for AES. Must
    be the same on every computer.
  * keepalive_delay: delay in milliseconds before closing the connection
    if no keep alive is received.
  * keepalive_interval: interval in milliseconds used between each keep
    alive.
  * verbose, verbose_bcast, verbose_peer: booleans used to activate verbose
    mode.
* local_frontend
  * local_port: local TCP port used to communicate with Shellclip and
    GTKClip.
  * verbose: boolean used to activate verbose mode.
