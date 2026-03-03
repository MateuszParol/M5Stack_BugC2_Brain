/**
 * vision_sensor.h — Vision Sensor I2C Master (Phase 5)
 * 
 * Odpytuje podłączoną na magistrali I2C kamerę AtomS3R-CAM (adres 0x40).
 * Zwraca status detekcji przeszkód na podstawie mockowanego sygnału z kamery.
 */

#ifndef VISION_SENSOR_H
#define VISION_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

class VisionSensor {
public:
    VisionSensor();

    /**
     * Sprawdza, czy sensor wizyjny jest dostępny na magistrali.
     */
    bool begin();

    /**
     * Odpytuje Slave (AtomS3R-CAM) o aktualny status.
     * Powinno być wywoływane w głównej pętli co np. 200ms.
     */
    void update();

    /**
     * Zwraca true, jeśli pomyślnie połączono z sensorem.
     */
    bool isConnected() const { return _connected; }

    /**
     * Zwraca true, czy wykryto obiekty traktowane jako przeszkody.
     */
    bool isObstacleDetected() const { return _obstacleDetected; }

    /**
     * Zwraca ciąg znaków gotowy do pokazania na HUD (np. "Clear" albo "Obstacle").
     */
    const char* getLastDetectionName() const { return _lastName; }


private:
    bool _connected;
    bool _obstacleDetected;
    const char* _lastName;
    unsigned long _lastReadTime;

    void requestData();
};

#endif // VISION_SENSOR_H
