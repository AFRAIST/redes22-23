from pwn import *

s = connect('127.0.0.1', 6969, typ="udp")

a = s.send(b"SNG 099312\n\x00\n")
a = s.recvline()

assert(a == b"RSG ERR\n")

a = s.send(b"SNG 099312\n\n\n\n")
a = s.recvline()

assert(a == b"RSG ERR\n")

a = s.send(b"SNG 099312\n\x00wdh")
a = s.recvline()

assert(a == b"ERR\n")

a = s.send(b"SNG 099312\n\x00")
a = s.recvline()

assert(a == b"ERR\n")

a = s.send(b"SNG 099312\n\n\n\x00fhuef\n")
a = s.recvline()

assert(a == b"RSG ERR\n")

