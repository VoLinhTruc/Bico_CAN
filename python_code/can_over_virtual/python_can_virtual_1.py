import time
import can

bus1 = can.interface.Bus('test', interface='virtual')
bus2 = can.interface.Bus('test', interface='virtual')

while True:
    msg2 = None
    # print(f"msg2 = {msg2}")

    bus1.send(can.Message(arbitration_id=0x712, data=[1]))
    bus1.send(can.Message(arbitration_id=0x712, data=[2]))
    bus1.send(can.Message(arbitration_id=0x712, data=[3]))
    
    while True:
        msg2 = bus2.recv(0.01)
        if msg2 == None:
            break
        else:
            # assert msg1 == msg2
            print(msg2)
    
    time.sleep(1)
    
bus1.shutdown()
bus2.shutdown()