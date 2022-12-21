from pwn import *

"""
w = connect('tejo.tecnico.ulisboa.pt', 58011, typ="udp")

w.send(b"SNG 099312\n")

print(w.recvline())

w.send(b"REV 099312\n")
dat = w.recvline()
dat = dat.split()
dat = dat[-1]

print(dat)

w.close()
"""

w = connect('tejo.tecnico.ulisboa.pt', 58011, typ="tcp")
w.send(b"GSB\n")
x = w.recv(200)
with open("ex.txt", "wb") as f:
    while True:
        x = w.recv(200)
        f.write(x)
w.close()

"""
w = connect('tejo.tecnico.ulisboa.pt', 58011, typ="udp")
w.send(b"QUT 099312\n")
print(w.recvline())
w.close()
"""

