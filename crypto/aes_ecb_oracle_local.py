from pwn import *
import string

p = process(["/challenge/run"])

def get_result_hex():
    p.recvuntil(b"Result: ")
    return p.recvline().strip().decode()

def enc_chosen(ch: bytes) -> bytes:
    p.sendlineafter(b"Choice? ", b"1")
    p.sendlineafter(b"Data? ", ch)
    hx = get_result_hex()
    return bytes.fromhex(hx)

def enc_flag(index: int, length: int) -> bytes:
    p.sendlineafter(b"Choice? ", b"2")
    p.sendlineafter(b"Index? ", str(index).encode())
    p.sendlineafter(b"Length? ", str(length).encode())
    hx = get_result_hex()
    return bytes.fromhex(hx)

alphabet = (string.ascii_letters + string.digits + "{}_-.").encode()

codebook = {}
for c in alphabet:
    ct = enc_chosen(bytes([c]))
    codebook[ct] = c

key = bytearray()

for i in range(60):
    enc = enc_flag(i, 1)
    dec = codebook.get(enc)
    key.append(dec)
    print(key)


print(key)

