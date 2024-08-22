
import time
import can
from datetime import datetime


CAN_TX_ID = 0x169
CAN_RX_ID = 0x196


def read_bin_file(bin_file):
    with open(bin_file, 'rb') as file:
        data = file.read()
    print(len(data))
    print(len(data)%4)
    return data

def send_can_messages(bus, can_tx_id, can_rx_id, data, chunk_size=4):
    msg = can.Message(arbitration_id=can_tx_id,
                          data=[0x07, 0x34, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55],
                          is_extended_id=False)
    try:
        bus.send(msg)
        print(f"tx: {msg}")
    except can.CanError:
        print("Message NOT sent")
    
    rx_msg = bus.recv(20)
    if rx_msg is not None:
        print(f"rx: {rx_msg}")
        
        
    if (rx_msg.arbitration_id == can_rx_id) and (rx_msg.data[1] == 0x74):
        for i in range(0, len(data), chunk_size):
            data_chunk = [0x08, 0x36]
            data_chunk.extend(data[i:i+chunk_size])
            data_chunk.extend([0xFF, 0xFF])
            print(f'\t\t{round(i*100/len(data))}%')
            msg = can.Message(arbitration_id=can_tx_id,
                            data=data_chunk,
                            is_extended_id=False)
            try:
                bus.send(msg)
                print(f"tx: {msg}")
            except can.CanError:
                print("Message NOT sent")
            time.sleep(0.02)
            
            rx_msg = bus.recv(1)
            if rx_msg is not None:
                if (rx_msg.arbitration_id == can_rx_id) and (rx_msg.data[1] == 0x76):
                    continue
                else:
                    print("flashing Failed")
                    break
            
            
    msg = can.Message(arbitration_id=can_tx_id,
                          data=[0x07, 0x37, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55],
                          is_extended_id=False)
    try:
        bus.send(msg)
        print(f"tx: {msg}")
    except can.CanError:
        print("Message NOT sent")


bus = can.Bus(interface="serial", channel="COM4")


bin_data = read_bin_file("flashing.bin")

start_time = datetime.now().time()
start_time_total_seconds = start_time.hour * 3600 + start_time.minute * 60 + start_time.second
send_can_messages(bus, CAN_TX_ID, CAN_RX_ID, bin_data)
stop_time = datetime.now().time()
stop_total_seconds = stop_time.hour * 3600 + stop_time.minute * 60 + stop_time.second

print(f'Total time: {stop_total_seconds - start_time_total_seconds} (s)')