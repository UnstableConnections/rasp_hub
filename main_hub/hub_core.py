from conf_table import hubs_conf_table
from struct import *
import time

# See: https://docs.python.org/3/library/struct.html#struct.pack

STRUCT_TYPE = '<H'
STRUCT_REGISTER_PACK = '<HIII'
STRUCT_REGISTER_OK = '<H'
STRUCT_SET_COEF = '<HI'
STRUCT_SET_TASK = '<HII'
STRUCT_GET_Q = '<H'
STRUCT_PRODUCED_Q = '<HIII'
STRUCT_INFORM = '<HI'

TYPE_REGISTER    = 0x0001
TYPE_REGISTER_OK = 0x0002
TYPE_SET_COEF    = 0x0003
TYPE_SET_TASK    = 0x0004
TYPE_GET_Q       = 0x0005
TYPE_PRODUCED_Q  = 0x0006
TYPE_INFORM_Q    = 0x0007

STATE_INITED     = 0
STATE_REGISTERED = 1
STATE_CONFIGURED = 2
STATE_TASK_SET   = 3
STATE_NEED_GET_Q = 4
STATE_INFORM     = 5

HUB_INFORM = 998
HUB_MAIN   = 999


class HubCore():
    def __init__(self):
        self.conf_table = hubs_conf_table
    

    def handle_message(self, data, client_addr):
        answers = []
        print("data size: " + str(len(data)))
        print("get new packet, parse type")
        packet_type = unpack(STRUCT_TYPE, data[:2])[0]

        print("packet type: " + str(packet_type))

        if packet_type == TYPE_REGISTER:
            register_pack = unpack(STRUCT_REGISTER_PACK, data)
            print("register_pack: " + str(register_pack))
            msg_type, hub_id, hub_state, hub_task_time = register_pack

            print("msg_type: " + str(msg_type) + " hub_id: " + str(hub_id) + " hub_state: " + str(hub_state) + " hub_task_time: " + str(hub_task_time))
            
            if hub_id in self.conf_table:

                if hub_state == STATE_INITED:
                    answer = pack(STRUCT_REGISTER_OK, TYPE_REGISTER_OK)
                    answers.append(answer)
                elif hub_state == STATE_REGISTERED:
                    answer = pack(STRUCT_SET_COEF, TYPE_SET_COEF, int(self.conf_table[hub_id]['Q'] * 100))
                    answers.append(answer)
                elif hub_state == STATE_CONFIGURED:
                    task_time = int(time.time())
                    answer = pack(STRUCT_SET_TASK, TYPE_SET_TASK, 600, task_time) # Need algo mock
                    self.conf_table[hub_id]['set_task_time'] = task_time
                    answers.append(answer)
                elif hub_state == STATE_TASK_SET:
                    need_get_q = False
                    cur_time = int(time.time())

                    if self.conf_table[hub_id]['set_task_time'] != hub_task_time:
                        self.conf_table[hub_id]['set_task_time'] = hub_task_time

                    diff = cur_time - self.conf_table[hub_id]['set_task_time']
                    print("Diff check for hub: " + str(diff))
                    if diff > 60:
                        need_get_q = True
                    
                    print("need_get_q: " + str(need_get_q))
                    if need_get_q == True:
                        print("Send TYPE_GET_Q")
                        answer = pack(STRUCT_GET_Q, TYPE_GET_Q)
                        answers.append(answer)
                elif hub_state == STATE_INFORM:
                    answer = pack(STRUCT_INFORM, TYPE_INFORM_Q, int(self.conf_table[HUB_MAIN]['vanadium'] * 100))
                    answers.append(answer)

        elif packet_type == TYPE_PRODUCED_Q:
            print("Get TYPE_PRODUCED_Q")
            register_pack = unpack(STRUCT_REGISTER_PACK, data)
            print("register_pack: " + str(register_pack))

            msg_type, hub_id, hub_state, prod_q = unpack(STRUCT_PRODUCED_Q, data)
            prod_q = prod_q / 100.
            print("msg_type: " + str(msg_type) + " hub_id: " + str(hub_id) + " hub_state: " + str(hub_state) + " prod_q: " + str(prod_q))

            self.conf_table[HUB_MAIN]['vanadium'] = self.conf_table[HUB_MAIN]['vanadium'] + prod_q 

            task_time = int(time.time())
            answer = pack(STRUCT_SET_TASK, TYPE_SET_TASK, 600, task_time)
            self.conf_table[hub_id]['set_task_time'] = task_time
            answers.append(answer)
        else:
            print("Unknown packet type")
        
        return answers