---
phase: 1
plan: 1
wave: 1
---

# Plan 1.1: Arduino Project Structure & BugC2 Motor Control

## Objective
Stworzyć bazowy projekt Arduino dla M5StickC Plus 2 z modularną architekturą plików. Zaimplementować sterownik silników BugC2 (I2C 0x38) z funkcjami: jazda przód/tył, skręt lewo/prawo, stop, regulacja prędkości.

## Context
- .gsd/SPEC.md
- .gsd/RESEARCH.md (sekcja: M5Stack BugC2, Komendy I2C BugC2)

## Tasks

<task type="auto">
  <name>Stwórz strukturę projektu Arduino</name>
  <files>
    bugc2_brain/bugc2_brain.ino
    bugc2_brain/config.h
    bugc2_brain/motor_driver.h
    bugc2_brain/motor_driver.cpp
  </files>
  <action>
    1. Stwórz folder `bugc2_brain/` w katalogu głównym projektu
    2. Stwórz `config.h` z definicjami:
       - I2C adresy: BUGC2_ADDR (0x38), TOF4M_ADDR (0x29), CAM_ADDR (0x40)
       - Piny I2C: SDA_PIN (21), SCL_PIN (22)
       - Piny przycisków: BTN_A (37), BTN_B (39)
       - Prędkości silników: SPEED_SLOW (40), SPEED_NORMAL (70), SPEED_FAST (100)
       - Progi ToF: TOF_STOP_CM (15), TOF_SLOW_CM (30)
       - Stałe LCD: szerokość (135), wysokość (240)
    3. Stwórz `motor_driver.h/.cpp` z klasą `MotorDriver`:
       - `void begin()` — inicjalizacja I2C dla BugC2
       - `void setMotor(uint8_t motor, int8_t speed)` — ustaw prędkość jednego silnika (0-3, -127..127)
       - `void forward(int8_t speed)` — jazda do przodu (wszystkie silniki)
       - `void backward(int8_t speed)` — jazda do tyłu
       - `void turnLeft(int8_t speed)` — skręt w lewo (silniki prawe naprzód, lewe stop lub wstecz)
       - `void turnRight(int8_t speed)` — skręt w prawo
       - `void stop()` — zatrzymaj wszystkie silniki
       - `float getBatteryVoltage()` — odczyt napięcia baterii (rejestr 0x10)
       - `void setLED(uint8_t led, uint8_t r, uint8_t g, uint8_t b)` — sterowanie LED RGB
       - Prywatne: `void writeI2C(uint8_t reg, uint8_t data)` — zapis do rejestru I2C
    4. Stwórz `bugc2_brain.ino` (main):
       - `#include <M5StickCPlus2.h>` (lub M5Unified)
       - `#include <Wire.h>`
       - `#include "config.h"`
       - `#include "motor_driver.h"`
       - W `setup()`: inicjalizacja M5, I2C (Wire.begin(SDA, SCL)), MotorDriver, LCD info startowe
       - W `loop()`: tymczasowy test — jazda do przodu 2s, stop 1s, skręt 1s (cykl)
    
    UWAGA: Mapowanie silników (który numer = które koło) może wymagać kalibracji.
    Silniki BugC2 to 0=lewy-przód, 1=prawy-przód, 2=lewy-tył, 3=prawy-tył (domyślne — do weryfikacji empirycznej).
    
    NIE UŻYWAJ: `delay()` w docelowym kodzie — to blokuje. Na razie dla testów OK, ale dodaj komentarz TODO.
  </action>
  <verify>
    arduino-cli compile --fqbn esp32:esp32:m5stick_c_plus2 bugc2_brain/
    Lub: Otwórz w Arduino IDE → Verify/Compile bez błędów
  </verify>
  <done>
    - Projekt kompiluje się bez błędów
    - Struktura plików jest modularna (config.h, motor_driver.h/cpp, main.ino)
    - Klasa MotorDriver ma wszystkie wymagane metody
    - config.h zawiera wszystkie stałe konfiguracyjne
  </done>
</task>

<task type="checkpoint:human-verify">
  <name>Test fizyczny: BugC2 Motor Control</name>
  <files>bugc2_brain/bugc2_brain.ino</files>
  <action>
    1. Wgraj firmware na M5StickC Plus 2 podłączony do BugC2
    2. Obserwuj zachowanie silników:
       - Czy robot jedzie do przodu? (wszystkie koła w tym samym kierunku)
       - Czy stop działa? (silniki się zatrzymują)
       - Czy skręt działa? (prawidłowe koła obracają się)
    3. Jeśli mapowanie silników jest złe — popraw w motor_driver.cpp
    4. Sprawdź odczyt baterii na Serial Monitor
    5. Sprawdź działanie LED RGB
  </action>
  <verify>
    Użytkownik potwierdza:
    - Robot jedzie prosto do przodu
    - Robot skręca prawidłowo
    - Stop zatrzymuje robota
    - Serial Monitor pokazuje napięcie baterii
  </verify>
  <done>
    - Silniki reagują na komendy z kodu
    - Mapowanie silników jest poprawne
    - Odczyt baterii działa
  </done>
</task>

## Success Criteria
- [ ] Projekt Arduino kompiluje się bez błędów dla ESP32 (M5StickC Plus 2)
- [ ] Struktura plików jest modularna i rozszerzalna
- [ ] MotorDriver obsługuje: forward, backward, turnLeft, turnRight, stop
- [ ] LED BugC2 można sterować z kodu
- [ ] Odczyt napięcia baterii działa
