import cantools
from pprint import pprint

def bytearray_to_hex(byte_array):
    ret = ''
    for b in byte_array:
        b_as_hex = hex(b).replace('0x', '')
        if len(b_as_hex) == 1:
            b_as_hex = '0' + b_as_hex
        ret += b_as_hex + ' '
    return ret

db = cantools.database.load_file('C:/D/Bico/my_tool/Bico_CAN/python_code/dbc_test/dbc_file.dbc')
db.messages

# for message in db.messages:
#     print(message)
#     for signal in message.signals:
#         attributes = vars(signal)
#         print(f'\t {signal}')
#         for key, value in attributes.items():
#             print(f"\t\t{key}: {value}")
            
ECM_TorqueControl_N2 = db.get_message_by_name('ECM_TorqueControl_N2')
print(ECM_TorqueControl_N2)
attributes = vars(ECM_TorqueControl_N2)
for key, value in attributes.items():
    print(f"\t{key}: {value}")
for signal in ECM_TorqueControl_N2.signals:
    print(f'\t {signal}')
    attributes = vars(signal)
    for key, value in attributes.items():
        print(f"\t\t{key}: {value}")
data = ECM_TorqueControl_N2.encode({'Com_tqTarEngWoTMReq': 0, 'Com_tqEstimdEngWoTMReq': 1.6, 'Com_flgFootUp': 0, 'Com_flgTqDwnInhb': 0, 'Com_stLockUpReq': 0})

print(type(data))
print(data)
for byte in data:
    print(hex(byte))

data_as_str = bytearray_to_hex(data)
print(data_as_str)