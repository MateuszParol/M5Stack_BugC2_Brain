---
phase: 1
plan: 2
wave: 1
---

# Plan 1.2: LCD Display Module & System Status

## Objective
Zaimplementować moduł wyświetlacza LCD M5StickC Plus 2 pokazujący informacje systemowe: tryb jazdy, napięcie baterii, status I2C. Stworzy to fundament pod przyszły HUD z odległością ToF i statusem kamery.

## Context
- .gsd/SPEC.md (Goal 5: Informacyjny HUD na LCD)
- .gsd/RESEARCH.md (sekcja: M5StickC Plus 2 — ekran 135x240, ST7789V2)
- bugc2_brain/config.h

## Tasks

<task type="auto">
  <name>Implementacja modułu LCD Display</name>
  <files>
    bugc2_brain/display_manager.h
    bugc2_brain/display_manager.cpp
    bugc2_brain/driving_mode.h
    bugc2_brain/bugc2_brain.ino
  </files>
  <action>
    1. Stwórz `driving_mode.h` z enumem:
       ```cpp
       enum DrivingMode {
         MODE_MANUAL,
         MODE_SEMI_AUTO,
         MODE_AUTONOMOUS
       };
       ```
       Plus funkcja `const char* modeToString(DrivingMode mode)`
    
    2. Stwórz `display_manager.h/.cpp` z klasą `DisplayManager`:
       - `void begin()` — inicjalizacja ekranu, orientacja landscape (robot jedzie "od ekranu")
       - `void clear()` — wyczyść ekran
       - `void drawStatusBar(DrivingMode mode, float batteryVoltage)` — górny pasek z trybem i baterią
       - `void drawDistanceBar(int distanceCm)` — pasek odległości z kolorami (zielony/żółty/czerwony)
       - `void drawCameraStatus(bool connected, const char* lastDetection)` — status kamery AI
       - `void drawMessage(const char* msg)` — główna wiadomość na środku ekranu
       - `void update(DrivingMode mode, float battery, int distance, bool camConnected)` — pełna aktualizacja
       
       Użyj kolorów:
       - Tło: czarne (TFT_BLACK)
       - Tryb MANUAL: niebieski, SEMI_AUTO: żółty, AUTONOMOUS: zielony
       - Bateria: zielony >3.7V, żółty 3.3-3.7V, czerwony <3.3V
       - Odległość: zielony >30cm, żółty 15-30cm, czerwony <15cm
    
    3. Zaktualizuj `bugc2_brain.ino`:
       - Dodaj `#include "display_manager.h"` i `#include "driving_mode.h"`
       - W `setup()`: zainicjalizuj DisplayManager, pokaż splash screen "BugC2 Brain v1.0"
       - W `loop()`: aktualizuj wyświetlacz co 200ms (nie co klatkę — oszczędność CPU)
       - Dodaj przycisk BtnA do przełączania trybów (na razie tylko zmiana wyświetlanego trybu)
    
    NIE UŻYWAJ: Zbyt dużych czcionek — ekran 135x240 jest mały.
    UŻYJ: M5.Lcd (z M5StickCPlus2.h) — ma wbudowane funkcje rysowania.
  </action>
  <verify>
    arduino-cli compile --fqbn esp32:esp32:m5stick_c_plus2 bugc2_brain/
    Lub: Arduino IDE → Verify/Compile
  </verify>
  <done>
    - Kompilacja bez błędów
    - DisplayManager ma wszystkie wymagane metody
    - driving_mode.h definiuje trzy tryby jazdy
    - Splash screen widoczny przy uruchomieniu
    - Przycisk BtnA przełącza wyświetlany tryb
  </done>
</task>

<task type="auto">
  <name>I2C Bus Scanner & Diagnostyka</name>
  <files>
    bugc2_brain/i2c_scanner.h
    bugc2_brain/i2c_scanner.cpp
    bugc2_brain/bugc2_brain.ino
  </files>
  <action>
    1. Stwórz `i2c_scanner.h/.cpp` z funkcjami:
       - `int scanI2CBus(uint8_t* foundAddresses, int maxDevices)` — skanuje magistralę I2C, zwraca liczbę znalezionych urządzeń
       - `bool isDevicePresent(uint8_t address)` — sprawdza czy urządzenie jest na adresie
       - `void printScanResults()` — wypisuje wyniki na Serial
    
    2. W `bugc2_brain.ino`:
       - W `setup()` po inicjalizacji I2C — uruchom skan I2C
       - Wyświetl na LCD znalezione urządzenia (BugC2: 0x38, ToF4M: 0x29)
       - Jeśli brakuje urządzenia — wyświetl ostrzeżenie na LCD i Serial
    
    To służy diagnostyce — użytkownik od razu widzi czy wszystko jest podłączone.
  </action>
  <verify>
    arduino-cli compile --fqbn esp32:esp32:m5stick_c_plus2 bugc2_brain/
  </verify>
  <done>
    - I2C scanner wykrywa urządzenia na magistrali
    - Wyniki widoczne na Serial i LCD
    - Brak urządzenia = ostrzeżenie (nie crash)
  </done>
</task>

## Success Criteria
- [ ] LCD wyświetla: tryb jazdy, napięcie baterii, status urządzeń I2C
- [ ] Przycisk BtnA przełącza tryb (MANUAL → SEMI_AUTO → AUTONOMOUS → MANUAL)
- [ ] I2C scanner wykrywa podłączone urządzenia i informuje o brakujących
- [ ] Splash screen "BugC2 Brain v1.0" przy starcie
