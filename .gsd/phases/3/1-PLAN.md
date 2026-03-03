---
phase: 3
plan: 1
wave: 1
---

# Plan 3.1: Autonomous Navigation Engine

## Objective
Stworzyć moduł autonomicznej nawigacji z algorytmem losowej eksploracji i omijania przeszkód. Silnik nawigacji korzysta z CollisionDetector i MotorDriver do samodzielnej jazdy, skrętów i unikania kolizji.

## Context
- .gsd/SPEC.md (Goal 1: autonomiczna jazda, losowa eksploracja)
- bugc2_brain/collision_detector.h (CollisionZone, getAdjustedSpeed)
- bugc2_brain/motor_driver.h (forward, turnLeft, turnRight, spinLeft, spinRight, stop)
- bugc2_brain/config.h (SPEED_*, TOF_STOP_CM, TOF_SLOW_CM)

## Tasks

<task type="auto">
  <name>Implementacja Autonomous Navigator</name>
  <files>
    bugc2_brain/auto_navigator.h
    bugc2_brain/auto_navigator.cpp
  </files>
  <action>
    1. Stwórz `auto_navigator.h/.cpp` z klasą `AutoNavigator`:
       - Enum `NavState`:
         ```cpp
         enum NavState {
           NAV_IDLE,           // Tryb nieaktywny (nie w AUTONOMOUS)
           NAV_FORWARD,        // Jazda prosto
           NAV_AVOIDING,       // Omijanie przeszkody (w trakcie manewru)
           NAV_TURNING,        // Losowy skręt eksploracyjny
           NAV_REVERSING       // Cofanie po kolizji
         };
         ```
       
       - `void begin(MotorDriver* motors, CollisionDetector* collision)` — referencje
       - `void setActive(bool active)` — włącz/wyłącz navigator
       - `bool isActive()` — getter
       - `void update()` — główna pętla decyzyjna (wywoływana co loop cycle)
       - `NavState getState()` — aktualny stan
       - `static const char* stateToString(NavState state)` — do logowania/LCD
       
    2. Algorytm autonomicznej jazdy (maszyna stanów w `update()`):
       
       **NAV_FORWARD** (domyślny):
       - Jedź prosto z prędkością dostosowaną do strefy: `collision->getAdjustedSpeed(SPEED_NORMAL)`
       - Jeśli ZONE_CRITICAL → przejdź do NAV_REVERSING
       - Jeśli ZONE_WARNING → zwolnij (getAdjustedSpeed automatycznie)
       - Co 3-8 sekund (losowo) → przejdź do NAV_TURNING (eksploracja)
       
       **NAV_REVERSING** (cofanie po kolizji):
       - Cofaj przez 500ms z SPEED_SLOW
       - Po 500ms → przejdź do NAV_AVOIDING
       
       **NAV_AVOIDING** (omijanie przeszkody):
       - Losowo wybierz kierunek: spinLeft lub spinRight (50/50)
       - Obracaj się przez 500-1500ms (losowo)
       - Po obrocie → jeśli ZONE_SAFE → NAV_FORWARD
       - Jeśli nadal przeszkoda → kontynuuj obrót (max 3 sekundy)
       - Jeśli po 3s nadal przeszkoda → spróbuj drugi kierunek
       
       **NAV_TURNING** (losowa eksploracja):
       - Losowo: turnLeft lub turnRight z SPEED_SLOW
       - Przez 300-1000ms (losowy kąt)
       - Po skręcie → NAV_FORWARD
       
       **NAV_IDLE**:
       - Nic nie rób (silniki sterowane zewnętrznie)
       
    3. Dodaj losowość:
       - `random(min, max)` dla czasów i kierunków
       - `randomSeed(analogRead(0))` w begin() dla różnych sekwencji
       - Zmienna `_nextExploreTime` — losowy czas do następnego skrętu eksploracyjnego
    
    4. Zabezpieczenia:
       - Jeśli setActive(false) → natychmiast stop() i NAV_IDLE
       - Timeout: jeśli w NAV_AVOIDING > 5s → zatrzymaj się, cofnij, spróbuj ponownie
       - Nigdy nie jedź do przodu gdy ZONE_CRITICAL (CollisionDetector i tak blokuje)
    
    NIE UŻYWAJ: delay() — wszystko na millis() + maszyna stanów.
    NIE RUSZAJ: CollisionDetector — navigator korzysta z niego, ale go nie modyfikuje.
  </action>
  <verify>
    Kompilacja bez błędów w Arduino IDE.
    Symulacja logiki: Serial output powinien pokazywać przejścia stanów.
  </verify>
  <done>
    - AutoNavigator ma pełną maszynę stanów (IDLE, FORWARD, AVOIDING, TURNING, REVERSING)
    - Algorytm losowej eksploracji z omijaniem przeszkód
    - Brak delay() — pełna współpraca z loop()
    - Zabezpieczenie: timeout w AVOIDING, natychmiastowy stop przy deaktywacji
  </done>
</task>

<task type="auto">
  <name>Integracja AutoNavigator z main firmware</name>
  <files>
    bugc2_brain/bugc2_brain.ino
  </files>
  <action>
    1. W `bugc2_brain.ino`:
       - `#include "auto_navigator.h"`
       - Globalny: `AutoNavigator navigator;`
       - W `setup()`: `navigator.begin(&motors, &collisionDetector);`
    
    2. W `loop()` — dodaj logikę aktywacji per tryb:
       ```
       // Aktywuj/deaktywuj navigator w zależności od trybu
       navigator.setActive(currentMode == MODE_AUTONOMOUS);
       
       // Autonomiczna nawigacja (tylko jeśli aktywna i nie ma motor testu)
       if (navigator.isActive() && !testRunning) {
         navigator.update();
       }
       ```
    
    3. Przy zmianie trybu (BtnA):
       - Z AUTONOMOUS na inny → navigator.setActive(false) → motors.stop()
       - Na AUTONOMOUS → navigator.setActive(true)
    
    4. Aktualizuj LCD:
       - W display.update() lub drawMessage() pokaż NavState gdy w trybie AUTONOMOUS
       - Np. "AUTO: Forward" / "AUTO: Avoiding..." / "AUTO: Exploring"
    
    5. Serial log: `[Nav] State: FORWARD → AVOIDING (obstacle at 12cm)`
    
    6. Zaktualizuj wersję: v1.2.0-alpha (Phase 3)
  </action>
  <verify>
    Kompilacja. Na hardware: przełącz na AUTONOMOUS (BtnA) → robot jedzie i omija przeszkody.
  </verify>
  <done>
    - Navigator aktywuje się TYLKO w trybie AUTONOMOUS
    - Przełączanie trybów poprawnie startuje/zatrzymuje navigator
    - LCD wyświetla stan nawigacji
    - Robot autonomicznie eksploruje i omija przeszkody
  </done>
</task>

## Success Criteria
- [ ] Robot w trybie AUTONOMOUS jedzie prosto, omija przeszkody i losowo eksploruje
- [ ] Przełączenie na MANUAL/SEMI_AUTO natychmiast zatrzymuje autonomiczną jazdę
- [ ] Omijanie przeszkód: cofnij → losowy spin → jedź dalej
- [ ] Brak zakleszczeń — timeout 5s w AVOIDING wymusza reset manewru
- [ ] LCD i Serial logują aktualny stan nawigacji
