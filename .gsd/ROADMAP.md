# ROADMAP.md

> **Current Phase**: Not started
> **Milestone**: v1.0 — Autonomous Explorer

## Must-Haves (from SPEC)
- [ ] Autonomiczna jazda z omijaniem przeszkód (ToF4M, próg 15cm)
- [ ] Wspomaganie kamerą AI (AtomS3R-CAM + TFLite)
- [ ] Trzy tryby jazdy (auto / półauto / manual)
- [ ] Sterowanie zdalne (WiFi / Cardputer)
- [ ] HUD na LCD M5StickC Plus 2

## Phases

### Phase 1: Foundation — Hardware Abstraction & Motor Control
**Status**: ✅ Complete
**Objective**: Uruchomić BugC2 — kontrola silników, odczyt baterii, LED. Stworzyć bazową strukturę projektu Arduino z modularną architekturą.
**Deliverable**: Robot rusza, skręca, zatrzymuje się na komendę z kodu
**Requirements**: REQ-01 (motor control), REQ-02 (project structure)

### Phase 2: Collision Detection — ToF4M Integration
**Status**: ✅ Complete
**Objective**: Zintegrować czujnik ToF4M, zaimplementować logikę wykrywania przeszkód z progami odległości (15cm stop, 30cm zwolnij).
**Deliverable**: Robot odczytuje odległość i wyświetla ją, reaguje na przeszkody
**Requirements**: REQ-03 (ToF sensor), REQ-04 (collision logic)

### Phase 3: Autonomous Driving — Random Exploration
**Status**: ✅ Complete
**Objective**: Zaimplementować tryb autonomicznej jazdy z losową eksploracją i omijaniem przeszkód na podstawie danych ToF.
**Deliverable**: Robot samodzielnie jeździ i omija przeszkody
**Requirements**: REQ-05 (autonomous mode), REQ-06 (obstacle avoidance)

### Phase 4: Driving Modes — Semi-Auto & Manual + Remote Control
**Status**: ⬜ Not Started
**Objective**: Dodać tryby półautonomiczny i manualny, implementacja sterowania zdalnego przez WiFi/ESP-NOW, przełączanie trybów przyciskiem i zdalnie.
**Deliverable**: 3 tryby jazdy z płynnym przełączaniem + aplikacja/pilot
**Requirements**: REQ-07 (driving modes), REQ-08 (remote control), REQ-09 (mode switching)

### Phase 5: Camera AI — AtomS3R-CAM Vision System
**Status**: ⬜ Not Started
**Objective**: Zaprogramować AtomS3R-CAM (osobny firmware ESP32-S3) z modelem TFLite do wykrywania przeszkód wizyjnych. Komunikacja ESP-NOW z kontrolerem głównym.
**Deliverable**: Kamera wykrywa obiekty i wpływa na decyzje jazdy
**Requirements**: REQ-10 (camera AI), REQ-11 (ESP-NOW comms), REQ-12 (vision integration)

### Phase 6: HUD & Polish
**Status**: ⬜ Not Started
**Objective**: Pełny interfejs LCD (tryb, odległość, status kamery, bateria), optymalizacja wydajności, finalne testy integracyjne.
**Deliverable**: Kompletny, spolishowany robot gotowy do użycia
**Requirements**: REQ-13 (LCD HUD), REQ-14 (integration testing)
