---
phase: 6
plan: 1
wave: 1
---

# Plan 6.1: Polish & Optimization

## Objective
Kod osiągnął pożądaną funkcjonalność. Etap ten polega na przeglądzie, refaktoryzacji, optymalizacji wydajności głównej pętli `loop()` oraz usunięciu martwego/zbędnego kodu testowego. HUD został już zaimplementowany w poprzednich fazach, więc skupiamy się na "Polish" (szlifach) i weryfikacji całego systemu.

## Context
- .gsd/SPEC.md (Wszystkie cele)
- bugc2_brain/bugc2_brain.ino
- bugc2_brain/config.h

## Tasks

<task type="auto">
  <name>Czyszczenie i optymalizacja pętli głównej</name>
  <files>
    bugc2_brain/bugc2_brain.ino
    bugc2_brain/config.h
  </files>
  <action>
    1. Przejrzyj `bugc2_brain.ino`.
    2. Zweryfikuj, czy zadania takie jak odczyt baterii, I2C z kamery, WebServer i AutoNavigator mają odpowiednie nieblokujące limity czasowe (`millis()`).
    3. Usuń sekcję `TestPhase` (zmienne: `currentTest`, `testTimer`, `testRunning`, funkcję `runMotorTest()` i obsługę długiego naciśnięcia przycisku BtnA), ponieważ platforma jest już przetestowana, a chcemy uwolnić pamięć i uprościć kod.
    4. Sformatuj kod dla lepszej czytelności (np. grupując deklaracje).
  </action>
  <verify>
    Kompilacja i sprawdzenie, czy usunięcie testów silnika nie zepsuło innych funkcjonalności (BtnA ma działać tylko jako zmiana trybu).
  </verify>
  <done>
    - Brak referencji do `runMotorTest` i zmiennych testRunning w kodzie.
    - BtnA obsługuje tylko krótkie naciśnięcia do zmiany trybów.
  </done>
</task>

<task type="auto">
  <name>Finalne ustawienia startowe</name>
  <files>
    bugc2_brain/bugc2_brain.ino
  </files>
  <action>
    1. Na koniec `setup()`, upewnij się, że tryb początkowy to konsekwentnie `MODE_MANUAL` i wyświetla się odpowiednio na ekranie oraz status silników i LEDów jest prawidłowy.
    2. Sprawdź, czy ekran IP poprawnie nadpisuje ewentualne komunikaty "Ready".
  </action>
  <verify>
    Kompilacja i weryfikacja wizualna `setup()`.
  </verify>
  <done>
    - Robot zachowuje się bezpiecznie po zasileniu (stoi w miejscu, czeka na komendy z WiFi lub zmianę trybu przez BtnA).
  </done>
</task>

## Success Criteria
- [ ] Zoptymalizowany firmware pozbawiony kodu deweloperskiego (motor tester)
- [ ] Płynnie działający system wielozadaniowy reagujący bez zwłoki na wejścia z ToF, Kamery, Przycisków i WiFi
