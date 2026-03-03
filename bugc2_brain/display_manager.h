/**
 * display_manager.h — LCD Display Manager
 * 
 * Zarządza wyświetlaniem HUD na ekranie M5StickC Plus 2 (135x240 TFT).
 * Wyświetla: tryb jazdy, bateria, odległość ToF, status kamery AI.
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <M5StickCPlus2.h>
#include "config.h"
#include "driving_mode.h"

class DisplayManager {
public:
    /**
     * Inicjalizacja wyświetlacza.
     * Ustawia orientację landscape i wyświetla splash screen.
     */
    void begin();

    /**
     * Wyczyść cały ekran.
     */
    void clear();

    /**
     * Wyświetl splash screen przy uruchomieniu.
     */
    void drawSplash();

    /**
     * Narysuj pasek statusu (góra ekranu): tryb + bateria.
     */
    void drawStatusBar(DrivingMode mode, float batteryVoltage);

    /**
     * Narysuj pasek odległości z kolorami (zielony/żółty/czerwony).
     * @param distanceCm  Odległość w cm (-1 = brak odczytu)
     */
    void drawDistanceBar(int distanceCm);

    /**
     * Narysuj status kamery AI.
     * @param connected       Czy kamera jest podłączona
     * @param lastDetection   Ostatnia detekcja (NULL = brak)
     */
    void drawCameraStatus(bool connected, const char* lastDetection);

    /**
     * Wyświetl wyniki skanowania I2C na ekranie.
     * @param devices  Tablica znalezionych adresów
     * @param count    Ilość znalezionych urządzeń
     */
    void drawI2CStatus(uint8_t* devices, int count);

    /**
     * Wyświetl wiadomość na środku ekranu.
     */
    void drawMessage(const char* msg);

    /**
     * Pełna aktualizacja HUD — wywoływana co DISPLAY_UPDATE_INTERVAL ms.
     */
    void update(DrivingMode mode, float battery, int distance, 
                bool camConnected, const char* camDetection);

private:
    DrivingMode _lastMode = MODE_MANUAL;
    float _lastBattery = 0.0f;
    int _lastDistance = -1;
    bool _lastCamConnected = false;
    bool _needsFullRedraw = true;

    /**
     * Zwraca kolor RGB565 dla poziomu baterii.
     */
    uint16_t batteryColor(float voltage);

    /**
     * Zwraca kolor RGB565 dla odległości.
     */
    uint16_t distanceColor(int distanceCm);
};

#endif // DISPLAY_MANAGER_H
