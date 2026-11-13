#!/usr/bin/env python3
"""Certificado autofirmado DEMO de servidor OTA, ligado a IP local con SAN."""
import argparse
import ipaddress
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description="Genera cert/key HTTPS demo para IP del PC")
    parser.add_argument("ip", help="IPv4/IPv6 local del computador que sirve firmware")
    args = parser.parse_args()
    ip = str(ipaddress.ip_address(args.ip))
    cert = ROOT / "main" / "certs" / "server_cert.pem"
    key = ROOT / "server" / "private" / "server_key.pem"
    cert.parent.mkdir(parents=True, exist_ok=True)
    key.parent.mkdir(parents=True, exist_ok=True)
    cmd = ["openssl", "req", "-x509", "-newkey", "rsa:2048", "-sha256",
           "-nodes", "-days", "365", "-keyout", str(key), "-out", str(cert),
           "-subj", f"/CN={ip}", "-addext", f"subjectAltName=IP:{ip}"]
    subprocess.run(cmd, check=True)
    print("Certificado publico:", cert)
    print("Clave privada (NO SUBIR A GITHUB):", key)
    print("Recompilar firmware v1 y v2 para que integren ESTE certificado.")
    print(f"Configurar TV_OTA_URL=https://{ip}:8443/firmware_v2.bin")


if __name__ == "__main__":
    main()
