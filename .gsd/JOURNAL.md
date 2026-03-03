# JOURNAL.md — Development Journal

## 2026-03-03: Project Initialization
- Zainicjalizowano projekt GSD dla M5Stack BugC2 Brain
- Przeprowadzono deep questioning — zebrano pełne wymagania
- Zbadano specyfikacje sprzętowe: BugC2, M5StickC Plus 2, ToF4M, AtomS3R-CAM
- Kluczowe odkrycie: AtomS3R-CAM to osobny ESP32-S3, nie ma wbudowanego modelu AI
- Decyzja: ESP-NOW zamiast UART/I2C dla komunikacji z kamerą
- Stworzono SPEC.md, ROADMAP.md (6 faz), REQUIREMENTS.md (14 wymagań)
