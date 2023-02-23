import cmd
import sys
import socket
import threading
import array
from packet_info import WhisperOP, LoginOP, JoinRoomOP, ShoutOP
from client_utils import get_return_codes, unpack_server_response, fixed_print


class InputWrapper:
    def __init__(self, file_desc):
        self.file_desc = file_desc

    def readline(self, *args):
        try:
            raw = self.file_desc.readline(*args)
            return f"{raw}\n"
        except KeyboardInterrupt:
            return "\n"

class Client(cmd.Cmd):
    def __init__(self):
        super().__init__(stdin=InputWrapper(sys.stdin))
        self.prompt = "$ "
        self.sock = None
        self.is_connected = False
        self.is_logged_in = False
        self.recv_thread = None
        self.codes = get_return_codes()
        self.use_rawinput = False

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
            try:
                self.sock.close()
                self.sock = None 
            finally:
                try:
                    self.recv_thread.join()
                except AttributeError:
                    fixed_print("Closing client. No connection to server made.")
                    return True
                fixed_print("Closing connection to server.")
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
        if self.sock and self.is_logged_in and self.is_connected:
            send_obj = ShoutOP(message).construct()
            if send_obj is None:
                fixed_print("Invalid input detected.")
                return
            self.sock.sendall(send_obj)
        else:
            fixed_print("You must be connected to a server and logged in to perform this action.")

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
                continue
            except OSError:
                self.is_connected = False
                self.is_logged_in = False
                fixed_print("Connection lost")
            if message:
                unpack_server_response(self, message)
            else:
                try:
                    self.sock.close()
                    self.sock = None
                finally:
                    self.is_connected = False
                    self.is_logged_in = False
                    fixed_print("Connection lost")


if __name__ == '__main__':
    Client().cmdloop()
