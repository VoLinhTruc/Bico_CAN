
import time
import threading

import can

bus = can.Bus(interface="serial", channel="COM4")

count = 0

while (1):
    count = (count+1)%256
    tx_msg = can.Message(
                arbitration_id=0x712,
                data=[0x30, 0xAA, 0x32, 0x33, 0x34, 0x35, 0x36, count],
            )
    bus.send(tx_msg)
    
    # rx_msg = bus.recv(0.1)
    # if rx_msg is not None:
    #     print(f"rx: {rx_msg}")
    
    # 0.015s is the smallest of sleep time, this depending on the PC/OS
    # Ref: https://stackoverflow.com/questions/1133857/how-accurate-is-pythons-time-sleep
    # Log from receiver side:
    # rx: Timestamp:     2101.316000    ID: 00000712    X Rx                DL:  8    30 aa 32 33 34 35 36 5d
    # rx: Timestamp:     2101.332000    ID: 00000712    X Rx                DL:  8    30 aa 32 33 34 35 36 5e
    # rx: Timestamp:     2101.348000    ID: 00000712    X Rx                DL:  8    30 aa 32 33 34 35 36 5f
    # rx: Timestamp:     2101.364000    ID: 00000712    X Rx                DL:  8    30 aa 32 33 34 35 36 60
    # rx: Timestamp:     2101.380000    ID: 00000712    X Rx                DL:  8    30 aa 32 33 34 35 36 61
    time.sleep(0.015)
    