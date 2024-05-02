
import time
import threading
import cantools
import can


bus = can.Bus(interface="serial", channel="COM4")
db = cantools.database.load_file('C:/D/Bico/my_tool/Bico_CAN/python_code/dbc_test/dbc_file.dbc')


def send_cyclic(stop_event):
    count = 0
    while not stop_event.is_set():
        AT_Torque_RN1 = db.get_message_by_name("AT_Torque_RN1")
        encode_dict = {}
        count += 1
        # for signal in AT_Torque_RN1.signals:
        #     encode_dict[signal.name] = count
        # tx_data = AT_Torque_RN1.encode(encode_dict)              
        # print(encode_dict)
        
        tx_data = AT_Torque_RN1.encode({'Com_tqFastReqRaw': count%2,
                                        'Com_tqLimSlowReqRaw': count%2,
                                        'Com_stTrsmRngEngdTarRaw': count%2, 
                                        'Com_stTrsmRngEngdCurrRaw': count%2, 
                                        'Com_stTqCtrlTypRaw': count%2,
                                        'Com_flgManModRaw': count%2, 
                                        'Com_clkATRN1Raw': count%2, 
                                        'Com_stTqCnvrRaw': count%2, 
                                        'Com_nATMinIdleReqRaw': count%2, 
                                        'Com_tqATLosRaw': count%2
                                        })
                        
        tx_msg = can.Message(arbitration_id=AT_Torque_RN1.frame_id, data=tx_data)
        print(tx_msg)
        bus.send(tx_msg)
        # time.sleep(AT_Torque_RN1.cycle_time/1000)
        time.sleep(1)


def receive(stop_event):
    while not stop_event.is_set():
        # COM port data:
        # AA 00 00 00 00 08 B6 01 00 00 00 00 00 00 00 00 20 00 BB
        rx_msg = bus.recv(1)
        if rx_msg is not None:
            print(f"rx: {rx_msg}")
            frame = db.decode_message(rx_msg.arbitration_id, rx_msg.data)
            print(frame)
        # result:
        # {'Com_flgCBAReqRaw': 'no request', 'Com_flgFCAActvRaw': 'FCA is not active', 'Com_flgCrsInhbITSRaw': 'Cruise Inhibit Request is OFF', 'Com_ratCBAAccrPedlRaw': 0.0, 'Com_clkIDMRaw': 2}

def main():
    if bus is not None:
        # Thread for sending and receiving messages
        stop_event = threading.Event()

        t_send_cyclic = threading.Thread(target=send_cyclic, args=(stop_event,))
        t_send_cyclic.start()

        t_receive = threading.Thread(target=receive, args=(stop_event,))
        t_receive.start()

        try:
            while True:
                time.sleep(0.05)  # yield
        except KeyboardInterrupt:
            pass  # exit normally

        stop_event.set()
        time.sleep(0.5)

    print("Stopped script")
    
if __name__ == "__main__":
    main()