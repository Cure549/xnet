# pylint: disable=missing-function-docstring
from cmd import Cmd
import asyncio
import sys


class ChatClientProtocol(asyncio.Protocol):
    def __init__(self, loop):
        self.loop = loop

    def connection_made(self, transport):
        self.transport = transport
        print("Successfully connected to server.")

    def data_received(self, data):
        try:
            print('Data received: {!r}'.format(data.decode()))
        except Exception as e:
            print('Error decoding data: {}'.format(e))

    def connection_lost(self, exc):
        print('The server closed the connection')
        print('Stop the event loop')
        self.loop.stop()


async def send_message(transport, message):
    try:
        transport.write(message.encode())
        print('Message sent: {!r}'.format(message))
        await asyncio.sleep(1)
    except Exception as e:
        print('Error sending message: {}'.format(e))

class InputWrapper:
    def __init__(self, file_desc):
        self.file_desc = file_desc

    def readline(self, *args):
        try:
            raw = self.file_desc.readline(*args)
            return raw
        except KeyboardInterrupt:
            return "\n"


class CommandDriver(Cmd):
    def __init__(self, loop):
        super().__init__(stdin=InputWrapper(sys.stdin))
        self.intro = "Type help or ? to list commands.\n"
        self.prompt = "$ "
        self.use_rawinput = False
        self.transport = None
        self.loop = loop
        self.host = "localhost"
        self.port = 47007
    
    async def start(self):
        await self.do_connect(None)
        self.cmdloop()

    def precmd(self, line):
        if line == "EOF":
            return "exit"
        print()
        return line

    async def do_connect(self, line):
        line = None
        
        try:
            self.transport, protocol = await self.loop.create_connection(
                lambda: ChatClientProtocol(self.loop),
                self.host, self.port)
        except Exception as e:
            print('Error connecting to server: {}'.format(e))

    async def do_send(self, message):
        try:
            await send_message(self.transport, message)
        except Exception as e:
            print('Error sending message: {}'.format(e))

    def do_exit(self, arg):
        try:
            self.transport.close()
            return True
        except Exception as e:
            print('Error quitting: {}'.format(e))

    def do_test(self, line):
        print(line[0])


# if __name__ == '__main__':
#     try:
#         client = ChatClient()
#         client.loop = asyncio.get_event_loop()
#         # client.loop.run_until_complete(client.cmdloop())
#     except Exception as e:
#         print('Error in main loop: {}'.format(e))
#     finally:
#         client.loop.close()
