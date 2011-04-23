#!/usr/bin/env python
import socket, sys

def get(port):
    s = socket.socket()
    s.connect(("localhost", port))
    s.send("GET\n")
    data = s.recv(1024).split(" ")
    length = int(data[1])
    data = " ".join(data[2:])
    while(len(data) < length):
        data += s.recv(1024)
    return data[:length]
    s.close()

if __name__ == "__main__":
    try:
        print get(int(sys.argv[1]))
    except KeyboardInterrupt:
        sys.exit(0)
    except:
        sys.stderr.write("Please specify as argument the port on which to " +
                         "get the clipboard as argument\n")
        sys.exit(1)
