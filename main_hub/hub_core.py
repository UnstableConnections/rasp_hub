from conf_table import hubs_conf_table
from struct import *

# See: https://docs.python.org/3/library/struct.html#struct.pack

STRUCT_TYPE = 'Hs'
STRUCT_REGISTER_PACK = 'HI'
STRUCT_REGISTER_OK = 'H'
STRUCT_SET_COEF = 'HI'
STRUCT_SET_TASK = 'HI'
STRUCT_GET_Q = 'H'
STRUCT_PRODUCED_Q = 'HI'

TYPE_REGISTER    = 0x0001
TYPE_REGISTER_OK = 0x0002
TYPE_SET_COEF    = 0x0003
TYPE_SET_TASK    = 0x0004
TYPE_GET_Q       = 0x0005
TYPE_PRODUCED_Q  = 0x0006


class HubCore():
    def __init__(self):
        self.conf_table = hubs_conf_table
    

    def handle_message(data, client_addr):
        packet_type = unpack(STRUCT_TYPE, data)

        if(packet_type == TYPE_REGISTER):
            msg_type, hub_id = unpack(STRUCT_REGISTER_PACK, data)
            answer = pack(STRUCT_REGISTER_OK)
            answers.append(answer)

        elif(packet_type == TYPE_PRODUCED_Q):
            msg_type, prod_q = unpack(STRUCT_PRODUCED_Q, data)

        answers = []

        return answers