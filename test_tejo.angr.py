from pwn import *

w = connect('tejo.tecnico.ulisboa.pt', 58011, typ="udp")

# Will not work because UDP everything goes in one packet.
#w.send(b"SNG 099312\naaaa")
#w.send(b"QUT 099312\n")
#print(w.recvline())
w.send(b"SNG  099312\n")

print(w.recvline())

#w.send(b"PLG 099312 C 1\n") (N)OK
#w.send(b"PLG 099312 C 7\n") INV
#w.send(b"PLG 099312 C 3\n")
#print(w.recvline())

#w.send(b"PLG s99312 q 69\n")
#print(w.recvline())

w.send(b"REV 099312\n")
dat = w.recvline()
print(dat)
dat = dat.split()
dat = dat[-1]

# You can also send in uppercase.
#dat = dat.decode().upper().encode('ascii')

print(b"PWG 099312 "+dat+b" 1\n")
w.send(b"PWG 099312 "+dat+b" 1\n")
print(w.recvline())
w.send(b"QUT 099312\n")

print(w.recvline())

w = connect('tejo.tecnico.ulisboa.pt', 58011, typ="tcp")
w.send(b"GSB\n")
print(w.recv(2000))
w.close()

