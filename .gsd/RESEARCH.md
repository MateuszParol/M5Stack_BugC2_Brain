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
- **AI**: Brak wbudowanego modelu — wymaga wgrania TensorFlow Lite Micro lub innego modelu

### Kluczowe odkrycie: Architektura dwu-procesorowa
AtomS3R-CAM ma własny ESP32-S3 z 8MB PSRAM — to osobny komputer, który może:
1. Samodzielnie uruchamiać modele TFLite (person detection, object detection)
2. Przetwarzać obraz lokalnie i wysyłać wyniki (nie surowy obraz) do M5StickC Plus 2
3. Komunikować się z M5StickC Plus 2 przez WiFi/ESP-NOW (preferowane) lub UART

### Zalecana architektura komunikacji
```
AtomS3R-CAM (ESP32-S3)          M5StickC Plus 2 (ESP32)
├── Kamera GC0308                ├── BugC2 Motor Driver (I2C 0x38)
├── TFLite model                 ├── ToF4M Sensor (I2C 0x29)
├── Przetwarzanie obrazu         ├── LCD Display
└── ESP-NOW TX ────────────────> └── ESP-NOW RX + logika jazdy
    (wyniki AI: kierunek,            (decyzje: silniki + wyświetlacz)
     przeszkody, obiekty)
```

## Komunikacja ESP-NOW
- **Protokół**: ESP-NOW (Espressif)
- **Opóźnienie**: <1ms (peer-to-peer)
- **Zasięg**: Do 100m w otwartym terenie
- **Przepustowość**: 250 bajtów/pakiet
- **Zalety**: Zero konfiguracji, niskie zużycie energii, brak routera WiFi
- **Kompatybilność**: ESP32 ↔ ESP32-S3 (wspierane)

## Decyzje technologiczne
1. **ESP-NOW** zamiast UART/I2C dla kamery — jeden port Grove wystarczy dla BugC2 + ToF4M
2. **TFLite Micro** na AtomS3R-CAM — person detection / obstacle detection model
3. **FreeRTOS** wielowątkowość — osobne taski dla: odczytu ToF, sterowania silnikami, komunikacji, LCD
4. **Grove Hub** jako backup — jeśli ESP-NOW nie wystarczy
