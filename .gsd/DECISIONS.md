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
