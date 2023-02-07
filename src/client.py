import cmd
import socket
import threading
from packet_info import SendMessageOP

class Client(cmd.Cmd):
    def __init__(self):
        super().__init__()
        self.prompt = '$ '
        self.sock = None
        self.recv_thread = None

    def postloop(self):
        print("jkl")

    def emptyline(self):
        pass


    def do_connect(self, args):
        'Move the turtle forward by the specified distance:  FORWARD 10'
        host, port = args.split()
        port = int(port)

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((host, port))
            print(f'Successfully connected to {host}:{port}')
            self.prompt = "[!] $ "
            self.recv_thread = threading.Thread(
                target=self.receive_messages, args=(self.sock,))
            self.recv_thread.start()
        except Exception as e:
            print(f'Failed to connect to {host}:{port}: {e}')

    def do_disconnect(self, _):
        if self.sock:
            self.sock.close()
            self.sock = None
            print('Disconnected from server')
        else:
            print('Not connected to any server')

    def receive_messages(self, sock):
        while True:
            message = sock.recv(1024)
            if not message:
                print('Connection lost')
                self.sock = None
                break
            print()
            print(f'Received: {message.decode().strip()}\n{self.prompt}', end="")
            # print(self.prompt, end="")

    def do_send(self, message):
        if self.sock:
            send_obj = SendMessageOP(message)
            self.sock.sendall(send_obj.construct())
            print(f'Sent: {message}')
        else:
            print('Not connected to any server')


if __name__ == '__main__':
    Client().cmdloop()
