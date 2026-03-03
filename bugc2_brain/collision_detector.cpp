/**
 * collision_detector.cpp — Collision Detection Implementation
 * 
 * Logika stref kolizji z debounce i reakcjami per-tryb:
 * - SAFE: normalna jazda, zielone LED, cisza
 * - WARNING: 50% prędkości, żółte LED, beep co 500ms
 * - CRITICAL: STOP!, czerwone LED, ciągły buzzer
 */

#include "collision_detector.h"
#include <M5StickCPlus2.h>

void CollisionDetector::begin(ToFSensor* tof, MotorDriver* motors) {
    _tof = tof;
    _motors = motors;
    _currentZone = ZONE_SAFE;
    _previousZone = ZONE_SAFE;
    _collisionActive = false;
    _overrideActive = false;
    _waitingForDebounce = false;
    
    Serial.println("[Collision] Detector initialized");
    Serial.printf("[Collision] Thresholds: STOP<%dcm, SLOW<%dcm\n", 
                  TOF_STOP_CM, TOF_SLOW_CM);
}

CollisionZone CollisionDetector::update(int distanceCm, DrivingMode currentMode) {
    // Brak danych z czujnika — nie ingeruj
    if (distanceCm < 0) {
        _currentZone = ZONE_SAFE;
        _overrideActive = false;
        return ZONE_SAFE;
    }
    
    // Klasyfikuj strefę
    CollisionZone newZone = classifyDistance(distanceCm);
    
    // Debounce: gdy wychodzisz z CRITICAL → wymagaj 500ms w SAFE
    if (_collisionActive && newZone != ZONE_CRITICAL) {
        if (!_waitingForDebounce) {
            _waitingForDebounce = true;
            _safeStartTime = millis();
        }
        
        if (millis() - _safeStartTime < DEBOUNCE_MS) {
            // Jeszcze za wcześnie — traktuj nadal jako CRITICAL
            newZone = ZONE_CRITICAL;
        } else {
            // Debounce minął — odblokuj
            _collisionActive = false;
            _waitingForDebounce = false;
            Serial.println("[Collision] Debounce complete — unlocked");
        }
    }
    
    // Wykryj zmianę strefy
    if (newZone != _previousZone) {
        handleZoneChange(newZone, currentMode);
        _previousZone = newZone;
    }
    
    _currentZone = newZone;
    
    // Ustaw override w zależności od trybu
    switch (currentMode) {
        case MODE_AUTONOMOUS:
            // Pełna kontrola: hamowanie + (skręt w Phase 3)
            _overrideActive = (newZone != ZONE_SAFE);
            break;
            
        case MODE_SEMI_AUTO:
            // Tylko hamowanie, nigdy skręcanie
            _overrideActive = (newZone == ZONE_CRITICAL);
            break;
            
        case MODE_MANUAL:
            // Brak ingerencji — tylko wizualne/dźwiękowe ostrzeżenia
            _overrideActive = false;
            break;
            
        default:
            _overrideActive = false;
            break;
    }
    
    // Wykonaj zatrzymanie jeśli CRITICAL i override aktywny
    if (_overrideActive && newZone == ZONE_CRITICAL && _motors != nullptr) {
        _motors->stop();
        _collisionActive = true;
        _lastCollisionTime = millis();
    }
    
    // Update wizualny feedback
    updateLEDs(newZone);
    updateBuzzer(newZone);
    
    return _currentZone;
}

int CollisionDetector::getAdjustedSpeed(int requestedSpeed) {
    switch (_currentZone) {
        case ZONE_SAFE:
            return requestedSpeed;              // 100%
        case ZONE_WARNING:
            return requestedSpeed / 2;          // 50%
        case ZONE_CRITICAL:
            return 0;                           // STOP
        default:
            return requestedSpeed;
    }
}

CollisionZone CollisionDetector::classifyDistance(int distanceCm) {
    if (distanceCm <= TOF_STOP_CM) return ZONE_CRITICAL;
    if (distanceCm <= TOF_SLOW_CM) return ZONE_WARNING;
    return ZONE_SAFE;
}

void CollisionDetector::handleZoneChange(CollisionZone newZone, DrivingMode mode) {
    Serial.printf("[Collision] Zone: %s → %s (mode: %s)\n", 
                  zoneToString(_previousZone),
                  zoneToString(newZone),
                  modeToString(mode));
    
    _zoneChangeTime = millis();
    
    if (newZone == ZONE_CRITICAL) {
        Serial.println("[Collision] ⚠ CRITICAL — OBSTACLE DETECTED!");
    }
}

void CollisionDetector::updateBuzzer(CollisionZone zone) {
    static unsigned long lastBeepTime = 0;
    static bool buzzerOn = false;
    unsigned long now = millis();
    
    switch (zone) {
        case ZONE_SAFE:
            // Cisza
            if (buzzerOn) {
                M5.Speaker.stop();
                buzzerOn = false;
            }
            break;
            
        case ZONE_WARNING:
            // Beep co 500ms
            if (now - lastBeepTime >= 500) {
                M5.Speaker.tone(2000, 50);  // 2kHz, 50ms
                lastBeepTime = now;
            }
            break;
            
        case ZONE_CRITICAL:
            // Ciągły alarm — krótkie beepy co 150ms
            if (now - lastBeepTime >= 150) {
                M5.Speaker.tone(3000, 100);  // 3kHz, 100ms
                lastBeepTime = now;
            }
            break;
    }
}

void CollisionDetector::updateLEDs(CollisionZone zone) {
    if (_motors == nullptr) return;
    
    static CollisionZone lastLedZone = ZONE_SAFE;
    static unsigned long lastBlinkTime = 0;
    static bool blinkState = false;
    
    // Aktualizuj LED tylko przy zmianie strefy lub pulsowaniu
    unsigned long now = millis();
    
    switch (zone) {
        case ZONE_SAFE:
            if (lastLedZone != ZONE_SAFE) {
                _motors->setLED(0, 0, 20, 0);   // Ciemny zielony
                _motors->setLED(1, 0, 20, 0);
            }
            break;
            
        case ZONE_WARNING:
            if (lastLedZone != ZONE_WARNING) {
                _motors->setLED(0, 30, 20, 0);  // Żółty
                _motors->setLED(1, 30, 20, 0);
            }
            break;
            
        case ZONE_CRITICAL:
            // Pulsujący czerwony
            if (now - lastBlinkTime >= 200) {
                blinkState = !blinkState;
                if (blinkState) {
                    _motors->setLED(0, 40, 0, 0);  // Jasny czerwony
                    _motors->setLED(1, 40, 0, 0);
                } else {
                    _motors->setLED(0, 5, 0, 0);   // Ciemny czerwony
                    _motors->setLED(1, 5, 0, 0);
                }
                lastBlinkTime = now;
            }
            break;
    }
    
    lastLedZone = zone;
}

const char* CollisionDetector::zoneToString(CollisionZone zone) {
    switch (zone) {
        case ZONE_SAFE:     return "SAFE";
        case ZONE_WARNING:  return "WARN";
        case ZONE_CRITICAL: return "STOP";
        default:            return "???";
    }
}

uint16_t CollisionDetector::zoneToColor(CollisionZone zone) {
    switch (zone) {
        case ZONE_SAFE:     return 0x07E0;  // Green
        case ZONE_WARNING:  return 0xFFE0;  // Yellow
        case ZONE_CRITICAL: return 0xF800;  // Red
        default:            return 0xFFFF;  // White
    }
}

unsigned long CollisionDetector::getTimeSinceLastCollision() {
    if (_lastCollisionTime == 0) return 0;
    return millis() - _lastCollisionTime;
}
