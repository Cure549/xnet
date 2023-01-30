import opcode
import socket
import cmd
import struct
from dataclasses import dataclass

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
        data = MsgProto(201, do_what)
        socket.sendall(data.construct())
    elif do_what == "join":
        data = MsgProto(202, do_what)
        socket.sendall(data.construct())
    elif do_what == "debug":
        data = MsgProto(203, do_what)
        socket.sendall(data.construct())
    else:
        data = MsgProto(204, do_what)
        socket.sendall(data.construct())