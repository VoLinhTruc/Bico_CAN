# In this example, we transmit a payload using a blocking send()
import isotp
import logging
import can
import time
from typing import Optional

TX_ADDR = 0x234
RX_ADDR = 0x123


bus = can.Bus(interface="serial", channel="COM5")


def my_error_handler(error):
    # Called from a different thread, needs to be thread safe
    logging.warning('IsoTp error happened : %s - %s' % (error.__class__.__name__, str(error)))


def my_rxfn(timeout:float) -> Optional[isotp.CanMessage]:
    # All my_hardware_something and get_something() function are fictive of course.
    msg = bus.recv(0.01) # Blocking read are encouraged for better timing.
    if msg is None:
        return None # Return None if no message available
    if msg.arbitration_id <= 0x7FF:
        msg.arbitration_id &= 0x7FF
        msg.is_extended_id = False
    print(f'{msg}')
    return isotp.CanMessage(arbitration_id=msg.arbitration_id, data=msg.data, dlc=msg.dlc, extended_id=msg.is_extended_id)


def my_txfn(isotp_msg:isotp.CanMessage):
    # all set_something functions and my_hardware_something are fictive.
    msg = can.Message()
    msg.arbitration_id = isotp_msg.arbitration_id
    msg.data = (isotp_msg.data)
    msg.dlc = (isotp_msg.dlc)
    msg.is_extended_id = (isotp_msg.is_extended_id)
    bus.send(msg)


addr = isotp.Address(isotp.AddressingMode.Normal_11bits, rxid=RX_ADDR, txid=TX_ADDR)
params = {
    'blocking_send' : True,
    'tx_padding': 0xFF
}
tp_layer = isotp.TransportLayer(address=addr, error_handler=my_error_handler, params=params, rxfn=my_rxfn, txfn=my_txfn)

def main():

    tp_layer.start()
    
    try:
        while True:
            # try:
            #     tp_layer.send(b'12345678901234567890123', send_timeout=2)    # Blocking send, raise on error
            #     # print("Payload transmission successfully completed.")     # Success is guaranteed because send() can raise
            # except isotp.BlockingSendFailure:   # Happens for any kind of failure, including timeouts
            #     print("Send failed")
            
            payload = tp_layer.recv(timeout=1)
            if payload is not None:
                print("Message: " + "".join("%02x " % b for b in payload))
                
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass  # exit normally
    
    tp_layer.stop()
    time.sleep(0.1)
    bus.shutdown()
    print("Stopped script")


if __name__ == "__main__":
    main()