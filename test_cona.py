from pwn import *

s = connect("127.0.0.1", 6969, typ="udp")

def wa(x):
    global s
    s.send(x)

wa(b"SNG 000001\n")
print(s.recvline())
wa(b"PWG 000001 cona 1\n")
print(s.recvline())
wa(b"PWG 000001 pila 1\n")
print(s.recvline())
wa(b"PWG 000001 coninha 2\n")
print(s.recvline())
wa(b"PWG 000001 coninha 2\n")
print(s.recvline())

