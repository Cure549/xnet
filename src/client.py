import cmd
import socket
import threading
import array
from packet_info import WhisperOP, LoginOP
from client_utils import get_return_codes, unpack_server_response

class Client(cmd.Cmd):
    def __init__(self):
        super().__init__()
        self.prompt = '$ '
        self.sock = None
        self.is_connected = False
        self.is_logged_in = False
        self.recv_thread = None
        self.codes = get_return_codes()
        self.use_rawinput = True

    def emptyline(self):
        pass

    def do_connect(self, args):
        host, port = args.split()
        port = int(port)

        if (self.is_connected):
            print("Already connected.")
            return False

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((host, port))
            print(f'Successfully connected to {host}:{port}')
            self.prompt = "[!] $ "
            self.recv_thread = threading.Thread(
                target=self.receive_messages)
            self.recv_thread.start()
            self.is_connected = True
        except Exception as e:
            print(f'Failed to connect to {host}:{port}: {e}')

    def do_quit(self, _):
        if self.is_connected:
            self.sock.close()
            self.sock = None
            print('Disconnected from server')
            self.recv_thread.join()
            return True
        else:
            return True

    def receive_messages(self):
        self.sock.setblocking(False)

        message = None

        while self.sock:
            try:
                message = self.sock.recv(8192)
            except BlockingIOError:
                # print("blo")
                continue
            except OSError:
                # print("os")
                self.is_connected = False
                print("Connection lost")
            # if not message:
            #     continue
                # print('Connection lost')
                # self.sock = None
                # break
            # if len(message) == 2:
            #     code = array.array("h", message)
            #     print(f"{self.codes[code.pop()]}\n{self.prompt}", end="")
            # else:
            # print(f'Received: {message}\n{self.prompt}', end="")
            if message:
                unpack_server_response(message)
            else:
                self.sock.close()
                self.sock = None
                self.is_connected = False
                print("Session Expired.")
        
            

    def do_whisper(self, args):
        user, msg = args.split(" ", 1)
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
        else:
            print('Not connected to any server')


if __name__ == '__main__':
    Client().cmdloop()
