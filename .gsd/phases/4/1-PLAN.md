---
phase: 4
plan: 1
wave: 1
---

# Plan 4.1: Remote Control via WiFi Web Server

## Objective
Implementacja klasy `RemoteController` wykorzystującej wbudowane w ESP32 moduły WiFi i WebServer. Robot utworzy własną sieć WiFi (Access Point) lub podłączy się do istniejącej, serwując stronę HTML z interfejsem do sterowania silnikami oraz wyboru trybu jazdy. Zapewni to realizację funkcjonalności "aplikacji na telefon" z wymagań.

## Context
- .gsd/SPEC.md (Goal 4: Sterowanie zdalne przez aplikację mobilną)
- bugc2_brain/motor_driver.h (funkcje jazdy)
- bugc2_brain/driving_mode.h (tryby jazdy)
- bugc2_brain/config.h (hasła WiFi, piny)

## Tasks

<task type="auto">
  <name>Implementacja Remote Controller (Web Server)</name>
  <files>
    bugc2_brain/remote_controller.h
    bugc2_brain/remote_controller.cpp
    bugc2_brain/config.h
  </files>
  <action>
    1. Zaktualizuj `config.h`:
       Dodaj sekcję z konfiguracją WiFi w AP mode:
       ```cpp
       #define WIFI_AP_SSID "BugC2_Robot"
       #define WIFI_AP_PASS "12345678" // min 8 znaków
       #define WEB_SERVER_PORT 80
       ```
       
    2. Stwórz `remote_controller.h/.cpp` z klasą `RemoteController`:
       - Dołącz `<WiFi.h>` oraz `<WebServer.h>`
       - Metody publiczne: 
         - `void begin()` — uruchamia tryb Access Point i serwer HTTP
         - `void handleClient()` — wywoływane w `loop()`, przetwarza zapytania HTTP
         - `String getIPAddress()` — zwraca IP jako string
         - `bool isClientConnected()` — sprawdza, czy ktoś jest podpięty do AP
       - Callbacki obsługiwane zewnętrznie (lub wewnątrz klasy z dostępem do globali - lepiej na callbackach lub zwracaniu statusu):
         Lepiej trzymać logikę prostą. Klasa `RemoteController` może wystawiać flagi i gettery:
         - `String getRequestedCommand()` — zwraca najnowszą komendę z HTTP ("F", "B", "L", "R", "S" dla stop)
         - `int getRequestedMode()` — zwraca żądany tryb (lub -1 jeśli brak zmiany)
         - `void clearRequests()` — czyści flagi po ich odczytaniu
         
    3. Serwer HTTP:
       - Endpoint `/` — zwraca prosty interfejs HTML/CSS/JS (mobilny responsywny UI z przyciskami F, B, L, R, Stop i wyborem Mode). Pobranie strony nie blokuje wykonania. Pamiętaj o użyciu PROGMEM albo stałej `const char*` na kod HTML by oszczędzać RAM. W JS użyj fetch('/api/action?cmd=...') podpiętego pod `ontouchstart` i `ontouchend` ułatwiając sterowanie. `ontouchend` = Stop.
       - Endpoint `/api/action` — przyjmuje zapytanie np. `?cmd=F` i zapisuje komendę.
       - Endpoint `/api/mode` — przyjmuje zapytanie np. `?set=1` (SemiAuto) i zapisuje zmianę. Zwraca JSON ze statusem dla UI.

    NIE UŻYWAJ: Skomplikowanych rameworków webowych, tylko prosty wbudowany `WebServer`.
    ZADBAJ O: Kod HTML osadzony musi mieć meta tag `viewport` `width=device-width, initial-scale=1` żeby ładnie wyglądał na telefonie.
  </action>
  <verify>
    Kompilacja. Oczekuje się braku błędów `WebServer` i `WiFi`.
  </verify>
  <done>
    - Dodane stałe WiFi w `config.h`
    - Klasa `RemoteController` kompiluje się bez błędów
    - Endpoint API przyjmuje akcje i poprawnie aktualizuje wewnętrzne zmienne klasy
  </done>
</task>

<task type="auto">
  <name>Integracja RemoteController i HUD</name>
  <files>
    bugc2_brain/bugc2_brain.ino
    bugc2_brain/display_manager.h
    bugc2_brain/display_manager.cpp
  </files>
  <action>
    1. W `bugc2_brain.ino`:
       - `#include "remote_controller.h"`
       - Globalny: `RemoteController remoteCtrl;`
       - W `setup()` odpal: `remoteCtrl.begin();`
       - Aktualizuj wyświetlanie powitalne o IP adres.

    2. W `display_manager.h/.cpp`:
       - Zmień `cameraConnected` z bool na inne powiadomienie, albo dodaj pole Wi-Fi do HUD-a (np. "AP: 192.168.4.1" na małym pasku na dole).
       - Aktualizuj `update(...)` by mogło to uwzględnić (lub po prostu rysować to nad statusem I2C).
       
    3. W `loop()`:
       - `remoteCtrl.handleClient();`
       - Obsługa przychodzących requestów `getRequestedMode()` — jeśli otrzymano nowy tryb z GUI w telefonie, zmień globalny `currentMode` analogicznie jak przycisk BtnA. Aktualizuj `navigator.setActive()`.
       - Obsługa komend ruchu `getRequestedCommand()` — ALE TYLKO dla trybów `MODE_MANUAL` i `MODE_SEMI_AUTO`. W trybie `MODE_AUTONOMOUS` ignorujemy komendy manualnego ruchu!
       - Przetłumacz "F" -> `motors.forward()`, "B" -> `motors.backward()`, itd.
       - Po obsłużeniu: `remoteCtrl.clearRequests();`

    ZABEZPIECZENIE: W przypadku `MODE_SEMI_AUTO`, jeśli CollisionDetector podaje `ZONE_CRITICAL` z przodu, zignoruj "F" i zatrzymaj silniki. Użyj już istniejącego `collisionDetector.shouldOverrideMotors()`.
  </action>
  <verify>
    Kompilacja i działanie logiki trybów wg zasad.
  </verify>
  <done>
    - Zintegrowany WebServer w pętli loop() bez jej spowalniania (bardzo ważne do płynnej jazdy)
    - Płynne przechodzenie z trybu AUTONOMOUS do MANUAL via Wi-Fi i powrót
    - Ekran M5Stick podaje przydatny IP do połączenia (np. domyślnie AP na 192.168.4.1)
  </done>
</task>

## Success Criteria
- [ ] ESP32 rozgłasza sieć Wi-Fi `BugC2_Robot`.
- [ ] Z telefonu (po wpisaniu IP 192.168.4.1 w przeglądarce) wyświetla się panel sterowania robota z przyciskami kierunku i zmianą trybów.
- [ ] Przycisk z telefonu płynnie steruje robotem w trybie MANUAL i SEMI_AUTO (w tym hamuje w SEMI_AUTO gdy kolizja).
- [ ] Wybranie AUTONOMOUS z telefonu uruchamia `AutoNavigator`, a naciśnięcie MANUAL odzyskuje kontrolę.
