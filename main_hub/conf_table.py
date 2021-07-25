HUB_TEST = 0
HUM_MAIN = 999


STATE_INITED = 0
STATE_REGISTERED = 1
STATE_CONFIGURED = 2
STATE_TASK_SET = 3
STATE_NEED_GET_Q = 4

hubs_conf_table = {
    HUB_TEST: {
            'Q': 0.26,            
        },
    HUM_MAIN : {
            'vanadium': 0
        }
}