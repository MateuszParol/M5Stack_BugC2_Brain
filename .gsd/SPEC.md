# SPEC.md — Project Specification

> **Status**: `FINALIZED`

## Vision
Autonomiczny robot mobilny oparty na platformie M5Stack BugC2, sterowany przez M5StickC Plus 2, zdolny do samodzielnej eksploracji otoczenia z wykrywaniem kolizji (ToF4M) i wspomaganiem wizyjnym (AtomS3R-CAM AI). Robot obsługuje trzy tryby jazdy: pełna autonomia, półautonomia (człowiek steruje, ale system hamuje przy zagrożeniu kolizją) oraz pełna kontrola manualna. Kod napisany w C++ (Arduino IDE) z myślą o przyszłym rozszerzeniu o wyszukiwanie przedmiotów na podstawie obrazu.

## Goals
1. **Autonomiczna jazda z unikaniem kolizji** — Robot samodzielnie eksploruje otoczenie, wykrywając i omijając przeszkody za pomocą czujnika ToF4M (próg hamowania: 15cm)
2. **Wspomaganie kamerą AI** — AtomS3R-CAM zapewnia wizyjne wykrywanie obiektów, rozpoznawanie drogi, detekcję krawędzi (TensorFlow Lite)
3. **Trzy tryby jazdy** — Autonomiczny, półautonomiczny (auto-hamowanie przy kolizji), manualny z płynnym przełączaniem
4. **Sterowanie zdalne** — Kontrola manualna przez aplikację mobilną (WiFi) lub M5Stack Cardputer Advance
5. **Informacyjny HUD na LCD** — Wyświetlanie trybu jazdy, odległości od przeszkód, statusu kamery na ekranie M5StickC Plus 2
6. **Architektura gotowa na rozbudowę** — Modularny kod umożliwiający łatwe dodanie wyszukiwania przedmiotów na podstawie zdjęcia

## Non-Goals (Out of Scope)
- Wyszukiwanie przedmiotów na podstawie zdjęcia (przyszły feature)
- Mapowanie SLAM / budowanie mapy otoczenia
- Sterowanie głosowe
- Integracja z chmurą / zdalne monitorowanie przez internet
- Kompatybilność z UIFlow 2.0 (Blockly/MicroPython) — priorytet to natywny C++ w Arduino IDE

## Hardware

### Platforma
| Komponent | Model | Rola |
|-----------|-------|------|
| Kontroler główny | **M5StickC Plus 2** | Mózg robota (ESP32-PICO-V3-02, 240MHz, 8MB Flash, 2MB PSRAM) |
| Baza robota | **M5Stack BugC2** | Silniki DC (L9110S), bateria 700mAh, STM32F030, I2C 0x38 |
| Czujnik odległości | **ToF4M** (VL53L1X) | Wykrywanie przeszkód, I2C 0x29 |
| Kamera AI | **AtomS3R-CAM** | Wizja komputerowa (ESP32-S3, GC0308, 0.3MP) |
| Pilot (opcja) | **M5Stack Cardputer Advance** | Sterowanie zdalne (WiFi) |
| Hub | **Grove Hub** | Rozdzielenie magistrali I2C na wiele urządzeń |

### Połączenia
```
M5StickC Plus 2
  └── Grove Port (GPIO21=SDA, GPIO22=SCL, 5V, GND)
       └── Grove Hub
            ├── BugC2 Motor Driver (I2C 0x38)
            ├── ToF4M Sensor (I2C 0x29)
            └── AtomS3R-CAM (I2C slave / UART via Grove)
```

> **Wszystkie urządzenia podłączone kablem Grove** przez Grove Hub. AtomS3R-CAM działa jako I2C slave na magistrali współdzielonej z BugC2 i ToF4M.

## Users
Właściciel robota (użytkownik hobbystyczny) — steruje i monitoruje robota bezpośrednio lub zdalnie.

## Constraints
- **Hardware**: Jeden port Grove na M5StickC Plus 2 — rozwiązany przez Grove Hub (wszystko kablem)
- **Pamięć**: ESP32-PICO-V3-02 ma 2MB PSRAM — modele AI muszą być lekkie
- **Kamera**: GC0308 to 0.3MP — ograniczona rozdzielczość obrazu
- **Zasilanie**: Bateria BugC2 700mAh — optymalizacja zużycia energii
- **Język**: C++ (Arduino IDE) — brak UIFlow 2.0
- **Czujnik ToF**: Jeden czujnik = jeden kierunek — ograniczone pole widzenia

## Success Criteria
- [ ] Robot jedzie autonomicznie, omijając przeszkody wykryte czujnikiem ToF4M (próg 15cm)
- [ ] Kamera AI wykrywa przeszkody wizyjne i przekazuje dane do kontrolera
- [ ] Przełączanie między 3 trybami jazdy działa poprawnie (przycisk + zdalne)
- [ ] W trybie półautonomicznym robot automatycznie hamuje przy zagrożeniu kolizją
- [ ] Sterowanie zdalne działa przez WiFi (aplikacja mobilna / Cardputer Advance)
- [ ] LCD wyświetla aktualny tryb, odległość, status kamery
- [ ] Kod kompiluje się i działa na oryginalnym sprzęcie M5Stack
