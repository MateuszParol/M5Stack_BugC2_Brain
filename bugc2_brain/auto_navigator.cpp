/**
 * auto_navigator.cpp — Autonomous Navigation Implementation
 * 
 * Algorytm losowej eksploracji:
 * 1. Jedź prosto (FORWARD) z prędkością dostosowaną do odległości
 * 2. Co 3-8s — losowy skręt eksploracyjny (TURNING)
 * 3. Przy przeszkodzie <15cm — cofnij (REVERSING) → losowy obrót (AVOIDING)
 * 4. Timeout 5s w AVOIDING → reset i kolejna próba
 * 
 * Brak delay() — czysta maszyna stanów na millis().
 */

#include "auto_navigator.h"

void AutoNavigator::begin(MotorDriver* motors, CollisionDetector* collision) {
    _motors = motors;
    _collision = collision;
    _active = false;
    _currentState = NAV_IDLE;
    
    // Seed random z szumem ADC
    randomSeed(analogRead(0) + millis());
    
    Serial.println("[Nav] Autonomous navigator initialized");
}

void AutoNavigator::setActive(bool active) {
    if (active == _active) return;
    
    _active = active;
    
    if (active) {
        Serial.println("[Nav] ACTIVATED — starting autonomous exploration");
        changeState(NAV_FORWARD);
        scheduleNextExplore();
    } else {
        Serial.println("[Nav] DEACTIVATED — stopping");
        if (_motors != nullptr) {
            _motors->stop();
        }
        changeState(NAV_IDLE);
    }
}

void AutoNavigator::update() {
    if (!_active || _motors == nullptr || _collision == nullptr) return;
    
    switch (_currentState) {
        case NAV_FORWARD:   handleForward();   break;
        case NAV_REVERSING: handleReversing(); break;
        case NAV_AVOIDING:  handleAvoiding();  break;
        case NAV_TURNING:   handleTurning();   break;
        case NAV_IDLE:      break;  // Nic nie rób
    }
}

// ============================================================
// State Handlers
// ============================================================

void AutoNavigator::handleForward() {
    CollisionZone zone = _collision->getCurrentZone();
    
    // CRITICAL → natychmiast cofaj
    if (zone == ZONE_CRITICAL) {
        Serial.println("[Nav] OBSTACLE! → Reversing");
        changeState(NAV_REVERSING);
        return;
    }
    
    // Jedź prosto z prędkością dostosowaną do strefy
    int speed = _collision->getAdjustedSpeed(SPEED_NORMAL);
    _motors->forward(speed);
    
    // Czas na losowy skręt eksploracyjny?
    if (millis() >= _nextExploreTime) {
        Serial.println("[Nav] Explore turn");
        changeState(NAV_TURNING);
        return;
    }
}

void AutoNavigator::handleReversing() {
    unsigned long elapsed = millis() - _stateStartTime;
    
    // Cofaj przez REVERSE_DURATION_MS
    if (elapsed < REVERSE_DURATION_MS) {
        _motors->backward(SPEED_SLOW);
    } else {
        // Koniec cofania → omijaj
        _motors->stop();
        _avoidAttempts = 0;
        // Losowy kierunek omijania
        _avoidDirection = random(0, 2) == 0;  // 50/50
        Serial.printf("[Nav] Reverse done → Avoiding (%s)\n", 
                      _avoidDirection ? "RIGHT" : "LEFT");
        changeState(NAV_AVOIDING);
    }
}

void AutoNavigator::handleAvoiding() {
    unsigned long elapsed = millis() - _stateStartTime;
    CollisionZone zone = _collision->getCurrentZone();
    
    // Timeout — zbyt długo w avoiding
    if (elapsed >= AVOID_TIMEOUT_MS) {
        _avoidAttempts++;
        
        if (_avoidAttempts >= MAX_AVOID_ATTEMPTS) {
            // Za dużo prób — zatrzymaj się, cofnij i spróbuj od nowa
            Serial.println("[Nav] STUCK! Max attempts reached → Reversing again");
            _motors->stop();
            _avoidAttempts = 0;
            changeState(NAV_REVERSING);
            return;
        }
        
        // Zmień kierunek i spróbuj ponownie
        _avoidDirection = !_avoidDirection;
        Serial.printf("[Nav] Avoid timeout → Trying %s (attempt %d)\n",
                      _avoidDirection ? "RIGHT" : "LEFT", _avoidAttempts + 1);
        _stateStartTime = millis();  // Reset timer
        return;
    }
    
    // Obracaj się w wybranym kierunku
    if (_avoidDirection) {
        _motors->spinRight(SPEED_SLOW);
    } else {
        _motors->spinLeft(SPEED_SLOW);
    }
    
    // Sprawdź czy droga wolna (po minimum 300ms obrotu)
    if (elapsed >= 300 && zone == ZONE_SAFE) {
        Serial.println("[Nav] Path clear → Forward");
        _motors->stop();
        changeState(NAV_FORWARD);
        scheduleNextExplore();
    }
}

void AutoNavigator::handleTurning() {
    unsigned long elapsed = millis() - _stateStartTime;
    CollisionZone zone = _collision->getCurrentZone();
    
    // Przeszkoda podczas skrętu → przerwij i omijaj
    if (zone == ZONE_CRITICAL) {
        Serial.println("[Nav] Obstacle during turn → Reversing");
        changeState(NAV_REVERSING);
        return;
    }
    
    // Skręcaj przez wylosowany czas
    if (elapsed < _stateDuration) {
        // Losowy kierunek skrętu (wybrany przy wejściu w stan)
        if (_avoidDirection) {
            _motors->turnRight(SPEED_SLOW);
        } else {
            _motors->turnLeft(SPEED_SLOW);
        }
    } else {
        // Koniec skrętu → jedź prosto
        _motors->stop();
        changeState(NAV_FORWARD);
        scheduleNextExplore();
    }
}

// ============================================================
// Utility
// ============================================================

void AutoNavigator::changeState(NavState newState) {
    _previousState = _currentState;
    _currentState = newState;
    _stateStartTime = millis();
    
    // Ustaw parametry per-stan
    switch (newState) {
        case NAV_TURNING:
            // Losowy czas skrętu: 300-1000ms
            _stateDuration = random(300, 1001);
            // Losowy kierunek
            _avoidDirection = random(0, 2) == 0;
            break;
            
        case NAV_AVOIDING:
            _stateDuration = AVOID_TIMEOUT_MS;
            break;
            
        case NAV_REVERSING:
            _stateDuration = REVERSE_DURATION_MS;
            break;
            
        default:
            _stateDuration = 0;
            break;
    }
    
    // Log zmianę stanu (throttled)
    unsigned long now = millis();
    if (now - _lastLogTime >= 200) {
        Serial.printf("[Nav] %s → %s\n", 
                      stateToString(_previousState),
                      stateToString(newState));
        _lastLogTime = now;
    }
}

void AutoNavigator::scheduleNextExplore() {
    // Losowy czas do następnego skrętu: 3-8 sekund
    _nextExploreTime = millis() + random(3000, 8001);
}

const char* AutoNavigator::stateToString(NavState state) {
    switch (state) {
        case NAV_IDLE:      return "IDLE";
        case NAV_FORWARD:   return "FORWARD";
        case NAV_AVOIDING:  return "AVOIDING";
        case NAV_TURNING:   return "EXPLORING";
        case NAV_REVERSING: return "REVERSING";
        default:            return "???";
    }
}
