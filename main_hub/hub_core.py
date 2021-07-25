from conf_table import hubs_conf_table
from struct import *

# See: https://docs.python.org/3/library/struct.html#struct.pack

STRUCT_TYPE = 'Hs'
STRUCT_REGISTER_PACK = 'HII'
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

        if packet_type == TYPE_REGISTER:
            msg_type, hub_id, hub_state = unpack(STRUCT_REGISTER_PACK, data)

            if hub_id in self.conf_table:

                if hub_state == STATE_INITED:
                    answer = pack(STRUCT_REGISTER_OK, TYPE_REGISTER_OK)
                    answers.append(answer)
                elif hub_state == STATE_REGISTERED:
                    answer = pack(STRUCT_SET_COEF, TYPE_SET_COEF, self.conf_table[hub_id]['Q'] * 100 )
                    answers.append(answer)
                elif hub_state == STATE_CONFIGURED:
                    answer = pack(STRUCT_SET_TASK, TYPE_SET_TASK, 600) # Need algo mock
                    answers.append(answer)
                elif hub_state == STATE_TASK_SET:
                    need_get_q = False # Add timer

                    if need_get_q:
                        answer = pack(STRUCT_GET_Q, TYPE_GET_Q)
                        answers.append(answer)

        elif packet_type == TYPE_PRODUCED_Q:
            msg_type, prod_q = unpack(STRUCT_PRODUCED_Q, data)
            self.conf_table[HUM_MAIN]['vanadium'] = self.conf_table[HUM_MAIN]['vanadium'] + prod_q

            answer = pack(STRUCT_SET_TASK, TYPE_SET_TASK, 600)
            answers.append(answer)

        answers = []

        return answers