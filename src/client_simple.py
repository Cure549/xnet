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
    msg: str
    
    def construct(self):
        packet = struct.pack(f'!HH{len(self.msg)}s', self.opcode, len(self.msg), self.msg.encode('utf-8'))
        return packet

host = "127.0.0.1"  # The server's hostname or IP address
port = 47007  # The port used by the server
socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
data = MsgProto(65534, "jello")
print(data.construct())

while True:
    do_what = input("command > ")

    if do_what == "connect":
        socket.connect((host, port))
    else:
        socket.sendall(data.construct())
        