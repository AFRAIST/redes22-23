from pwn import *

w = connect('tejo.tecnico.ulisboa.pt', 58011, typ="udp")

w.send(b"REV 099312\n")
dat = w.recvline()
print(dat)
dat = dat.split()
dat = dat[-1]

# You can also send in uppercase.
dat = dat.decode().upper().encode('ascii')

print(dat)

w.close()

