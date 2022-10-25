import opcode
import socket
import os
import ctypes
import string
import struct
from dataclasses import dataclass

@dataclass
class MsgProto:
    opcode: int
    length: int
    msg: str
    def __bytes__(self):
        return f'{self.opcode}{self.length}{self.msg}'.encode()




host = "127.0.0.1"  # The server's hostname or IP address
port = 47007  # The port used by the server
socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
data = MsgProto(4, 17, "heyo")
print(data.__bytes__())
print(bytes(data))

while True:
    do_what = input("command > ")

    if do_what == "connect":
        socket.connect((host, port))
    else:
        socket.sendall(bytes(data))
        