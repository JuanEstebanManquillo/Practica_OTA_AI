# TerraVision IA — Entregable SoC: OTA + IA con SHT31

Firmware para **ESP32-S3-WROOM-1-N16R8** (flash de 16 MB / PSRAM de 8 MB), realizado con **ESP-IDF 5.3.x**. Se utiliza un **sensor SHT31 de temperatura y humedad ambiental por I²C**. Se conservan OTA por HTTPS, rollback, servidor HTTPS, pruebas unitarias y documentación Doxygen.

**Alcance:** se atienden únicamente los requisitos **1) OTA** y **2) IA para gestión de dispositivos**. No contiene programación para cámara, e-paper, panel solar, GNSS, ESP-NOW o controles por gestos.

## 1. Qué está implementado y qué aún falta probar

| Elemento | Estado en este paquete |
|---|---|
| Controlador SHT31 | Implementado: I²C 100 kHz, comando `0x2400`, lectura de 6 bytes, CRC8 y conversión T/HR |
| IA local | Implementado: tres ventanas adaptativas (temperatura, HR, tiempo I²C) y fallos consecutivos |
| OTA HTTPS | Implementado: 2 particiones, TLS contra certificado embebido, reinicio y confirmación/rollback |
| LED de comprobación | Opcional: 1 pulso con v1 / 2 con v2, solo al conectar físicamente LED externo |
| Pruebas PC | Se incluyen pruebas de IA y CRC/conversión de la trama SHT31 |
| Prueba en hardware | **Pendiente**: hay que compilar en ESP-IDF, cargar y ensayar con ESP32-S3 + SHT31 |

**La IA usa un método de detección de anomalías estadístico basado en puntuación Z, no una red neuronal.** 

## 2. Cableado SHT31 a la PCB TerraVision

| Terminal del SHT31 | Terminal del PCB `J_CAM` | ESP32-S3 |
|---|---|---|
| **VCC / VIN** | **pin 15** | 3V3 |
| **GND** | **pin 16** | GND |
| **SDA** | **pin 2** | GPIO4 (`CAM_SDA`) |
| **SCL** | **pin 1** | GPIO5 (`CAM_SCL`) |

- El sensor SHT31 suele usar dirección `0x44`; si ADDR está conectado a 3V3 puede usar `0x45`. Editar `TV_SHT31_ADDRESS` en `main/tv_config.h`.
- **SDA/SCL requieren pull-ups a 3V3**. Muchas placas breakout SHT31 ya los incluyen; compruebe su modelo. Para módulo sin ellos, agregar por ejemplo 4,7 kΩ de SDA a 3V3 y de SCL a 3V3 (longitudes cortas). El software habilita pull-ups internos, insuficientes como única garantía para un montaje largo.
- El RTC DS3231 está en otros pines (GPIO1/GPIO2), por lo que permanece independiente.


## 3. Qué hace el programa

Cada 2 segundos solicita una medición de alta repetibilidad sin clock stretching (`0x2400`), espera 20 ms y lee 6 bytes: `T_MSB, T_LSB, CRC_T, RH_MSB, RH_LSB, CRC_RH`. Verifica ambos CRC y calcula:

- Temperatura: `T = -45 + 175 × raw_t / 65535` (°C).
- Humedad relativa: `HR = 100 × raw_h / 65535` (%).

La IA mantiene 16 muestras por variable y comienza a detectar anomalías una vez que tiene 8 muestras base. Un cambio brusco respecto a esa base genera `IA: ANOMALIA`. También mide la duración de cada consulta I²C y detecta errores de comunicación: **2 fallos seguidos → ALERTA; 5 → CRÍTICO**. Se restablece al volver una lectura válida. Se muestran las lecturas y el estado en la consola USB.

**Importante:** un cambio anómalo según la estadística no prueba que exista un riesgo agronómico; es una alerta del funcionamiento/entorno, y debe interpretarse dentro del contexto.

## 4. Preparación del entorno

Necesita VS Code + extensión Espressif IDF, ESP-IDF **5.3.x**, Python 3, Git y opcionalmente Doxygen y GCC para las pruebas PC. El código utiliza `esp_driver_i2c` (API master nueva de ESP-IDF 5.x), no la librería Arduino `Wire`.

1. Descomprima esta carpeta y ábrala con VS Code.
2. Abra **ESP-IDF: Open ESP-IDF Terminal**.
3. Si desea ensayar SHT31 real, deje en `main/tv_config.h`: `TV_DEMO_MODE 0` (valor por defecto).
4. Si aún no tiene sensor conectado y solo necesita comprobar la IA con datos simulados, cambie temporalmente a `TV_DEMO_MODE 1`. En pantalla se etiquetan como **SINTÉTICOS**.
5. Compile desde la raíz del proyecto:

```powershell
idf.py set-target esp32s3
idf.py build
```

6. Conecte la placa por USB y grabe, sustituyendo `COM5` por el COM visible en su PC:

```powershell
idf.py -p COM5 flash monitor
```

Salida esperada con sensor real conectado:

```text
I (...) TERRAVISION: TERRAVISION - SHT31 - FIRMWARE VERSION 1.0.0
I (...) SHT31: Bus I2C SDA=GPIO4 SCL=GPIO5, SHT31 0x44
I (...) TERRAVISION: Medicion 0: T=... C HR=... % I2C=... ms salud=NORMAL
```

Los números `...` son marcadores de ejemplo. No son lecturas obtenidas de un sensor real.


## 5. Demostrar OTA 1.0.0 → 2.0.0 (sin programación USB de v2)

### A. Configure HTTPS y Wi-Fi **antes de compilar v1**

1. Busque IPv4 del PC en la red local: `ipconfig` (por ejemplo `192.168.1.50`, no usar la IP del ejemplo sin verificarla).
2. Instale OpenSSL si falta. Genere certificado para la IP real:

```powershell
python scripts/generate_cert.py 192.168.1.50
```

3. Copie `main/tv_secrets.example.h` a **`main/tv_secrets.h`** y escriba SSID, contraseña y URL del servidor HTTPS. La URL debe ser `https://IP_REAL:8443/firmware_v2.bin`. El archivo de credenciales y la clave privada del servidor se excluyen de GitHub mediante `.gitignore`.
4. Mantenga `TV_STATUS_LED_ENABLED 0` salvo que haya conectado físicamente un LED externo al GPIO48.

### B. Compile y grabe la versión 1.0.0

```powershell
python scripts/set_version.py 1.0.0
idf.py set-target esp32s3
idf.py build
idf.py -p COM5 flash monitor
```

Busque el texto `FIRMWARE VERSION 1.0.0`.

### C. Prepare la versión 2.0.0 **sin usar flash USB**

Cierre el monitor con `Ctrl+]` y ejecute:

```powershell
python scripts/set_version.py 2.0.0
idf.py build
Copy-Item build/TerraVision_SoC.bin server/firmware/firmware_v2.bin
```

La ruta `build/TerraVision_SoC.bin` corresponde al nombre del proyecto de CMake. Si cambió el nombre, compruebe el `.bin` de aplicación en `build/` (no bootloader).

### D. Publique v2 y observe el cambio en el dispositivo

```powershell
python server/https_server.py --port 8443
```

El PC y ESP32 deben compartir red Wi-Fi y el firewall permitir TCP 8443. La v1 intentará descargar la v2. La evidencia esperada en consola será:

```text
TERRAVISION - SHT31 - FIRMWARE VERSION 1.0.0
OTA: Iniciando OTA HTTPS...
OTA: Firmware grabado; reiniciando para verificar nueva version
TERRAVISION - SHT31 - FIRMWARE VERSION 2.0.0
OTA: ACTUALIZACION OTA CONFIRMADA: firmware 2.0.0
```


### E. LED de comprobación (opcional)

Conecte un LED convencional: `GPIO48 → resistencia de 330 Ω → ánodo del LED; cátodo → GND`. Active `#define TV_STATUS_LED_ENABLED 1` y recompílelo en v1 y v2. Parpadeo previsto: **v1 = un destello; v2 = dos destellos**.

## 6. Comprobaciones locales de IA + CRC

En PC con GCC disponible:

```bash
bash tests/run_tests.sh
```

Las pruebas verifican: ventana base, anomalía térmica, anomalía de humedad, recuperación, 5 fallos consecutivos, CRC8 y conversión a °C y %HR.

Para una demostración rápida de la IA con el sensor conectado, deje completar primero la línea base (mínimo 8 mediciones) y acerque **su respiración** al módulo para cambiar humedad, o acerque ligeramente una mano cálida. Evite agua directa o una fuente intensa de calor. Si no aparece una alerta, pruebe un cambio gradual mayor, sin modificar el código para inventar resultados.

## 7. Seguridad y límites

- La actualización se descarga por HTTPS con validación del certificado autofirmado asociado a la IP local en el campo SAN. El certificado del proyecto debe regenerarse para el servidor del laboratorio.
- La clave privada no se publica. Para producción se requiere gestión profesional de certificados y credenciales.
- OTA utiliza dos particiones y rollback. La **verificación de firma del firmware requiere Secure Boot**; no afirmar que está implementada en este paquete.
- El autotest de OTA verifica que el firmware y el controlador I²C se inicialicen, pero **no exige presencia de SHT31**; así, una desconexión temporal no revierte necesariamente la versión instalada.
- El servidor HTTPS del paquete es una utilidad académica local, no un servicio preparado para exposición a Internet.

## 8. Documentación y fuentes

- `docs/PRUEBAS_Y_EVIDENCIAS.md`: qué registrar para los dos requisitos.
- `docs/VALIDACION.md`: qué está implementado y qué sigue pendiente en placa.
- `Doxyfile`: ejecutar `doxygen Doxyfile` para generar documentación HTML.
- Referencias del sensor: [Sensirion SHT3x-DIS (datasheet)](https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf) y [Sensirion SHT3x driver](https://github.com/Sensirion/embedded-sht/blob/master/sht3x/sht3x.c).
- Referencia ESP-IDF I²C: https://docs.espressif.com/projects/esp-idf/en/v5.3.3/esp32s3/api-reference/peripherals/i2c.html
- Referencia ESP-IDF OTA: https://docs.espressif.com/projects/esp-idf/en/release-v5.3/esp32s3/api-reference/system/esp_https_ota.html
