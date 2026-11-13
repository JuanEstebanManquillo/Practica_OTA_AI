# Pruebas y evidencias · TerraVision SHT31

## Requisito 1: OTA HTTPS

1. Capturar monitor USB mostrando `FIRMWARE VERSION 1.0.0`.
2. Ejecutar servidor HTTPS en PC y registrar petición/descarga de `firmware_v2.bin`.
3. Capturar monitor USB tras reinicio mostrando `FIRMWARE VERSION 2.0.0` y `ACTUALIZACION OTA CONFIRMADA`.
4. Opcional: LED externo conectado y habilitado, 1 destello v1 / 2 destellos v2. No está implementado en la PCB.
5. Conservar logs de fallos si servidor no está disponible, mostrando que permanece v1.

## Requisito 2: IA en ESP32-S3

1. Mostrar sensor SHT31 I²C detectado en `0x44` (o `0x45` si configurado).
2. Capturar lecturas reales de temperatura °C y humedad relativa %, con ambos CRC válidos.
3. Mantener por lo menos 8 muestras de línea base; mostrar `IA: ANOMALIA` al cambiar condiciones ambientales de forma controlada.
4. Desconectar SDA o el módulo temporalmente, evidenciar alerta a 2 fallos y crítico a 5; restaurar el sensor y verificar recuperación.
5. Guardar pruebas locales (`bash tests/run_tests.sh`) para algoritmo y CRC; distinguirlas explícitamente de pruebas en placa.

**Seguridad:** no publicar claves Wi-Fi ni claves privadas TLS. La IA es estadística adaptativa, no una red neuronal. No presentar modo DEMO como medición real.
