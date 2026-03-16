from pwn import *

p = process("/challenge/run")

for _ in range(1,10):
    p.recvuntil(b"- Encrypted Character: ")
    enc_ch = p.recvline().strip()
    enc = enc_ch[0]

    p.recvuntil(b"- XOR Key: ")
    key_line = p.recvline().strip()
    key = int(key_line, 16)

    dec = bytes([key^enc])

    p.recvuntil(b"- Decrypted Character? ")
    p.sendline(dec)

print(p.recvall())

