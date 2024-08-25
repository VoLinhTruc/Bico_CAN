import can
import struct

CAN_TX_ID = 0x791
CAN_RX_ID = 0x719


bus = can.Bus(interface="serial", channel="COM10")

data = []

while (1):    
    rx_msg = bus.recv(5)
    if rx_msg is not None:
        if (rx_msg.arbitration_id == CAN_RX_ID):
            print(f"rx: {rx_msg}") 
            if rx_msg.data[1] == 0x36:
                data.extend(rx_msg.data[2:6])
    
    if (rx_msg.arbitration_id == CAN_RX_ID):     
        tx_msg = can.Message(
                    arbitration_id=CAN_TX_ID,
                    data=[0x07, rx_msg.data[1] + 0x40, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55],
                )
        bus.send(tx_msg)
        
    if (rx_msg.arbitration_id == CAN_RX_ID) and (rx_msg.data[1] == 0x37):
        break

with open("retriving.bin", "wb") as file:
    # Convert the list of hex bytes to a bytes object
    byte_data = bytes(data)
    # Write the bytes object to the binary file
    file.write(byte_data)