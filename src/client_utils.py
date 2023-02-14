import struct
from packet_info import LoginOP, WhisperOP, JoinRoomOP, ShoutOP

def get_return_codes():
    codes = {
        0: "Action successful",
        1: "Failed to login",
        2: "Failed to join room",
        3: "Failed to whisper",
        4: "Failed to shout",
    }
    return codes

def unpack_server_response(data):
    format = "!h"
    format_size = struct.calcsize(format)
    opcode = struct.unpack(format, data[:format_size])[0]
    notify_deconstructor(opcode, data)

def notify_deconstructor(opcode, data):
    features = {
        LoginOP.opcode: deconstruct_login,
        WhisperOP.opcode: deconstruct_whisper,
        WhisperOP.im_target: deconstruct_whisper_target,
        JoinRoomOP.opcode: JoinRoomOP,
        ShoutOP.opcode: ShoutOP,
    }

    deconstructor_idx = list(features.keys()).index(opcode)

    list(features.values())[deconstructor_idx](data)

def deconstruct_login(data):
    format = "!hh"
    format_size = struct.calcsize(format)
    return_code = struct.unpack(format, data[:format_size])[1]

    print(get_return_codes()[return_code])

def deconstruct_whisper(data):
    format = "!hh"
    format_size = struct.calcsize(format)
    return_code = struct.unpack(format, data[:format_size])[1]

    print(get_return_codes()[return_code])


def deconstruct_whisper_target(data):
    format = f"!hi{WhisperOP.max_user_length}si{WhisperOP.max_msg_length}s"
    format_size = struct.calcsize(format)
    whisper_info = struct.unpack(format, data[:format_size])

    from_user = whisper_info[2].decode("utf-8")
    message = whisper_info[4].decode("utf-8")

    print(f"[From {from_user}] : {message}")
