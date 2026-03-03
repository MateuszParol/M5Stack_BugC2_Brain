# REQUIREMENTS.md

## Format
| ID | Requirement | Source | Status |
|----|-------------|--------|--------|
| REQ-01 | Kontrola 4 silników DC przez I2C (BugC2, 0x38) — przód, tył, lewo, prawo, stop | SPEC Goal 1 | Pending |
| REQ-02 | Modularna struktura projektu Arduino z podziałem na moduły (motors, sensors, ai, comms, display) | SPEC Goal 6 | Pending |
| REQ-03 | Odczyt odległości z czujnika ToF4M (VL53L1X, I2C 0x29) z częstotliwością min. 10Hz | SPEC Goal 1 | Pending |
| REQ-04 | Logika kolizji: <15cm = stop+skręt, 15-30cm = zwolnij, >30cm = normalna jazda | SPEC Goal 1 | Pending |
| REQ-05 | Tryb autonomiczny: losowa eksploracja z unikaniem przeszkód | SPEC Goal 1 | Pending |
| REQ-06 | Algorytm omijania przeszkód: zwolnij → zatrzymaj → skręć w losowym kierunku | SPEC Goal 1 | Pending |
| REQ-07 | Trzy tryby jazdy: AUTONOMOUS, SEMI_AUTO, MANUAL z przejściami między nimi | SPEC Goal 3 | Pending |
| REQ-08 | Sterowanie zdalne przez WiFi (web server / ESP-NOW z Cardputer Advance) | SPEC Goal 4 | Pending |
| REQ-09 | Przełączanie trybów: przycisk BtnA na M5StickC Plus 2 + komenda zdalna | SPEC Goal 3 | Pending |
| REQ-10 | Firmware AtomS3R-CAM: przechwytywanie obrazu + inference TFLite Micro | SPEC Goal 2 | Pending |
| REQ-11 | Komunikacja ESP-NOW: AtomS3R-CAM → M5StickC Plus 2 (wyniki AI) | SPEC Goal 2 | Pending |
| REQ-12 | Integracja wyników wizyjnych z decyzjami jazdy (fuzja sensorów: ToF + kamera) | SPEC Goal 2 | Pending |
| REQ-13 | LCD HUD: wyświetlanie trybu, odległości ToF, statusu kamery, poziomu baterii | SPEC Goal 5 | Pending |
| REQ-14 | Testy integracyjne: każdy tryb działa stabilnie, przełączanie jest płynne | SPEC Goal 1-5 | Pending |
