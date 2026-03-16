from pwn import *

p = process("/challenge/run")

for _ in range(10):
    p.readuntil(b"key: ")
    key = int(p.readline().strip(), 16)

    p.readuntil(b"secret: ")
    enc = int(p.readline().strip(), 16)

    plain = key ^ enc

    p.sendline(hex(plain).encode())

print(p.readall().decode())
