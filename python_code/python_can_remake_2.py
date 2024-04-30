
import time
import threading

import can

bus = can.Bus(interface="serial", channel="COM5")

while (1):
    tx_msg = can.Message(
                arbitration_id=0x712,
                data=[0x30, 0xAA, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37],
            )
    bus.send(tx_msg)
    
    rx_msg = bus.recv(1)
    if rx_msg is not None:
        print(f"rx: {rx_msg}")
    
    time.sleep(1)
