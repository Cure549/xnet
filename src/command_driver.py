import cmd

class CommandDriver(cmd.Cmd):
    intro = "Type help or ? to list commands.\n"
    prompt = "-> "

    def __init__(self):
        super().__init__(self)

    def do_connect(self, line):
        socket.connect((host, port))

    def do_send_msg(self, line):
        data = MsgProto(201, line)
        socket.sendall(data.construct())

    def do_join_room(self, line):
        pass

    def do_EOF(self, line):
        return True
