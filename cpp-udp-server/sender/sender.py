#!/usr/bin/env python3

import random
import socket
from functools import partial

UDP_IP = "127.0.0.1"
UDP_PORT = 3000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP

with open("./books.xml", "rb") as f:
    document_id = random.randint(1, 256)
    chunk_id = 0

    document_id_repr = document_id.to_bytes(4, byteorder="little")

    for chunk in iter(partial(f.read, 128), b''):
        chunk_id += 1
        chunk_id_repr = chunk_id.to_bytes(4, byteorder="little")

        divider = 0
        divider_repr = divider.to_bytes(1)

        packet = document_id_repr + chunk_id_repr + chunk + divider_repr
        sock.sendto(packet, (UDP_IP, UDP_PORT))
