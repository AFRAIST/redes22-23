import sys
import os
from pwn import *

def get_word():
    w = connect('tejo.tecnico.ulisboa.pt', 58011, typ="udp")
    dat = []
    for i in range(30):
        w.send(b"SNG 099312\n")
        w.recvline()

        w.send(b"REV 099312\n")
        d = w.recvline()
        d = d.split()
        d = d[-1]
        print(d)
        w.send(b"QUT 099312\n")
        w.recvline()
        dat.append(d)

    w.close()
    return dat

def main():
    r = set()
    while True:
        dat = get_word()
        
        for word in dat:
            if word != b'ERR':
                r.add(word)
        print(len(r))
        if os.path.isfile("/tmp/stop.txt"):
            import ipdb; ipdb.set_trace()

    return 0

if __name__ == "__main__":
    sys.exit(main())

