/**
 * tof_sensor.h — ToF4M Distance Sensor Driver
 * 
 * Sterownik czujnika VL53L1X (ToF4M) na magistrali I2C (0x29).
 * Obsługuje: ciągły odczyt odległości, filtrowanie szumów,
 * wykrywanie przeszkód z progami odległości.
 * 
 * Wymaga biblioteki: VL53L1X by Pololu
 */

#ifndef TOF_SENSOR_H
#define TOF_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <VL53L1X.h>
#include "config.h"

// Moving average filter size
#define TOF_FILTER_SIZE  3

class ToFSensor {
public:
    /**
     * Inicjalizacja czujnika VL53L1X.
     * @return true jeśli inicjalizacja się powiodła
     */
    bool begin();

    /**
     * Odczytaj odległość z czujnika.
     * Stosuje filtr średniej kroczącej.
     * @return Odległość w cm, lub -1 przy błędzie/timeout
     */
    int readDistance();

    /**
     * @return true jeśli odległość <= TOF_STOP_CM (15cm)
     */
    bool isObstacleClose();

    /**
     * @return true jeśli odległość <= TOF_SLOW_CM (30cm)
     */
    bool isObstacleNear();

    /**
     * Sprawdź czy czujnik jest podłączony do I2C.
     */
    bool isConnected();

    /**
     * Zwróć ostatni poprawny odczyt (bez ponownego pomiaru).
     * @return Odległość w cm, lub -1 jeśli brak odczytu
     */
    int getLastDistance() { return _lastDistance; }

    /**
     * Czy czujnik został poprawnie zainicjalizowany?
     */
    bool isInitialized() { return _initialized; }

private:
    VL53L1X _sensor;
    bool _initialized = false;
    int _lastDistance = -1;
    unsigned long _lastReadTime = 0;

    // Moving average filter
    int _filterBuffer[TOF_FILTER_SIZE];
    int _filterIndex = 0;
    int _filterCount = 0;  // Ilość zebranych próbek (do wypełnienia bufora)

    /**
     * Dodaj wartość do filtra i zwróć średnią.
     */
    int applyFilter(int rawValue);
};

#endif // TOF_SENSOR_H
