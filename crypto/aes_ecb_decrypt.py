from Crypto.Cipher import AES
from Crypto.Util.Padding import pad

key = "9c16e575348a710c5079d0c05314e500"

flag = "83e46ca1c188f1842bda50d32aa91805757fc9449d8b93b555563e7d64968bdd3eff8714da23789d3a91434796a5a0b2bf5408064bc23e6b3637764c3c8f68e2"

flag2 = "83e46ca1c188f1842bda50d32aa91805"
cipher = AES.new(key=bytes.fromhex(key), mode=AES.MODE_ECB)
ciphertext = cipher.decrypt(pad(bytes.fromhex(flag), cipher.block_size))

print(ciphertext)
