/**
 * tof_sensor.cpp — ToF4M Distance Sensor Implementation
 * 
 * VL53L1X (I2C 0x29) — Time-of-Flight laser distance sensor.
 * Zakres: 40mm - 4000mm, dokładność ±20mm, FOV 27°.
 * Tryb: Short Range (lepszy do bliskich przeszkód ~15cm).
 */

#include "tof_sensor.h"

bool ToFSensor::begin() {
    Serial.println("[ToF] Initializing VL53L1X...");
    
    // Sprawdź czy czujnik jest na magistrali
    if (!isConnected()) {
        Serial.println("[ToF] ERROR: Sensor not found at 0x29!");
        _initialized = false;
        return false;
    }
    
    // Timeout — nie czekaj w nieskończoność
    _sensor.setTimeout(500);
    
    // Inicjalizacja
    if (!_sensor.init()) {
        Serial.println("[ToF] ERROR: init() failed!");
        _initialized = false;
        return false;
    }
    
    // Konfiguracja:
    // Short = lepsza dokładność na krótkich dystansach (do ~130cm)
    // Long  = większy zasięg (do ~400cm), mniejsza dokładność
    _sensor.setDistanceMode(VL53L1X::Short);
    
    // Timing budget: 33ms = ~30Hz odczytów
    // Mniejszy budget = szybsze odczyty, ale mniejsza dokładność
    _sensor.setMeasurementTimingBudget(33000);
    
    // Uruchom ciągły pomiar
    _sensor.startContinuous(TOF_READ_INTERVAL);
    
    // Zainicjalizuj bufor filtra
    for (int i = 0; i < TOF_FILTER_SIZE; i++) {
        _filterBuffer[i] = 0;
    }
    _filterIndex = 0;
    _filterCount = 0;
    
    _initialized = true;
    Serial.println("[ToF] Initialized OK — Short Range mode, 30Hz");
    return true;
}

int ToFSensor::readDistance() {
    if (!_initialized) return -1;
    
    // Sprawdź czy nowy odczyt jest gotowy
    if (!_sensor.dataReady()) {
        return _lastDistance;  // Zwróć ostatni odczyt
    }
    
    // Odczytaj surową wartość (mm)
    int rawMm = _sensor.read(false);  // false = nie czekaj na nowy odczyt
    
    // Sprawdź timeout
    if (_sensor.timeoutOccurred()) {
        Serial.println("[ToF] WARNING: Timeout!");
        return _lastDistance;  // Zwróć ostatni poprawny odczyt
    }
    
    // Sprawdź range status (VL53L1X zwraca status pomiaru)
    // Status 0 = valid, inne = error
    if (_sensor.ranging_data.range_status != 0) {
        // Pomiar niepoprawny — zachowaj ostatni dobry
        return _lastDistance;
    }
    
    // Konwersja mm → cm
    int rawCm = rawMm / 10;
    
    // Ogranicz do sensownego zakresu
    if (rawCm < 0) rawCm = 0;
    if (rawCm > TOF_MAX_CM) rawCm = TOF_MAX_CM;
    
    // Odrzuć spike'y — nagły skok > 50cm w jednym cyklu
    if (_lastDistance > 0 && abs(rawCm - _lastDistance) > 50) {
        // Prawdopodobnie szum — ignoruj ten odczyt
        Serial.printf("[ToF] Spike rejected: %dcm → %dcm\n", _lastDistance, rawCm);
        return _lastDistance;
    }
    
    // Filtr średniej kroczącej
    int filtered = applyFilter(rawCm);
    
    _lastDistance = filtered;
    _lastReadTime = millis();
    
    return filtered;
}

bool ToFSensor::isObstacleClose() {
    return (_lastDistance >= 0 && _lastDistance <= TOF_STOP_CM);
}

bool ToFSensor::isObstacleNear() {
    return (_lastDistance >= 0 && _lastDistance <= TOF_SLOW_CM);
}

bool ToFSensor::isConnected() {
    Wire.beginTransmission(TOF4M_ADDR);
    return (Wire.endTransmission() == 0);
}

int ToFSensor::applyFilter(int rawValue) {
    // Dodaj do cyklicznego bufora
    _filterBuffer[_filterIndex] = rawValue;
    _filterIndex = (_filterIndex + 1) % TOF_FILTER_SIZE;
    
    if (_filterCount < TOF_FILTER_SIZE) {
        _filterCount++;
    }
    
    // Oblicz średnią z dostępnych próbek
    long sum = 0;
    for (int i = 0; i < _filterCount; i++) {
        sum += _filterBuffer[i];
    }
    
    return (int)(sum / _filterCount);
}
