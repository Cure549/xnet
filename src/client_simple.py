import socket
import os
import ctypes
import struct

host = "127.0.0.1"  # The server's hostname or IP address
port = 47007  # The port used by the server
socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

while True:
    do_what = input("command > ")

    if do_what == "connect":
        socket.connect((host, port))
    else:
        socket.sendall(do_what.encode())
        