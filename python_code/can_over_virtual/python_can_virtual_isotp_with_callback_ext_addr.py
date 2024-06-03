# In this example, we transmit a payload using a blocking send()
import isotp
import logging
import can
import time
import threading
from typing import Optional

bus1 = can.interface.Bus('test', interface='virtual')
bus2 = can.interface.Bus('test', interface='virtual')
bus3 = can.interface.Bus('test', interface='virtual')


def my_error_handler(error):
    # Called from a different thread, needs to be thread safe
    logging.warning('IsoTp error happened : %s - %s' % (error.__class__.__name__, str(error)))
    
    
def my_rxfn_1(timeout:float) -> Optional[isotp.CanMessage]:
    # All my_hardware_something and get_something() function are fictive of course.
    msg = bus1.recv(0.01) # Blocking read are encouraged for better timing.
    if msg is None:
        return None # Return None if no message available
    if msg.arbitration_id <= 0x7FF:
        msg.arbitration_id &= 0x7FF
        msg.is_extended_id = False
    print(f'my_rxfn_1 - {msg}')
    return isotp.CanMessage(arbitration_id=msg.arbitration_id, data=msg.data, dlc=msg.dlc, extended_id=msg.is_extended_id)


def my_txfn_1(isotp_msg:isotp.CanMessage):
    # all set_something functions and my_hardware_something are fictive.
    msg = can.Message()
    msg.arbitration_id = isotp_msg.arbitration_id
    msg.data = (isotp_msg.data)
    msg.dlc = (isotp_msg.dlc)
    msg.is_extended_id = (isotp_msg.is_extended_id)
    bus1.send(msg)
    print(f'my_txfn_1 - {msg}')
    
    
def my_rxfn_2(timeout:float) -> Optional[isotp.CanMessage]:
    # All my_hardware_something and get_something() function are fictive of course.
    msg = bus2.recv(0.01) # Blocking read are encouraged for better timing.
    if msg is None:
        return None # Return None if no message available
    if msg.arbitration_id <= 0x7FF:
        msg.arbitration_id &= 0x7FF
        msg.is_extended_id = False
    print(f'my_rxfn_2 - {msg}')
    return isotp.CanMessage(arbitration_id=msg.arbitration_id, data=msg.data, dlc=msg.dlc, extended_id=msg.is_extended_id)


def my_txfn_2(isotp_msg:isotp.CanMessage):
    # all set_something functions and my_hardware_something are fictive.
    msg = can.Message()
    msg.arbitration_id = isotp_msg.arbitration_id
    msg.data = (isotp_msg.data)
    msg.dlc = (isotp_msg.dlc)
    msg.is_extended_id = (isotp_msg.is_extended_id)
    msg.is_rx = False
    bus2.send(msg)
    print(f'my_txfn_2 - {msg}')

TX_1_TARGET_ADDR = 0x1
TX_1_SOURCE_ADDR = 0x2
RX_1_TARGET_ADDR = 0x3
RX_1_SOURCE_ADDR = 0x4

RX_2_TARGET_ADDR = 0xF1
RX_2_SOURCE_ADDR = 0x10
TX_2_TARGET_ADDR = 0x10
TX_2_SOURCE_ADDR = 0xF1

addr1 = isotp.AsymmetricAddress(
    tx_addr=isotp.Address(isotp.AddressingMode.NormalFixed_29bits, target_address=TX_1_TARGET_ADDR, source_address=TX_1_SOURCE_ADDR, tx_only=True),
    rx_addr=isotp.Address(isotp.AddressingMode.NormalFixed_29bits, target_address=RX_1_TARGET_ADDR, source_address=RX_1_SOURCE_ADDR, rx_only=True)   # txid is not required
)
params1 = {
    'blocking_send' : True,
    # 'tx_padding': 0xFF
}
tp_layer1 = isotp.TransportLayer(address=addr1, error_handler=my_error_handler, params=params1, rxfn=my_rxfn_1, txfn=my_txfn_1)


addr2 = isotp.AsymmetricAddress(
    tx_addr=isotp.Address(isotp.AddressingMode.NormalFixed_29bits, target_address=TX_2_TARGET_ADDR, source_address=TX_2_SOURCE_ADDR, tx_only=True),
    rx_addr=isotp.Address(isotp.AddressingMode.NormalFixed_29bits, target_address=RX_2_TARGET_ADDR, source_address=RX_2_SOURCE_ADDR, rx_only=True)   # txid is not required
)
params2 = {
    'blocking_send' : True,
    # 'tx_padding': 0xFF
}
tp_layer2 = isotp.TransportLayer(address=addr2, error_handler=my_error_handler, params=params2, rxfn=my_rxfn_2, txfn=my_txfn_2)


def bus1_task(stack: isotp.CanStack, stop_event):
    stack.start()
    while not stop_event.is_set():
        payload = stack.recv(timeout=1)
        if payload is not None:
            print(payload)
    stack.stop()
    print("Stopped bus1_task")


def bus2_task(stack: isotp.CanStack, stop_event):
    stack.start()
    while not stop_event.is_set():
        try:
            stack.send(b'123456789', send_timeout=0.1)    # Blocking send, raise on error
            # print("Payload transmission successfully completed.")     # Success is guaranteed because send() can raise
        except isotp.BlockingSendFailure:   # Happens for any kind of failure, including timeouts
            print("Send failed")
        time.sleep(1)
    stack.stop()
    print("Stopped bus2_task")


def bus3_task(bus: can.interface.Bus, stop_event):
    while not stop_event.is_set():
        msg = bus.recv(0.01)
        if (msg) is not None:
            # print(f'\t{msg}')
            pass
    bus3.shutdown()
    print("Stopped bus3_task")


def main():
    stop_event = threading.Event()
    
    t_bus1_task = threading.Thread(target=bus1_task, args=(tp_layer1, stop_event))
    t_bus1_task.start()
    
    t_bus2_task = threading.Thread(target=bus2_task, args=(tp_layer2, stop_event))
    t_bus2_task.start()
    
    t_bus3_task = threading.Thread(target=bus3_task, args=(bus3, stop_event))
    t_bus3_task.start()

    try:
        while True:
            time.sleep(0)  # yield
    except KeyboardInterrupt:
        pass  # exit normally

    stop_event.set()
    time.sleep(0.5)

    print("Stopped script")


if __name__ == "__main__":
    main()