#!/usr/bin/env python

import socket, sys

def send(port, data):
    s = socket.socket()
    s.connect(("localhost", port))
    s.send("DATA {0} {1}\n".format(len(data), data))
    s.close()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        try:
            send(int(sys.argv[1]), sys.stdin.read())
        except KeyboardInterrupt:
            sys.exit(0)
        except:
            sys.stderr.write("Unable to connect\n")
            sys.exit(1)
    else:
        sys.stderr.write("Please specify as argument the port on which to " +
                         "send the clipboard as argument\n")
