# In this example, we transmit a payload using a blocking send()
import isotp
import logging
import can
import time
import threading

bus1 = can.interface.Bus('test', interface='virtual')
bus2 = can.interface.Bus('test', interface='virtual')
bus3 = can.interface.Bus('test', interface='virtual')


def my_error_handler(error):
    # Called from a different thread, needs to be thread safe
    logging.warning('IsoTp error happened : %s - %s' % (error.__class__.__name__, str(error)))


addr1 = isotp.Address(isotp.AddressingMode.Normal_11bits, rxid=0x123, txid=0x234)
params1 = {
    'blocking_send' : True,
    # 'tx_padding': 0xFF
}
stack1 = isotp.CanStack(bus1, address=addr1, error_handler=my_error_handler, params=params1)

addr2 = isotp.Address(isotp.AddressingMode.Normal_11bits, rxid=0x234, txid=0x123)
params2 = {
    'blocking_send' : True,
    # 'tx_padding': 0xFF
}
stack2 = isotp.CanStack(bus2, address=addr2, error_handler=my_error_handler, params=params2)


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
            stack.send(b'12345678901234567890123', send_timeout=2)    # Blocking send, raise on error
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
            print(f'\t{msg}')
    bus3.shutdown()
    print("Stopped bus3_task")


def main():
    stop_event = threading.Event()
    
    t_bus1_task = threading.Thread(target=bus1_task, args=(stack1, stop_event))
    t_bus1_task.start()
    
    t_bus2_task = threading.Thread(target=bus2_task, args=(stack2, stop_event))
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