import asyncio
import cmd
import socket
from packet_info import SendMessageOP

class Client(cmd.Cmd):
    def __init__(self, loop):
        super().__init__()
        self.prompt = '$ '
        self.loop = loop
        self.sock = None

    def do_connect(self, args):
        host, port = args.split()
        port = int(port)

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((host, port))
            asyncio.ensure_future(self.receive_messages(self.sock))
            print(f'Successfully connected to {host}:{port}')
            self.prompt = "[!] $ "
        except Exception as e:
            print(f'Failed to connect to {host}:{port}: {e}')

    def do_disconnect(self, _):
        if self.sock:
            self.sock.close()
            self.sock = None
            print('Disconnected from server')
        else:
            print('Not connected to any server')

    async def receive_messages(self, sock):
        while True:
            message = sock.recv(1024)
            if not message:
                print('Connection lost')
                self.sock = None
                break
            print(f'Received: {message.decode().strip()}')

    def do_send(self, message):
        if self.sock:
            send_obj = SendMessageOP(message)
            self.sock.sendall(send_obj.construct())
            print(f'Sent: {message}')
        else:
            print('Not connected to any server')


if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    client = Client(loop)
    client.cmdloop()
