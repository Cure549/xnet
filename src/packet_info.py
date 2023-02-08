import struct
from abc import ABC, abstractmethod

class BasePacket(ABC):
    opcode : int
    
    @abstractmethod
    def construct(self):
        raise NotImplementedError
    
    @abstractmethod
    def deconstruct(self):
        raise NotImplementedError

class SendMessageOP(BasePacket):
    opcode = 201

    def __init__(self, data):
        self.msg = data
        self.msg_len = len(self.msg)

    def construct(self):
        packet = struct.pack(f'!Hi{self.msg_len}s', SendMessageOP.opcode, self.msg_len, self.msg.encode('utf-8'))
        print(packet)
        return packet
    
    def deconstruct(self):
        pass


class JoinRoomOP(BasePacket):
    opcode = 202

    def __init__(self, data):
        self.msg = data
        self.msg_len = len(self.msg)

    def construct(self):
        packet = ""
        return packet

    def deconstruct(self):
        pass


class SendRoomMessageOP(BasePacket):
    opcode = 203

    def __init__(self, data):
        self.msg = data
        self.msg_len = len(self.msg)

    def construct(self):
        packet = ""
        return packet

    def deconstruct(self):
        pass