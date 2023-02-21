import struct
from packet_info import LoginOP, WhisperOP, JoinRoomOP, ShoutOP

def fixed_print(message):
    print(f"{message}\n$ ", end="")

def get_return_codes():
    codes = {
        0: "Action successful",
        1: "Failed to login",
        2: "Failed to join room",
        3: "Failed to whisper",
        4: "Failed to shout",
    }
    return codes

def unpack_server_response(client, data):
    format = "!h"
    format_size = struct.calcsize(format)
    opcode = struct.unpack(format, data[:format_size])[0]
    notify_deconstructor(client, opcode, data)

def notify_deconstructor(client, opcode, data):
    features = {
        LoginOP.opcode: deconstruct_login,
        WhisperOP.opcode: deconstruct_whisper,
        WhisperOP.im_target: deconstruct_whisper_target,
        JoinRoomOP.opcode: deconstruct_join_op,
        ShoutOP.opcode: deconstruct_shout_op,
    }

    deconstructor_idx = list(features.keys()).index(opcode)

    list(features.values())[deconstructor_idx](client, data)

def deconstruct_login(client, data):
    format = "!hh"
    format_size = struct.calcsize(format)
    return_code = struct.unpack(format, data[:format_size])[1]
    if 0 == return_code:
        client.is_logged_in = True

    fixed_print(get_return_codes()[return_code])

def deconstruct_whisper(client, data):
    format = "!hh"
    format_size = struct.calcsize(format)
    return_code = struct.unpack(format, data[:format_size])[1]

    fixed_print(get_return_codes()[return_code])


def deconstruct_whisper_target(client, data):
    format = f"!hi{WhisperOP.max_user_length}si{WhisperOP.max_msg_length}s"
    format_size = struct.calcsize(format)
    whisper_info = struct.unpack(format, data[:format_size])

    from_user = whisper_info[2].decode("utf-8")
    message = whisper_info[4].decode("utf-8")

    fixed_print(f"[From {from_user}] : {message}")


def deconstruct_join_op(client, data):
    format = "!hh"
    format_size = struct.calcsize(format)
    return_code = struct.unpack(format, data[:format_size])[1]

    fixed_print(get_return_codes()[return_code])

def deconstruct_shout_op(client, data):
    format = "!hh"
    format_size = struct.calcsize(format)
    return_code = struct.unpack(format, data[:format_size])[1]

    fixed_print(get_return_codes()[return_code])
