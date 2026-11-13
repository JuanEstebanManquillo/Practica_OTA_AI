# Estado y alcance de la validación

- Se tomó el proyecto OTA+IA original como base. La parte Modbus/RS485 se reemplazó por SHT31 I²C.
- Puntos físicos propuestos para la PCB existente: J_CAM pin 2 SDA=GPIO4, pin 1 SCL=GPIO5, pin 15 3V3, pin 16 GND. Validar orientación real del header antes de conectar.
- Se implementó comando `0x2400`, conversión T/HR y CRC8 polinomio `0x31`.
- El algoritmo de IA usa ventanas independientes para temperatura, humedad y latencia de I²C, junto con contador de fallos.
- OTA HTTPS, Wi-Fi, particiones con rollback y LED externo opcional se conservan.
- Se incluyen pruebas de host para algoritmo y datos SHT31, y procedimientos para pruebas reales.
- **Sin validación física de ESP32-S3/SHT31** y **sin compilación ESP-IDF en este entorno**. Verificar conexión, compilación, OTA real y logs antes de entregar como ensayo ejecutado.
