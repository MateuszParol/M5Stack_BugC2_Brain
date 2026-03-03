/**
 * i2c_scanner.cpp — I2C Bus Scanner Implementation
 * 
 * Skanuje adresy I2C 0x01-0x7F i raportuje znalezione urządzenia.
 * Identyfikuje znane urządzenia M5Stack po adresie.
 */

#include "i2c_scanner.h"
#include "config.h"

int I2CScanner::scan() {
    _deviceCount = 0;
    Serial.println("\n[I2C Scanner] Scanning bus...");
    
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            if (_deviceCount < MAX_I2C_DEVICES) {
                _foundDevices[_deviceCount] = addr;
                _deviceCount++;
            }
        }
    }
    
    Serial.printf("[I2C Scanner] Found %d device(s)\n", _deviceCount);
    return _deviceCount;
}

bool I2CScanner::isDevicePresent(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

void I2CScanner::printResults() {
    Serial.println("┌──────────────────────────────────┐");
    Serial.println("│       I2C Bus Scan Results        │");
    Serial.println("├──────────┬───────────────────────┤");
    Serial.println("│ Address  │ Device                │");
    Serial.println("├──────────┼───────────────────────┤");
    
    for (int i = 0; i < _deviceCount; i++) {
        uint8_t addr = _foundDevices[i];
        const char* name = "Unknown";
        
        // Identyfikuj znane urządzenia
        if (addr == BUGC2_ADDR)  name = "BugC2 Motor Driver";
        else if (addr == TOF4M_ADDR) name = "ToF4M (VL53L1X)";
        else if (addr == CAM_ADDR)   name = "AtomS3R-CAM AI";
        else if (addr == 0x68 || addr == 0x69) name = "MPU6886 (IMU)";
        else if (addr == 0x34) name = "AXP2101 (Power)";
        else if (addr == 0x51) name = "BM8563 (RTC)";
        
        Serial.printf("│  0x%02X    │ %-21s │\n", addr, name);
    }
    
    Serial.println("└──────────┴───────────────────────┘");
    
    // Sprawdź brakujące urządzenia
    Serial.println("\n[I2C Scanner] Expected devices:");
    Serial.printf("  BugC2 (0x%02X): %s\n", BUGC2_ADDR, 
                  isDevicePresent(BUGC2_ADDR) ? "OK ✓" : "MISSING ✗");
    Serial.printf("  ToF4M (0x%02X): %s\n", TOF4M_ADDR,
                  isDevicePresent(TOF4M_ADDR) ? "OK ✓" : "NOT CONNECTED (Phase 2)");
    Serial.printf("  CAM   (0x%02X): %s\n", CAM_ADDR,
                  isDevicePresent(CAM_ADDR) ? "OK ✓" : "NOT CONNECTED (Phase 5)");
}
