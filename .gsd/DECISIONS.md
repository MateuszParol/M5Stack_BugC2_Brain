# DECISIONS.md — Architecture Decision Records

## Format
| # | Date | Decision | Rationale | Status |
|---|------|----------|-----------|--------|
| ADR-01 | 2026-03-03 | ~~ESP-NOW~~ → Grove Hub (I2C) dla komunikacji AtomS3R-CAM ↔ M5StickC Plus 2 | Użytkownik posiada Grove Hub. Wszystkie urządzenia na wspólnej magistrali I2C. AtomS3R-CAM jako I2C slave | Accepted (revised) |
| ADR-02 | 2026-03-03 | Arduino IDE (C++) zamiast UIFlow 2.0 | Wydajność, wielowątkowość FreeRTOS, pełen dostęp do bibliotek wizyjnych, kontrola niskopoziomowa | Accepted |
| ADR-03 | 2026-03-03 | TFLite Micro na AtomS3R-CAM do wizji komputerowej | ESP32-S3 z 8MB PSRAM obsługuje TFLite, lokalne przetwarzanie bez transferu obrazu | Accepted |
| ADR-04 | 2026-03-03 | Dwa osobne firmware (M5StickC Plus 2 + AtomS3R-CAM) | Różne chipy ESP32 wymagają osobnych projektów. Komunikacja przez I2C (Grove Hub) | Accepted |
| ADR-05 | 2026-03-03 | FreeRTOS tasks dla wielowątkowości | Równoległy odczyt sensorów, sterowanie silnikami, komunikacja, LCD — bez blokowania | Accepted |
