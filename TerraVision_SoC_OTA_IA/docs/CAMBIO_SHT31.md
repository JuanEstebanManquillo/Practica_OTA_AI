# Registro de cambio: sonda de suelo → SHT31 (12/11/2026)

El proyecto utiliza temporalmente el **sensor SHT31** como alternativa a la sonda multiparámetro de suelo, aún no disponible. La medida de **humedad relativa ambiental** no debe describirse como humedad del suelo.

## Cambios en software

- `soil_modbus.c/.h` retirados: la interfaz RS485 no se utiliza en esta demostración.
- `sht31_sensor.c/.h` nuevos: adquisición por I²C desde J_CAM (GPIO4/GPIO5).
- `sht31_codec.c/.h` nuevos: verificación de trama CRC8 y conversión física T/HR.
- `ai_engine.c/.h` adaptados para ventanas independientes de temperatura, humedad y latencia I²C.
- `main.c` usa por defecto mediciones reales del SHT31; si no hay sensor físico se puede habilitar demo sintética de forma explícita.
- `ota_manager.c`, `wifi_manager.c`, `status_led.c`, servidor HTTPS y particiones OTA se conservan.

## Lo que NO se modificó

El PCB original no incluye SHT31 ni se ha rediseñado. El encabezado J_CAM se usa como conexión provisional. Los LEDs OTA son externos opcionales, no componentes ya integrados al circuito.

## Validación pendiente

Verificar físicamente los pines del conector, compilar en la instalación ESP-IDF del equipo, registrar lecturas SHT31 y demostrar por monitor USB el cambio v1→v2 vía HTTPS.
