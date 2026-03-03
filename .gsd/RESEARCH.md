# RESEARCH.md — Hardware & Technology Research

## M5Stack BugC2
- **Motor Driver**: L9110S (4-kanałowy) sterowany przez STM32F030F4
- **I2C Address**: `0x38` (stały)
- **Funkcje I2C**: Sterowanie prędkością/kierunkiem 4 silników DC, odczyt napięcia baterii, sterowanie LED WS2812C
- **IR Receiver**: SL0038GD — dekodowanie pilota IR
- **Bateria**: 700mAh 16340, ładowanie USB-C
- **Wymiary**: 55x40x25mm, 74.5g
- **Arduino Library**: `M5Hat-BugC` (GitHub: m5stack/M5Hat-BugC)

### Komendy I2C BugC2
| Rejestr | Funkcja |
|---------|---------|
| 0x00-0x03 | Prędkość silników 0-3 (-127 do 127) |
| 0x10 | Odczyt napięcia baterii |
| 0x20-0x27 | LED RGB (R,G,B dla LED0 i LED1) |

## M5StickC Plus 2
- **MCU**: ESP32-PICO-V3-02 (Dual-Core 240MHz)
- **Pamięć**: 8MB Flash, 2MB PSRAM
- **Ekran**: 1.14" TFT 135x240px (ST7789V2)
- **Grove Port**: HY2.0-4P — GPIO21 (SDA), GPIO22 (SCL)
- **Sensory**: MPU6886 (6-axis IMU), SPM1423 (mikrofon)
- **Bateria**: 200mAh LiPo
- **Przyciski**: BtnA (GPIO37), BtnB (GPIO39), Power
- **IR**: Nadajnik IR
- **Buzzer**: Pasywny

## ToF4M (VL53L1X)
- **Typ**: Time-of-Flight laser distance sensor
- **I2C Address**: `0x29` (domyślny)
- **Zakres pomiarowy**: 40mm - 4000mm (4m)
- **Dokładność**: ±20mm (typowo)
- **Kąt FOV**: 27° (field of view)
- **Częstotliwość**: Do 50Hz
- **Arduino Library**: `M5Unit-ToF4M` + `VL53L1X`
- **Tryby**: Short range (lepsza dokładność), Long range (większy zasięg)

## AtomS3R-CAM AI
- **MCU**: ESP32-S3-PICO-1-N8R8 (Dual-Core LX7 240MHz)
- **Pamięć**: 8MB Flash, 8MB PSRAM
- **Kamera**: GC0308 (0.3MP, F2.6, 57.6° FOV)
- **Sensory**: BMI270 (6-axis IMU), BMM150 (3-axis magnetometer)
- **Audio**: ES8311 codec, mikrofon MEMS, głośnik 8Ω 1W
- **Connectivity**: 2.4GHz WiFi, USB OTG, USB Serial/JTAG
- **IR**: Nadajnik IR (180°, do 12.46m)
- **GPIO dostępne**: G5, G6, G7, G8, G38, G39
- **Port**: HY2.0-4P (Grove compatible)
- **AI**: Brak wbudowanego modelu — wymaga wgrania TensorFlow Lite Micro

### Architektura dwu-procesorowa
AtomS3R-CAM ma własny ESP32-S3 z 8MB PSRAM — to osobny komputer, który:
1. Samodzielnie uruchamia modele TFLite (person detection, object detection)
2. Przetwarza obraz lokalnie i wysyła wyniki (nie surowy obraz) do M5StickC Plus 2
3. Komunikuje się z M5StickC Plus 2 przez **Grove Hub (I2C slave)** — wszystko kablem

### Architektura komunikacji (Grove Hub)
```
M5StickC Plus 2 (ESP32) ── Grove Hub ──┬── BugC2 (I2C 0x38)
                                        ├── ToF4M (I2C 0x29)
                                        └── AtomS3R-CAM (I2C slave 0x40*)
                                             ├── Kamera GC0308
                                             ├── TFLite model
                                             └── Lokalne przetwarzanie obrazu

* Proponowany adres I2C slave — do potwierdzenia
```

**Protokół I2C slave na AtomS3R-CAM:**
- M5StickC Plus 2 (master) odpytuje AtomS3R-CAM (slave) o wyniki AI
- AtomS3R-CAM ciągle przetwarza obraz i buforuje wyniki
- Master odczytuje: typ przeszkody, kierunek, odległość szacunkowa, pewność
- Maksymalny transfer I2C: ~32 bajty na odczyt (wystarczające dla wyników AI)

---

## Wgrywanie modelu TFLite na AtomS3R-CAM — krok po kroku

### Przygotowanie środowiska
1. **Zainstaluj Arduino IDE 2.x** — https://www.arduino.cc/en/software
2. **Dodaj board ESP32-S3** w Arduino Board Manager:
   - File → Preferences → Additional Board URLs: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board Manager → szukaj "esp32" → zainstaluj "esp32 by Espressif"
3. **Wybierz board**: Tools → Board → ESP32S3 Dev Module
4. **Skonfiguruj**:
   - Flash Size: 8MB
   - PSRAM: OPI PSRAM
   - Partition Scheme: Huge APP (3MB No OTA)
   - USB Mode: Hardware CDC and JTAG

### Przygotowanie modelu AI
5. **Pobierz pre-trenowany model** — zalecany punkt startowy:
   - **Person Detection** (z TFLite Micro examples): ~300KB, wykrywa obecność osoby
   - **Object Detection** (MobileNet SSD v1): ~2MB, wykrywa 90 kategorii obiektów
   - Źródło: https://github.com/tensorflow/tflite-micro/tree/main/tensorflow/lite/micro/examples
6. **Konwertuj model do formatu C array**:
   ```bash
   xxd -i model.tflite > model_data.h
   ```
   To tworzy plik .h z modelem jako tablica `unsigned char[]`
7. **Alternatywnie** — użyj `tflite_model_generator.py`:
   ```python
   # Konwersja .tflite → .h do embedowania w firmware
   with open("model.tflite", "rb") as f:
       data = f.read()
   with open("model_data.h", "w") as f:
       f.write(f"const unsigned char model_data[] = {{\n")
       for i, byte in enumerate(data):
           f.write(f"0x{byte:02x}, ")
           if (i + 1) % 12 == 0:
               f.write("\n")
       f.write(f"}};\nconst int model_data_len = {len(data)};\n")
   ```

### Kompilacja firmware AtomS3R-CAM
8. **Zainstaluj biblioteki Arduino**:
   - `TensorFlowLite_ESP32` (Arduino Library Manager)
   - `esp32-camera` (lub `ESP32Camera` z Espressif)
   - `Wire` (wbudowana — dla I2C slave)
9. **Struktura firmware AtomS3R-CAM**:
   ```
   atomS3r_cam_firmware/
   ├── atomS3r_cam_firmware.ino   // main — setup + loop
   ├── model_data.h               // model TFLite jako C array
   ├── camera_config.h            // konfiguracja kamery GC0308
   ├── ai_inference.h             // wrapper TFLite inference
   └── i2c_slave.h                // I2C slave komunikacja z masterem
   ```
10. **Wgraj firmware** przez USB-C:
    - Podłącz AtomS3R-CAM przez USB-C
    - Tools → Port → wybierz port COM
    - Kliknij Upload

### Weryfikacja
11. **Test I2C**: Z M5StickC Plus 2 uruchom I2C scanner — powinien wykryć 0x40 (AtomS3R-CAM)
12. **Test AI**: Monitor Serial na AtomS3R-CAM — powinien logować wyniki detekcji
13. **Test end-to-end**: M5StickC Plus 2 odczytuje wyniki AI z AtomS3R-CAM przez I2C

---

## Decyzje technologiczne
1. **Grove Hub (I2C)** dla wszystkich połączeń — BugC2 + ToF4M + AtomS3R-CAM na wspólnej magistrali
2. **I2C slave** na AtomS3R-CAM — master (M5StickC Plus 2) odpytuje o wyniki AI
3. **TFLite Micro** na AtomS3R-CAM — person detection / obstacle detection model
4. **FreeRTOS** wielowątkowość — osobne taski dla: odczytu ToF, sterowania silnikami, komunikacji, LCD
5. **Dwa osobne firmware**: M5StickC Plus 2 (master) + AtomS3R-CAM (slave AI)
