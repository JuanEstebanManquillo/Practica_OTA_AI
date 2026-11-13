#!/usr/bin/env python3
"""Servidor HTTPS local de prueba OTA (firmware v2); no es un servidor de produccion."""
import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import ssl

ROOT = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser(description="TerraVision HTTPS OTA server")
    parser.add_argument("--port", type=int, default=8443)
    args = parser.parse_args()
    cert = ROOT.parent / "main" / "certs" / "server_cert.pem"
    key = ROOT / "private" / "server_key.pem"
    fw = ROOT / "firmware" / "firmware_v2.bin"
    if not cert.exists() or not key.exists():
        raise SystemExit("Falta certificado/clave. Ejecute: python scripts/generate_cert.py IP_DEL_PC")
    if not fw.exists():
        raise SystemExit("Falta server/firmware/firmware_v2.bin. Compile primero firmware v2.")
    handler = partial(SimpleHTTPRequestHandler, directory=str(ROOT / "firmware"))
    httpd = ThreadingHTTPServer(("0.0.0.0", args.port), handler)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.minimum_version = ssl.TLSVersion.TLSv1_2
    context.load_cert_chain(certfile=str(cert), keyfile=str(key))
    httpd.socket = context.wrap_socket(httpd.socket, server_side=True)
    print(f"Servidor OTA HTTPS escuchando puerto {args.port} (red local). Ctrl+C para detener")
    print("URL de firmware: https://IP_DEL_PC:%d/firmware_v2.bin" % args.port)
    httpd.serve_forever()


if __name__ == "__main__":
    main()
