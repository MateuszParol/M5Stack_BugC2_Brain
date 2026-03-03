/**
 * collision_detector.h — Collision Detection System
 * 
 * Trzy strefy bezpieczeństwa:
 * - ZONE_SAFE (>30cm): pełna prędkość
 * - ZONE_WARNING (15-30cm): zwolnij do 50%
 * - ZONE_CRITICAL (<15cm): STOP!
 * 
 * Zachowanie zależy od trybu jazdy:
 * - AUTONOMOUS: pełna kontrola (stop + auto-skręt w Phase 3)
 * - SEMI_AUTO: może TYLKO hamować, nigdy skręcać
 * - MANUAL: wyłączony (tylko wizualne ostrzeżenia)
 */

#ifndef COLLISION_DETECTOR_H
#define COLLISION_DETECTOR_H

#include <Arduino.h>
#include "config.h"
#include "tof_sensor.h"
#include "motor_driver.h"
#include "driving_mode.h"

enum CollisionZone {
    ZONE_SAFE = 0,      // > TOF_SLOW_CM  — pełna prędkość
    ZONE_WARNING = 1,   // TOF_STOP_CM..TOF_SLOW_CM — zwolnij
    ZONE_CRITICAL = 2   // < TOF_STOP_CM — STOP!
};

class CollisionDetector {
public:
    /**
     * Inicjalizacja z referencjami do czujnika i silników.
     */
    void begin(ToFSensor* tof, MotorDriver* motors);

    /**
     * Zaktualizuj stan kolizji na podstawie aktualnej odległości.
     * @param distanceCm   Odległość w cm (-1 = brak danych)
     * @param currentMode  Aktualny tryb jazdy
     * @return Aktualna strefa kolizji
     */
    CollisionZone update(int distanceCm, DrivingMode currentMode);

    /**
     * Czy system powinien nadpisać komendy silników?
     */
    bool shouldOverrideMotors() { return _overrideActive; }

    /**
     * Dostosuj żądaną prędkość do aktualnej strefy.
     * @param requestedSpeed  Żądana prędkość (1..127)
     * @return Skorygowana prędkość (0 przy CRITICAL)
     */
    int getAdjustedSpeed(int requestedSpeed);

    /**
     * Aktualna strefa kolizji.
     */
    CollisionZone getCurrentZone() { return _currentZone; }

    /**
     * Nazwa strefy jako string.
     */
    static const char* zoneToString(CollisionZone zone);

    /**
     * Kolor strefy (RGB565) do wyświetlania.
     */
    static uint16_t zoneToColor(CollisionZone zone);

    /**
     * Czas (ms) od ostatniego zdarzenia CRITICAL.
     */
    unsigned long getTimeSinceLastCollision();

    /**
     * Czy aktualnie trwa alarm kolizyjny?
     */
    bool isCollisionActive() { return _collisionActive; }

private:
    ToFSensor* _tof = nullptr;
    MotorDriver* _motors = nullptr;
    
    CollisionZone _currentZone = ZONE_SAFE;
    CollisionZone _previousZone = ZONE_SAFE;
    bool _collisionActive = false;
    bool _overrideActive = false;
    
    unsigned long _lastCollisionTime = 0;
    unsigned long _zoneChangeTime = 0;
    
    // Debounce: wymagaj bezpiecznej odległości przez 500ms przed odblokowaniem
    static const unsigned long DEBOUNCE_MS = 500;
    unsigned long _safeStartTime = 0;
    bool _waitingForDebounce = false;

    /**
     * Klasyfikuj odległość do strefy.
     */
    CollisionZone classifyDistance(int distanceCm);

    /**
     * Wykonaj reakcję na zmianę strefy.
     */
    void handleZoneChange(CollisionZone newZone, DrivingMode mode);

    /**
     * Obsłuż buzzer alarm.
     */
    void updateBuzzer(CollisionZone zone);

    /**
     * Obsłuż LED feedback.
     */
    void updateLEDs(CollisionZone zone);
};

#endif // COLLISION_DETECTOR_H
