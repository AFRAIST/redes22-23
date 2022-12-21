from pwn import *

w = connect('localhost', 6969, typ="udp")

w.send(b"SNG 099312\n")

print(w.recvline())


w.send(b"REV 099312\n")
dat = w.recvline()
print(dat)

w.send(b"QUT 099312\n")

print(w.recvline())

