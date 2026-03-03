/**
 * auto_navigator.h — Autonomous Navigation Engine
 * 
 * Maszyna stanów do autonomicznej jazdy:
 * - IDLE: navigator nieaktywny
 * - FORWARD: jazda prosto + losowe skręty eksploracyjne
 * - REVERSING: cofanie po kolizji (500ms)
 * - AVOIDING: losowy obrót by ominąć przeszkodę
 * - TURNING: losowy skręt eksploracyjny
 */

#ifndef AUTO_NAVIGATOR_H
#define AUTO_NAVIGATOR_H

#include <Arduino.h>
#include "config.h"
#include "motor_driver.h"
#include "collision_detector.h"

enum NavState {
    NAV_IDLE,           // Navigator wyłączony
    NAV_FORWARD,        // Jazda prosto
    NAV_AVOIDING,       // Omijanie przeszkody (spin)
    NAV_TURNING,        // Losowy skręt eksploracyjny
    NAV_REVERSING       // Cofanie po kolizji
};

class AutoNavigator {
public:
    /**
     * Inicjalizacja z referencjami do silników i detektora kolizji.
     */
    void begin(MotorDriver* motors, CollisionDetector* collision);

    /**
     * Włącz/wyłącz nawigację autonomiczną.
     * setActive(false) natychmiast zatrzymuje silniki.
     */
    void setActive(bool active);

    /**
     * Czy navigator jest aktywny?
     */
    bool isActive() { return _active; }

    /**
     * Główna pętla decyzyjna — wywoływana co loop cycle.
     * Steruje silnikami w trybie autonomicznym.
     */
    void update();

    /**
     * Aktualny stan nawigacji.
     */
    NavState getState() { return _currentState; }

    /**
     * Nazwa stanu jako string.
     */
    static const char* stateToString(NavState state);

private:
    MotorDriver* _motors = nullptr;
    CollisionDetector* _collision = nullptr;
    
    bool _active = false;
    NavState _currentState = NAV_IDLE;
    NavState _previousState = NAV_IDLE;
    
    // Timing
    unsigned long _stateStartTime = 0;   // Kiedy wszedł w aktualny stan
    unsigned long _stateDuration = 0;    // Ile ms ma trwać stan (dla TURNING, AVOIDING, REVERSING)
    unsigned long _nextExploreTime = 0;  // Kiedy wykonać losowy skręt
    unsigned long _lastLogTime = 0;      // Throttle serial logs
    
    // Avoiding state
    bool _avoidDirection = false;         // false=left, true=right
    int _avoidAttempts = 0;               // Ile prób uniknięcia
    static const int MAX_AVOID_ATTEMPTS = 3;
    static const unsigned long AVOID_TIMEOUT_MS = 5000;
    static const unsigned long REVERSE_DURATION_MS = 500;

    /**
     * Przejdź do nowego stanu.
     */
    void changeState(NavState newState);

    /**
     * Oblicz losowy czas do następnego skrętu eksploracyjnego (3-8s).
     */
    void scheduleNextExplore();

    // Handlery stanów
    void handleForward();
    void handleReversing();
    void handleAvoiding();
    void handleTurning();
};

#endif // AUTO_NAVIGATOR_H
