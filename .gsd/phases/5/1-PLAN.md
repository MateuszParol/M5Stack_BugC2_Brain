---
phase: 5
plan: 1
wave: 1
---

# Plan 5.1: AtomS3R-CAM AI Vision System

## Objective
Zaprogramowanie kamery M5Stack AtomS3R-CAM jako niezależnego modułu AI (osobny firmware). Moduł ten będzie uruchamiał lekki model (np. TensorFlow Lite lub wbudowane funkcje detekcji koloru/ruchu) w celu wspomagania unikania kolizji. Skomunikuje się z głównym kontrolerem (M5StickC Plus 2) przez protokół I2C (na współdzielonej magistrali) lub dedykowany UART przekierowany przez Grove HUB jako Slave. Główne zadanie kamery to raportowanie: "Droga czysta" lub "Wykryto przeszkodę (rodzaj/rozmiar)".

## Context
- .gsd/SPEC.md (Goal 2: Wspomaganie kamerą AI)
- .gsd/ROADMAP.md (Phase 5: Kamera wykrywa obiekty i wpływa na decyzje jazdy)
- bugc2_brain/config.h (I2C CAM_ADDR 0x40)
- bugc2_brain/display_manager.h (gotowy UI na wyświetlanie statusu)

## Tasks

<task type="auto">
  <name>PlatformIO/Arduino setup dla AtomS3R-CAM (Osobny Projekt)</name>
  <files>
    cam_firmware/platformio.ini
    cam_firmware/src/main.cpp
  </files>
  <action>
    1. Utwórz nowy katalog poboczny `cam_firmware/` (poza `bugc2_brain/`).
    2. Zainicjalizuj strukturę (skrypt/plik `platformio.ini` lub po prostu plik `.ino` jeśli preferowane Arduino IDE). Ze względu na to, że pracowaliśmy w `.ino`, utwórz `cam_firmware/cam_firmware.ino`.
    3. W `cam_firmware.ino` przygotuj szkielet:
       - Konfiguracja kamery (piny dla ESP32-S3 wbudowane w AtomS3R-CAM: PWDN -1, RESET -1, XCLK 39, SIOD 12, SIOC 11, Y9 42, Y8 41, Y7 40, Y6 38, Y5 37, Y4 36, Y3 35, Y2 34, VSYNC 46, HREF 47, PCLK 48).
       - Inicjalizacja I2C Slave na adresie `0x40`.
       - W pętli głównej `loop()` pobieraj klatkę (`esp_camera_fb_get()`).
       - Na ten moment zaimplementuj *mock* AI (np. proste zliczanie klatek lub jasności obrazu reprezentujące "wykryto/nie wykryto") dopóki nie dodamy tflite. Zwracaj `1` dla przeszkody, `0` dla czystej drogi co 2 sekundy w ramach symulacji modelu.
       - Skonfiguruj `Wire.onRequest()` by odpowiadać głównemu kontrolerowi bajtem statusu.
    
    ZADBAJ O: Kod musi obsługiwać standardowe biblioteki `esp_camera.h`. Adres I2C Slave `0x40` (64).
  </action>
  <verify>
    Kompilacja `cam_firmware.ino` (nawet na sucho jako szkic). Sprawdzenie portów kamery.
  </verify>
  <done>
    - Istnieje osobny katalog kodów źródłowych dla kamery
    - Kod konfiguruje I2C jako Slave (0x40) i kamerę
    - Zwraca mockowane dane po zapytaniu
  </done>
</task>

<task type="auto">
  <name>Integracja kamery w M5StickC Plus 2 (Master)</name>
  <files>
    bugc2_brain/vision_sensor.h
    bugc2_brain/vision_sensor.cpp
    bugc2_brain/bugc2_brain.ino
  </files>
  <action>
    1. Utwórz klasę `VisionSensor` wewnątrz `bugc2_brain/`.
       - Konstruktor / `begin()`: Inicjalizacja sprawdzenia komunikacji I2C pod adresem `CAM_ADDR` (0x40).
       - `update()`: Pobiera asynchronicznie polecenia co np. 500ms za pomocą `Wire.requestFrom()`.
       - `bool isObstacleDetected()`: Zwraca przesyłany od kamery sygnał ostrzegawczy.
       - `String getLastDetectionName()`: Zwraca np. "Person" (na razie mock).
    2. Integracja w `bugc2_brain.ino`:
       - Dołącz plik, stwórz obiekt `visionSensor`.
       - Wywołuj w pętli `loop()`.
       - Przekaż wynik do `display.update(..., visionSensor.isConnected(), visionSensor.getLastDetectionName().c_str(), ...)`.
       - Zmodyfikuj detekcję kolizji: jeśli ToF OR VisionSensor zgłasza błąd -> hamuj silniki w trybie Auto/Semi-Auto.
  </action>
  <verify>
    Kompilacja głównego kodu po dołączeniu nowego modułu wizji.
  </verify>
  <done>
    - Główny robot potrafi odczytać status z I2C (0x40)
    - Integracja wpływająca na moduł nawigacyjny i HUD
  </done>
</task>

## Success Criteria
- [ ] Osobny projekt wgrywany na AtomS3R-CAM czytający klatki wizji.
- [ ] AtomS3R-CAM udostępnia informacje przez złącze Grove do M5Stick używając protokołu I2C (lub Serial, chociaż I2C pozwala zamknąć to w jednym Grove Hub rozgałęziaczu).
- [ ] Ekran główny wykazuje CAM: OK oraz najnowszą etykietkę detekcji.
