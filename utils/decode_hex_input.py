import pwn

p = pwn.process("/challenge/runme")

myHexa = '1101011010011101111100001101011110001111111010011010100110001011'

data = bytes.fromhex(myHexa)

p.readuntil(b':')
p.write(myHexa)
p.shutdown('send')
out = p.readall()
print(out.decode('utf-8', errors='replace'))
