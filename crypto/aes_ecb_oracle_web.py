import re
import requests
import string

URL = "http://challenge.localhost/"

def get_ct_hex(query: str) -> str:
    r = requests.get(URL, params={"query": query})
    r.raise_for_status()
    pres = re.findall(r"<pre>(.*?)</pre>", r.text, flags=re.S)
    return pres[-1].strip()

def enc_chosen(ch: str) -> bytes:
    return bytes.fromhex(get_ct_hex("'" + ch + "'"))

def enc_flag(pos: int) -> bytes:
    return bytes.fromhex(get_ct_hex(f"substr(flag,{pos},1)"))

alphabet = string.ascii_letters + string.digits + "{}_-."

codebook = {}
for ch in alphabet:
    ct = enc_chosen(ch)
    codebook[ct] = ch

key = ""

for i in range(1,70):
    enc = enc_flag(i)
    dec = codebook.get(enc)
    if dec is None:
        break
    else:
        key += dec
    print(key)


print(key)

