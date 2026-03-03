/**
 * vision_sensor.cpp — Vision Sensor Implementation (Phase 5)
 */

#include "vision_sensor.h"

#define VISION_READ_INTERVAL 300 // Odpytywanie co 300ms

VisionSensor::VisionSensor() {
    _connected = false;
    _obstacleDetected = false;
    _lastName = "--";
    _lastReadTime = 0;
}

bool VisionSensor::begin() {
    Wire.beginTransmission(CAM_ADDR);
    byte error = Wire.endTransmission();
    
    _connected = (error == 0);
    if (_connected) {
        Serial.println("[Vision] AtomS3R-CAM found at 0x40");
        _lastName = "Clear";
    } else {
        Serial.println("[Vision] AtomS3R-CAM NOT FOUND");
        _lastName = "Offline";
    }
    return _connected;
}

void VisionSensor::update() {
    if (!_connected) return;

    unsigned long now = millis();
    if (now - _lastReadTime >= VISION_READ_INTERVAL) {
        requestData();
        _lastReadTime = now;
    }
}

void VisionSensor::requestData() {
    // Prosi kamerę o 1 bajt
    uint8_t bytesReceived = Wire.requestFrom((uint8_t)CAM_ADDR, (size_t)1);
    
    if (bytesReceived == 1) {
        uint8_t status = Wire.read();
        
        if (status == 1) {
            _obstacleDetected = true;
            _lastName = "Obstacle!";
        } else {
            _obstacleDetected = false;
            _lastName = "Clear";
        }
    } else {
        // Utrata połączenia!
        _connected = false;
        _obstacleDetected = false;
        _lastName = "Offline";
        Serial.println("[Vision] Lost connection to camera!");
    }
}
