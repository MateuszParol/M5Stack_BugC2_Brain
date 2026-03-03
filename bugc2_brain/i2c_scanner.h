/**
 * i2c_scanner.h — I2C Bus Scanner
 * 
 * Diagnostyka magistrali I2C — skanuje adresy 0x01-0x7F
 * i raportuje znalezione urządzenia.
 */

#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include <Arduino.h>
#include <Wire.h>

#define MAX_I2C_DEVICES 16

class I2CScanner {
public:
    /**
     * Skanuj magistralę I2C i znajdź podłączone urządzenia.
     * @return Ilość znalezionych urządzeń
     */
    int scan();

    /**
     * Sprawdź czy urządzenie jest dostępne na danym adresie.
     * @param address  Adres I2C (7-bit)
     * @return true jeśli urządzenie odpowiada
     */
    bool isDevicePresent(uint8_t address);

    /**
     * Wypisz wyniki skanowania na Serial.
     */
    void printResults();

    /**
     * Pobierz tablicę znalezionych adresów.
     */
    uint8_t* getFoundDevices() { return _foundDevices; }

    /**
     * Pobierz ilość znalezionych urządzeń.
     */
    int getDeviceCount() { return _deviceCount; }

private:
    uint8_t _foundDevices[MAX_I2C_DEVICES];
    int _deviceCount = 0;
};

#endif // I2C_SCANNER_H
