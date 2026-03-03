---
phase: 2
plan: 1
wave: 1
---

# Plan 2.1: ToF4M Sensor Driver & Distance Reading

## Objective
Stworzyć moduł sterownika czujnika VL53L1X (ToF4M) z ciągłym odczytem odległości przez I2C (0x29). Zintegrować odczyty z wyświetlaczem LCD (pasek odległości) i Serial Monitor.

## Context
- .gsd/SPEC.md (Goal 1: wykrywanie kolizji, próg 15cm)
- .gsd/RESEARCH.md (sekcja: ToF4M — I2C 0x29, 40mm-4000mm, 50Hz, VL53L1X library)
- bugc2_brain/config.h (TOF4M_ADDR, TOF_STOP_CM, TOF_SLOW_CM, TOF_READ_INTERVAL)
- bugc2_brain/display_manager.h (drawDistanceBar)

## Tasks

<task type="auto">
  <name>Implementacja modułu ToF4M Sensor</name>
  <files>
    bugc2_brain/tof_sensor.h
    bugc2_brain/tof_sensor.cpp
  </files>
  <action>
    1. Stwórz `tof_sensor.h/.cpp` z klasą `ToFSensor`:
       - `bool begin()` — inicjalizacja VL53L1X, konfiguracja trybu Short Range (lepszy do 15cm), return success/fail
       - `int readDistance()` — odczyt odległości w mm, konwersja na cm, return -1 przy błędzie
       - `bool isObstacleClose()` — true jeśli distance <= TOF_STOP_CM
       - `bool isObstacleNear()` — true jeśli distance <= TOF_SLOW_CM
       - `bool isConnected()` — sprawdzenie obecności na I2C bus
       - `int getLastDistance()` — ostatni poprawny odczyt (bez ponownego pomiaru)
       - Prywatne: `int _lastDistance = -1`, `unsigned long _lastReadTime = 0`
    
    2. Użyj biblioteki `VL53L1X` (Pololu) lub `M5Unit-ToF4M`:
       - `#include <VL53L1X.h>` (Pololu library — bardziej dojrzała)
       - Inicjalizacja: `sensor.init()`, `sensor.setDistanceMode(VL53L1X::Short)`, `sensor.setMeasurementTimingBudget(33000)` (30Hz)
       - Odczyt: `sensor.read()` lub `sensor.readRangeContinuousMillimeters()`
       - Obsługuj timeout: `sensor.timeoutOccurred()` → return -1
    
    3. Dodaj filtrowanie odczytów (opcjonalne, ale zalecane):
       - Średnia krocząca z 3 ostatnich pomiarów (prosty filtr szumów)
       - Odrzucaj spikes (nagłe skoki >50cm w jednym cyklu)
    
    UWAGA: VL53L1X wymaga biblioteki Pololu VL53L1X lub m5stack/M5Unit-ToF4M.
    NIE UŻYWAJ: VL53L0X library — to inny czujnik!
    ZAMIEŃ: `sensor.setTimeout(500)` — timeout 500ms, nie blokuj na zawsze.
  </action>
  <verify>
    Kompilacja w Arduino IDE z zainstalowaną biblioteką VL53L1X.
    Serial output powinien wypisywać odległość co TOF_READ_INTERVAL ms.
  </verify>
  <done>
    - ToFSensor klasa kompiluje się bez błędów
    - Obsługuje: begin, readDistance, isObstacleClose, isObstacleNear
    - Timeout i error handling zaimplementowane
    - Średnia krocząca filtruje szumy
  </done>
</task>

<task type="auto">
  <name>Integracja ToF z main firmware i LCD</name>
  <files>
    bugc2_brain/bugc2_brain.ino
  </files>
  <action>
    1. Dodaj `#include "tof_sensor.h"` w bugc2_brain.ino
    2. Stwórz globalny obiekt: `ToFSensor tofSensor;`
    3. W `setup()`:
       - Po Wire.begin() i I2C scan → `tofSensor.begin()`
       - Wyświetl status na LCD i Serial: "ToF: OK" lub "ToF: FAIL"
    4. W `loop()`:
       - Odczytuj odległość co TOF_READ_INTERVAL ms (millis-based, nie delay!)
       - `currentDistance = tofSensor.readDistance()`
       - Już przekazywane do `display.update()` → pasek odległości automatycznie się zaktualizuje
       - Loguj na Serial co 500ms: `[ToF] Distance: XXcm`
    5. Dodaj wizualne ostrzeżenia:
       - <15cm: LED BugC2 = czerwony pulsujący
       - 15-30cm: LED = żółty
       - >30cm: LED = zielony (normalny)
    
    NIE: Dodawaj jeszcze logiki hamowania — to jest Plan 2.2.
  </action>
  <verify>
    Kompilacja bez błędów.
    Na hardware: Serial Monitor pokazuje odległość, LCD aktualizuje pasek.
  </verify>
  <done>
    - ToF odczyty widoczne na LCD (pasek odległości z kolorami)
    - Serial loguje odległość
    - LED reagują na odległość (czerwony/żółty/zielony)
    - Brak czujnika = graceful degradation (nie crash)
  </done>
</task>

## Success Criteria
- [ ] Czujnik ToF4M odczytuje odległość w cm (40mm-4000mm zakres)
- [ ] LCD pasek odległości pokazuje dane w czasie rzeczywistym
- [ ] LED BugC2 zmieniają kolor w zależności od odległości
- [ ] Brak czujnika nie powoduje crash (graceful error)
