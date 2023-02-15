import cmd
import socket
import threading
import array
from packet_info import WhisperOP, LoginOP, JoinRoomOP
from client_utils import get_return_codes, unpack_server_response, fixed_print

class Client(cmd.Cmd):
    def __init__(self):
        super().__init__()
        self.prompt = "$ "
        self.sock = None
        self.is_connected = False
        self.is_logged_in = False
        self.recv_thread = None
        self.codes = get_return_codes()
        self.use_rawinput = True

    def emptyline(self):
        pass

    def do_connect(self, args):
        if self.is_connected:
            fixed_print("Already connected.")
            return False

        host, port = args.split()
        port = int(port)

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((host, port))
            print(f'Successfully connected to {host}:{port}')
            self.recv_thread = threading.Thread(
                target=self.receive_messages)
            self.recv_thread.start()
            self.is_connected = True
        except Exception as e:
            fixed_print(f'Failed to connect to {host}:{port}: {e}')

    def do_quit(self, _):
        if self.is_connected:
            self.sock.close()
            self.sock = None
            fixed_print('Disconnected from server')
            self.recv_thread.join()
            return True
        else:
            return True
        
    def do_whisper(self, args):
        user, msg = args.split(" ", 1)
        if self.sock and self.is_logged_in and self.is_connected:
            send_obj = WhisperOP(user, msg).construct()
            if send_obj is None:
                fixed_print("Invalid input detected.")
                return
            self.sock.sendall(send_obj)
        else:
            fixed_print('Not connected to any server')

    def do_join(self, room_name):
        if self.sock and self.is_logged_in and self.is_connected:
            send_obj = JoinRoomOP(room_name).construct()
            if send_obj is None:
                fixed_print("Invalid input detected.")
                return
            self.sock.sendall(send_obj)
        else:
            fixed_print("You must be connected to a server and logged in to perform this action.")

    def do_shout(self, message):
        pass

    def do_login(self, creds):
        if self.is_logged_in:
            fixed_print("Already logged in.")
            return False

        username, password = creds.split()
        if self.sock:
            send_obj = LoginOP(username, password).construct()
            if send_obj is None:
                fixed_print("Invalid input detected.")
                return
            self.sock.sendall(send_obj)
        else:
            fixed_print('Not connected to any server')

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
                self.is_logged_in = False
                fixed_print("Connection lost")
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
                unpack_server_response(self, message)
            else:
                self.sock.close()
                self.sock = None
                self.is_connected = False
                self.is_logged_in = False
                fixed_print("Session Expired.")

if __name__ == '__main__':
    Client().cmdloop()
