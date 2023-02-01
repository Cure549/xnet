import opcode
import socket
import cmd
import struct
from dataclasses import dataclass
import asyncio

class EventHandler(asyncio.Protocol):
    def __init__(self, message, loop):
        self.message = message
        self.loop = loop

    def connection_made(self, transport):
        transport.write(self.message.encode())
        print('Data sent: {!r}'.format(self.message))

    def data_received(self, data):
        print('Data received: {!r}'.format(data.decode()))

    def connection_lost(self, exc):
        print('The server closed the connection')
        print('Stop the event loop')
        self.loop.stop()


@dataclass
class MsgProto:
    opcode: int
    msg: str
    
    def construct(self):
        packet = struct.pack(f'!Hi{len(self.msg)}s', self.opcode, len(self.msg), self.msg.encode('utf-8'))
        return packet

host = "127.0.0.1"  # The server's hostname or IP address
port = 47007  # The port used by the server
socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


# class CommandDriver(cmd.Cmd):
#     intro = "Type help or ? to list commands.\n"
#     prompt = "> "

#     def do_connect(self, line):
#         socket.connect((host, port))

#     def do_send_msg(self, line):
#         data = MsgProto(201, line)
#         socket.sendall(data.construct())

#     def do_join_room(self, line):
#         pass

#     def do_EOF(self, line):
#         return True


# if __name__ == '__main__':
#     CommandDriver().cmdloop()

while True:
    do_what = input("command > ")

    if do_what == "connect":
        socket.connect((host, port))
    elif do_what == "send":
        pdata = MsgProto(201, do_what)
        socket.sendall(pdata.construct())
    elif do_what == "join":
        pdata = MsgProto(202, do_what)
        socket.sendall(pdata.construct())
    elif do_what == "debug":
        pdata = MsgProto(203, do_what)
        socket.sendall(pdata.construct())
    else:
        pdata = MsgProto(204, do_what)
        socket.sendall(pdata.construct())