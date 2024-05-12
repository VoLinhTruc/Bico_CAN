import can

bus1 = can.interface.Bus('test', interface='virtual')
bus2 = can.interface.Bus('test', interface='virtual')

msg2 = None
print(msg2)

msg1 = can.Message(arbitration_id=0xabcde, data=[1,2,3])
bus1.send(msg1)

msg2 = bus2.recv()
print(msg2)

#assert msg1 == msg2
assert msg1.arbitration_id == msg2.arbitration_id
assert msg1.data == msg2.data
assert msg1.timestamp != msg2.timestamp