from flask import Request
from typing import Tuple
import ipaddress

def check_ip_version(ip_s: str) -> str:
    ip = ipaddress.ip_address(ip_s)
    if isinstance(ip, ipaddress.IPv6Address):
        return "IPv6"
    elif isinstance(ip, ipaddress.IPv4Address):
        return "IPv4"
    return ""



def get_ip_addr(req: Request) -> str:

    if req.headers.get("X-Forwarded-For"):
        addresses =  req.headers.get("X-Forwarded-For").split()
        for addr in addresses:
            if check_ip_version(addr) == "IPv4": return addr

    elif req.headers.get("X-Real-Ip") and check_ip_version(req.headers.get("X-Real-Ip")) == "IPv4":
        return req.headers.get("X-Real-Ip")

    elif check_ip_version(req.remote_addr) == "IPv4":
        return req.remote_addr

    return ""