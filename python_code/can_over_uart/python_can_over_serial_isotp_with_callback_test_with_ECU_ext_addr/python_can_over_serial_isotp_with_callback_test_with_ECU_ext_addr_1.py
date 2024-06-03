# In this example, we transmit a payload using a blocking send()
import isotp
import logging
import can
import time
from typing import Optional


TX_ADDR = 0x18da10f1
RX_ADDR = 0x18daf110


bus = can.Bus(interface="serial", channel="COM4")


def my_error_handler(error):
    # Called from a different thread, needs to be thread safe
    logging.warning('IsoTp error happened : %s - %s' % (error.__class__.__name__, str(error)))


def my_rxfn(timeout:float) -> Optional[isotp.CanMessage]:
    # All my_hardware_something and get_something() function are fictive of course.
    msg = bus.recv(0.1) # Blocking read are encouraged for better timing.
    if msg is None:
        return None # Return None if no message available
    if msg.arbitration_id <= 0x7FF:
        msg.arbitration_id &= 0x7FF
        msg.is_extended_id = False
    # if (msg.arbitration_id == RX_ADDR):
    # # if 1:
    #     print(f'my_rxfn: {msg}')
    return isotp.CanMessage(arbitration_id=msg.arbitration_id, data=msg.data, dlc=msg.dlc, extended_id=msg.is_extended_id)


def my_txfn(isotp_msg:isotp.CanMessage):
    # all set_something functions and my_hardware_something are fictive.
    msg = can.Message()
    msg.is_rx = False
    msg.arbitration_id = isotp_msg.arbitration_id
    msg.data = (isotp_msg.data)
    msg.dlc = (isotp_msg.dlc)
    msg.is_extended_id = (isotp_msg.is_extended_id)
    # print(f'my_txfn: {msg}')
    bus.send(msg)

addr = isotp.Address(isotp.AddressingMode.Normal_29bits, rxid=RX_ADDR, txid=TX_ADDR)

params = {
    'blocking_send' : True,
    'tx_padding': 0x00
}
tp_layer = isotp.TransportLayer(address=addr, error_handler=my_error_handler, params=params, rxfn=my_rxfn, txfn=my_txfn)


communication_mode = input('Type mode to use: Auto Sequence Request - "seq" or Manual Request - "man": ')
# communication_mode = "seq"
sequence_file = None
# sequence_file_path = ""
sequence_file_path = 'seq1.txt'

if communication_mode == "seq":
    if sequence_file_path == "":
        sequence_file_path = input('Input sequence file path: ')
    sequence_file = open(sequence_file_path, 'r')

def main():
    tp_layer.start()
    try:
        if communication_mode == "seq":
            lines = sequence_file.readlines()
            for request in lines:
                request = request.replace('\n', '')
                bytes_as_str = str(request).split(' ')
                bytes = [int(x, 16) for x in bytes_as_str]
                # print(bytes)
                try:
                    tp_layer.send(bytes, send_timeout=0.0001)
                    # print("Payload transmission successfully completed.")     # Success is guaranteed because send() can raise
                except isotp.BlockingSendFailure:   # Happens for any kind of failure, including timeouts
                    # print("Send failed")
                    pass
                    
                payload = tp_layer.recv(timeout=1)
                if payload is not None:
                    print(f'Response: ' + "".join("%02x " % b for b in payload))
                    
                
        if communication_mode == "man":
            while(1):
                request = input("Request: ")
                tp_layer.clear_rx_queue()
                request = request.replace('\n', '')
                bytes_as_str = str(request).split(' ')
                bytes = [int(x, 16) for x in bytes_as_str]
                # print(bytes)
                try:
                    tp_layer.send(bytes, send_timeout=0.1) # send timeout unit is s
                    # print("Payload transmission successfully completed.")     # Success is guaranteed because send() can raise
                except isotp.BlockingSendFailure:   # Happens for any kind of failure, including timeouts
                    # print("Send failed")
                    pass
                    
                payload = tp_layer.recv(timeout=1)
                if payload is not None:
                    print(f'Response: ' + "".join("%02x " % b for b in payload))
        

    except KeyboardInterrupt:
        pass  # exit normally
    
    tp_layer.stop()
    time.sleep(0.1)
    bus.shutdown()
    print("Stopped script")


if __name__ == "__main__":
    main()

sequence_file.close()