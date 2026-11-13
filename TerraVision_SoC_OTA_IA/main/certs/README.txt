Certificado PEM PUBLICO de ejemplo para que CMake encuentre el archivo embebido.
Se genero con direccion ficticia 192.168.1.100 y NO tiene clave privada incluida.
OBLIGATORIO antes de probar OTA: python scripts/generate_cert.py IP_REAL_PC
Este comando sustituye el PEM por el certificado real y crea server/private/server_key.pem.
NO subir la clave privada a GitHub; no comprometa credenciales.
