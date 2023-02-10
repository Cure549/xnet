import cmd
import socket
import threading
import array
from packet_info import WhisperOP, LoginOP
from client_utils import get_return_codes

class Client(cmd.Cmd):
    def __init__(self):
        super().__init__()
        self.prompt = '$ '
        self.sock = None
        self.recv_thread = None
        self.codes = get_return_codes()

    def emptyline(self):
        pass

    def do_connect(self, args):
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

    def do_quit(self, _):
        if self.sock:
            self.sock.close()
            self.sock = None
            print('Disconnected from server')
        else:
            print('Not connected to any server')

    def receive_messages(self, sock):
        while True:
            message = sock.recv(8192)
            if not message:
                print('Connection lost')
                self.sock = None
                break
            if len(message) == 2:
                code = array.array("h", message)
                print(f"{self.codes[code.pop()]}\n{self.prompt}", end="")
            else:
                print(f'Received: {message.decode()}\n{self.prompt}', end="")
            

    def do_whisper(self, args):
        user, msg = args.split()
        if self.sock:
            send_obj = WhisperOP(user, msg)
            self.sock.sendall(send_obj.construct())
            print(f'Sent: {user} {msg}')
        else:
            print('Not connected to any server')

    def do_shout(self, message):
        pass

    def do_login(self, creds):
        username, password = creds.split()
        if self.sock:
            send_obj = LoginOP(username, password)
            self.sock.sendall(send_obj.construct())
            # print("Attempted login.")
        else:
            print('Not connected to any server')



if __name__ == '__main__':
    Client().cmdloop()
