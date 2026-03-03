---
phase: 2
plan: 2
wave: 1
---

# Plan 2.2: Collision Detection Logic & Emergency Response

## Objective
Zaimplementować logikę wykrywania kolizji z trzema strefami reagowania (bezpieczna, ostrzegawcza, krytyczna). System automatycznie zwalnia i zatrzymuje robota niezależnie od aktualnego trybu jazdy.

## Context
- .gsd/SPEC.md (Goal 1: próg hamowania 15cm, Goal 3: auto-hamowanie w SEMI_AUTO)
- bugc2_brain/config.h (TOF_STOP_CM=15, TOF_SLOW_CM=30)
- bugc2_brain/tof_sensor.h (z Planu 2.1)
- bugc2_brain/motor_driver.h (stop, forward, backward)
- bugc2_brain/driving_mode.h (DrivingMode enum)

## Tasks

<task type="auto">
  <name>Implementacja Collision Detector</name>
  <files>
    bugc2_brain/collision_detector.h
    bugc2_brain/collision_detector.cpp
  </files>
  <action>
    1. Stwórz `collision_detector.h/.cpp` z klasą `CollisionDetector`:
       - Enum `CollisionZone`:
         ```cpp
         enum CollisionZone {
           ZONE_SAFE,      // > TOF_SLOW_CM (30cm)  — pełna prędkość
           ZONE_WARNING,   // TOF_STOP_CM..TOF_SLOW_CM (15-30cm) — zwolnij
           ZONE_CRITICAL   // < TOF_STOP_CM (15cm) — STOP!
         };
         ```
       
       - `void begin(ToFSensor* tof, MotorDriver* motors)` — zarejestruj referencje
       - `CollisionZone update(int distanceCm)` — oblicz strefę na podstawie odległości
       - `bool shouldOverrideMotors()` — true jeśli ZONE_CRITICAL lub ZONE_WARNING
       - `int getAdjustedSpeed(int requestedSpeed)` — zwraca skorygowaną prędkość:
         - ZONE_SAFE: requestedSpeed (100%)
         - ZONE_WARNING: requestedSpeed * 0.5 (50% — zwolnienie)
         - ZONE_CRITICAL: 0 (stop!)
       - `CollisionZone getCurrentZone()` — getter
       - `const char* zoneToString(CollisionZone zone)` — do wyświetlania
       - `unsigned long getTimeSinceLastCollision()` — czas od ostatniego CRITICAL
    
    2. Dodaj reakcję na kolizję:
       - Przy ZONE_CRITICAL: natychmiast motors->stop(), ustaw flagę `_collisionActive = true`
       - Odblokowanie: dopiero gdy odległość > TOF_SLOW_CM przez 500ms (debounce)
       - To zapobiega "drganiom" — robot nie rusza natychmiast po cofnięciu przeszkody
    
    3. Logika per tryb jazdy:
       - MODE_AUTONOMOUS: CollisionDetector pełna kontrola (stop + automatyczny skręt — Phase 3)
       - MODE_SEMI_AUTO: CollisionDetector może TYLKO hamować (getAdjustedSpeed), nigdy skręcać
       - MODE_MANUAL: CollisionDetector wyłączony (użytkownik ma pełną kontrolę)
  </action>
  <verify>
    Kompilacja bez błędów.
    Unit test logiki: SAFE@50cm, WARNING@20cm, CRITICAL@10cm.
  </verify>
  <done>
    - CollisionDetector poprawnie klasyfikuje strefy
    - getAdjustedSpeed zwraca prawidłowe prędkości dla każdej strefy
    - Debounce zapobiega drganiom przy progu
    - Logika per-mode jest poprawna
  </done>
</task>

<task type="auto">
  <name>Integracja CollisionDetector z main firmware</name>
  <files>
    bugc2_brain/bugc2_brain.ino
    bugc2_brain/display_manager.h
    bugc2_brain/display_manager.cpp
  </files>
  <action>
    1. W `bugc2_brain.ino`:
       - `#include "collision_detector.h"`
       - Globalny obiekt: `CollisionDetector collisionDetector;`
       - W `setup()`: `collisionDetector.begin(&tofSensor, &motors);`
       - W `loop()`:
         ```
         // Po odczycie ToF:
         CollisionZone zone = collisionDetector.update(currentDistance);
         
         // W trybie SEMI_AUTO i AUTONOMOUS — collision override
         if (currentMode != MODE_MANUAL) {
           if (zone == ZONE_CRITICAL) {
             motors.stop();
             // LED migające czerwone
           }
         }
         ```
    
    2. Zaktualizuj `display_manager` — dodaj wyświetlanie strefy kolizji:
       - Dodaj metodę `void drawCollisionZone(CollisionZone zone)` 
       - W drawDistanceBar: pokazuj nazwę strefy obok odległości (SAFE/WARN/STOP)
       - ZONE_CRITICAL: migający czerwony tekst "⚠ COLLISION!"
    
    3. Dodaj dźwięk buzzer:
       - ZONE_WARNING: krótki beep co 500ms
       - ZONE_CRITICAL: ciągły alarm buzzer
       - ZONE_SAFE: cisza
       - Użyj M5.Beep (jeśli dostępny) lub tone() na BuzzerPin
    
    4. Serial log: `[Collision] Zone: CRITICAL (12cm)` przy zmianie strefy
  </action>
  <verify>
    Kompilacja. Na hardware: zbliż rękę do czujnika → LED/LCD/buzzer reagują.
  </verify>
  <done>
    - CollisionDetector zintegrowany z main loop
    - LCD wyświetla strefę kolizji (SAFE/WARN/STOP)
    - Buzzer ostrzega dźwiękowo
    - W trybie SEMI_AUTO: auto-hamowanie działa
    - W trybie MANUAL: brak ingerencji (tylko wizualne ostrzeżenia)
  </done>
</task>

## Success Criteria
- [ ] Trzy strefy kolizji działają poprawnie (SAFE >30cm, WARNING 15-30cm, CRITICAL <15cm)
- [ ] W SEMI_AUTO: robot automatycznie hamuje przy ZONE_CRITICAL
- [ ] W MANUAL: tylko wizualne/dźwiękowe ostrzeżenia (brak blokady silników)
- [ ] Debounce zapobiega drganiom przy progu
- [ ] Buzzer i LED ostrzegają o zagrożeniu
