import struct
from abc import ABC, abstractmethod

class BasePacket(ABC):
    opcode : int
    
    @abstractmethod
    def construct(self):
        raise NotImplementedError

class LoginOP(BasePacket):
    opcode = 200

    def __init__(self, username, password):
        self.username = username
        self.username_len = len(self.username)
        self.password = password
        self.password_len = len(self.password)

    def construct(self):
        format = f'!Hi{self.username_len}si{self.password_len}s'
        packet = struct.pack(format, LoginOP.opcode, self.username_len, self.username.encode('utf-8'), self.password_len, self.password.encode('utf-8'))
        return packet

class WhisperOP(BasePacket):
    opcode = 201
    im_target = 299
    max_msg_length = 256
    max_user_length = 32

    def __init__(self, to_user, data):
        self.to_user = to_user
        self.to_user_len = len(self.to_user)
        self.msg = data
        self.msg_len = len(self.msg)

    def construct(self):
        if self.msg_len > WhisperOP.max_msg_length:
            return

        format = f'!Hi{self.to_user_len}si{self.msg_len}s'
        packet = struct.pack(format, WhisperOP.opcode,
                             self.to_user_len, self.to_user.encode('utf-8'), self.msg_len, self.msg.encode('utf-8'))
        return packet


class JoinRoomOP(BasePacket):
    opcode = 202
    max_room_name_len = 32

    def __init__(self, data):
        self.room_name = data
        self.room_name_len = len(self.room_name)

    def construct(self):
        if self.room_name_len > JoinRoomOP.max_room_name_len:
            return

        format = f"!Hi{self.room_name_len}s"
        packet = struct.pack(format, JoinRoomOP.opcode, self.room_name_len, self.room_name.encode("utf-8"))
        return packet


class ShoutOP(BasePacket):
    opcode = 203

    def __init__(self, data):
        self.msg = data
        self.msg_len = len(self.msg)

    def construct(self):
        if self.msg_len > WhisperOP.max_msg_length:
            return

        format = f"!Hi{self.msg_len}s"
        packet = struct.pack(format, ShoutOP.opcode,
                             self.msg_len, self.msg.encode("utf-8"))
        return packet
