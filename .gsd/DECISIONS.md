<<<<<<< HEAD
## Phase 6.1 Decisions (Diagnostic Firmware)

**Date:** 2026-03-03

### Scope
- Stworzenie dedykowanego, uproszczonego firmware'u (`bugc2_diagnostic`) do testów sprzętowych płyty bazowej BugC2.
- Kod ma całkowicie ignorować piny I2C dla czujnika ToF4M oraz kamery AtomS3R-CAM.
- Zaimplementowanie wyłącznie funkcji jazdy z użyciem serwera webowego (WiFi AP: `BugC2_Robot`).
- Wbudowany tryb MANUAL jako jedyny dostępny tryb (usunięcie mechanizmów zatrzymywania awaryjnego z czujników).

### Approach
- Chose: Utworzenie nowej struktury katalogów `bugc2_diagnostic` obok istniejącego `bugc2_brain`.
- Reason: Zachowanie oryginalnego i pełnego firmware'u `bugc2_brain` wersji 1.0 pozwala na łatwy powrót do pełnej funkcjonalności po zweryfikowaniu działania sprzętu (silników BugC2).
- W nowym firmware skopiujemy tylko wymagane pliki (`config.h`, `motor_driver.h/cpp`, `remote_controller.h/cpp`, i `bugc2_diagnostic.ino`) z usunięciem w nich wszelkich odniesień do czujników.

### Constraints
- Tylko jeden port I2C zostaje zainicjalizowany (w kierunku silników BugC2 pod adresem 0x38).
- Wymaga fizycznego włączenia zasilania na spodzie BugC2, o czym trzeba pamiętać podczas wgrywania.
=======
# DECISIONS.md — Architecture Decision Records

## Format
| # | Date | Decision | Rationale | Status |
|---|------|----------|-----------|--------|
| ADR-01 | 2026-03-03 | ~~ESP-NOW~~ → Grove Hub (I2C) dla komunikacji AtomS3R-CAM ↔ M5StickC Plus 2 | Użytkownik posiada Grove Hub. Wszystkie urządzenia na wspólnej magistrali I2C. AtomS3R-CAM jako I2C slave | Accepted (revised) |
| ADR-02 | 2026-03-03 | Arduino IDE (C++) zamiast UIFlow 2.0 | Wydajność, wielowątkowość FreeRTOS, pełen dostęp do bibliotek wizyjnych, kontrola niskopoziomowa | Accepted |
| ADR-03 | 2026-03-03 | TFLite Micro na AtomS3R-CAM do wizji komputerowej | ESP32-S3 z 8MB PSRAM obsługuje TFLite, lokalne przetwarzanie bez transferu obrazu | Accepted |
| ADR-04 | 2026-03-03 | Dwa osobne firmware (M5StickC Plus 2 + AtomS3R-CAM) | Różne chipy ESP32 wymagają osobnych projektów. Komunikacja przez I2C (Grove Hub) | Accepted |
| ADR-05 | 2026-03-03 | FreeRTOS tasks dla wielowątkowości | Równoległy odczyt sensorów, sterowanie silnikami, komunikacja, LCD — bez blokowania | Accepted |
>>>>>>> a5063d07b15cca2d849e0a95c10ca807d2509a0f
