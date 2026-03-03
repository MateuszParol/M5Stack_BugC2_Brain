/**
 * bugc2_brain.ino — M5Stack BugC2 Brain
 * 
 * Główny firmware robota BugC2 działający na M5StickC Plus 2.
 * Phase 1: Foundation — kontrola silników, LCD HUD, diagnostyka I2C.
 * 
 * Hardware:
 * - M5StickC Plus 2 (ESP32-PICO-V3-02)
 * - BugC2 Motor Driver (I2C 0x38)
 * - ToF4M Distance Sensor (I2C 0x29) — Phase 2
 * - AtomS3R-CAM AI (I2C slave) — Phase 5
 * 
 * Sterowanie:
 * - BtnA: Przełączanie trybów / Start testu silników (długie naciśnięcie)
 * - BtnB: Emergency Stop
 * 
 * Wersja: 1.0.0-alpha (Phase 1)
 */

#include <M5StickCPlus2.h>
#include <Wire.h>
#include "config.h"
#include "motor_driver.h"
#include "display_manager.h"
#include "driving_mode.h"
#include "i2c_scanner.h"

// ============================================================
// Global Objects
// ============================================================
MotorDriver motors;
DisplayManager display;
I2CScanner i2cScanner;

// ============================================================
// State
// ============================================================
DrivingMode currentMode = MODE_MANUAL;
float batteryVoltage = 0.0f;
int currentDistance = -1;          // -1 = brak czujnika (Phase 2)
bool cameraConnected = false;     // Phase 5
unsigned long lastDisplayUpdate = 0;
unsigned long lastBatteryRead = 0;

// Motor Test State
enum TestPhase {
    TEST_FORWARD,
    TEST_STOP1,
    TEST_LEFT,
    TEST_STOP2,
    TEST_RIGHT,
    TEST_STOP3,
    TEST_BACKWARD,
    TEST_STOP4,
    TEST_SPIN_LEFT,
    TEST_STOP5,
    TEST_SPIN_RIGHT,
    TEST_STOP6,
    TEST_DONE
};
TestPhase currentTest = TEST_FORWARD;
unsigned long testTimer = 0;
bool testRunning = false;

// ============================================================
// Setup
// ============================================================
void setup() {
    // Inicjalizacja M5StickC Plus 2
    auto cfg = M5.config();
    M5.begin(cfg);
    
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("========================================");
    Serial.println("  BugC2 Brain v1.0.0-alpha (Phase 1)");
    Serial.println("========================================");
    
    // Inicjalizacja LCD — splash screen
    display.begin();
    delay(1500);  // Pokaż splash screen
    
    // Inicjalizacja I2C (Grove Port)
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);  // 100kHz — bezpieczna prędkość
    Serial.println("[I2C] Initialized on SDA=" + String(SDA_PIN) + " SCL=" + String(SCL_PIN));
    
    // Skanowanie I2C
    int deviceCount = i2cScanner.scan();
    i2cScanner.printResults();
    
    // Wyświetl wyniki na LCD
    display.clear();
    display.drawI2CStatus(i2cScanner.getFoundDevices(), deviceCount);
    
    // Inicjalizacja silników
    motors.begin();
    
    // Sprawdź kamerę (Phase 5 — na razie zawsze false)
    cameraConnected = i2cScanner.isDevicePresent(CAM_ADDR);
    
    // Odczyt baterii
    batteryVoltage = motors.getBatteryVoltage();
    lastBatteryRead = millis();
    
    // Wyświetl pełny HUD
    display.update(currentMode, batteryVoltage, currentDistance, 
                   cameraConnected, NULL);
    display.drawMessage("BtnA:Mode  Long:Test  B:Stop");
    
    Serial.println("[Setup] Complete!");
    Serial.printf("[Mode] Current: %s\n", modeToString(currentMode));
    Serial.println("[Controls] BtnA=Mode, LongPress=MotorTest, BtnB=Stop");
}

// ============================================================
// Motor Test Sequence
// ============================================================
void runMotorTest() {
    unsigned long elapsed = millis() - testTimer;
    
    switch (currentTest) {
        case TEST_FORWARD:
            if (elapsed < 50) {
                Serial.println("[Test] Forward...");
                display.drawMessage("TEST: Forward");
                motors.forward(SPEED_SLOW);
                motors.setLED(0, 0, 0, 30);
                motors.setLED(1, 0, 0, 30);
            }
            if (elapsed >= 2000) { currentTest = TEST_STOP1; testTimer = millis(); }
            break;
            
        case TEST_STOP1:
            if (elapsed < 50) { motors.stop(); display.drawMessage("TEST: Stop"); }
            if (elapsed >= 1000) { currentTest = TEST_LEFT; testTimer = millis(); }
            break;
            
        case TEST_LEFT:
            if (elapsed < 50) {
                Serial.println("[Test] Turn Left...");
                display.drawMessage("TEST: Turn Left");
                motors.turnLeft(SPEED_SLOW);
                motors.setLED(0, 30, 30, 0);
                motors.setLED(1, 0, 0, 0);
            }
            if (elapsed >= 1500) { currentTest = TEST_STOP2; testTimer = millis(); }
            break;
            
        case TEST_STOP2:
            if (elapsed < 50) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_RIGHT; testTimer = millis(); }
            break;
            
        case TEST_RIGHT:
            if (elapsed < 50) {
                Serial.println("[Test] Turn Right...");
                display.drawMessage("TEST: Turn Right");
                motors.turnRight(SPEED_SLOW);
                motors.setLED(0, 0, 0, 0);
                motors.setLED(1, 30, 30, 0);
            }
            if (elapsed >= 1500) { currentTest = TEST_STOP3; testTimer = millis(); }
            break;
            
        case TEST_STOP3:
            if (elapsed < 50) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_BACKWARD; testTimer = millis(); }
            break;
            
        case TEST_BACKWARD:
            if (elapsed < 50) {
                Serial.println("[Test] Backward...");
                display.drawMessage("TEST: Backward");
                motors.backward(SPEED_SLOW);
                motors.setLED(0, 30, 0, 30);
                motors.setLED(1, 30, 0, 30);
            }
            if (elapsed >= 2000) { currentTest = TEST_STOP4; testTimer = millis(); }
            break;
            
        case TEST_STOP4:
            if (elapsed < 50) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_SPIN_LEFT; testTimer = millis(); }
            break;
            
        case TEST_SPIN_LEFT:
            if (elapsed < 50) {
                Serial.println("[Test] Spin Left...");
                display.drawMessage("TEST: Spin Left");
                motors.spinLeft(SPEED_SLOW);
                motors.setLED(0, 0, 30, 30);
                motors.setLED(1, 0, 30, 30);
            }
            if (elapsed >= 1500) { currentTest = TEST_STOP5; testTimer = millis(); }
            break;
            
        case TEST_STOP5:
            if (elapsed < 50) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_SPIN_RIGHT; testTimer = millis(); }
            break;
            
        case TEST_SPIN_RIGHT:
            if (elapsed < 50) {
                Serial.println("[Test] Spin Right...");
                display.drawMessage("TEST: Spin Right");
                motors.spinRight(SPEED_SLOW);
            }
            if (elapsed >= 1500) { currentTest = TEST_STOP6; testTimer = millis(); }
            break;
            
        case TEST_STOP6:
            if (elapsed < 50) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_DONE; testTimer = millis(); }
            break;
            
        case TEST_DONE:
            Serial.println("[Test] Motor test COMPLETE!");
            motors.stop();
            motors.setLED(0, 0, 30, 0);
            motors.setLED(1, 0, 30, 0);
            display.drawMessage("TEST COMPLETE!");
            testRunning = false;
            currentTest = TEST_FORWARD;
            break;
    }
}

// ============================================================
// Main Loop
// ============================================================
void loop() {
    M5.update();  // Odczyt przycisków
    unsigned long now = millis();
    
    // ---- Obsługa przycisków ----
    
    // BtnA: krótkie naciśnięcie = zmiana trybu
    if (M5.BtnA.wasPressed() && !testRunning) {
        currentMode = nextMode(currentMode);
        Serial.printf("[Mode] Switched to: %s\n", modeToString(currentMode));
        display.drawMessage(modeToString(currentMode));
        
        // LED wskazujący tryb
        switch (currentMode) {
            case MODE_MANUAL:     motors.setLED(0, 0, 0, 30); motors.setLED(1, 0, 0, 30); break;
            case MODE_SEMI_AUTO:  motors.setLED(0, 30, 30, 0); motors.setLED(1, 30, 30, 0); break;
            case MODE_AUTONOMOUS: motors.setLED(0, 0, 30, 0); motors.setLED(1, 0, 30, 0); break;
            default: break;
        }
    }
    
    // BtnA: długie naciśnięcie (>1s) = start motor test
    if (M5.BtnA.pressedFor(1000) && !testRunning) {
        Serial.println("[Input] Long press BtnA — starting motor test");
        testRunning = true;
        currentTest = TEST_FORWARD;
        testTimer = millis();
    }
    
    // BtnB: Emergency Stop
    if (M5.BtnB.wasPressed()) {
        Serial.println("[Input] BtnB — EMERGENCY STOP");
        motors.stop();
        motors.setLED(0, 30, 0, 0);
        motors.setLED(1, 30, 0, 0);
        testRunning = false;
        display.drawMessage("EMERGENCY STOP!");
    }
    
    // ---- Motor Test ----
    if (testRunning) {
        runMotorTest();
    }
    
    // ---- Periodic Updates ----
    
    // Odczyt baterii co 5 sekund
    if (now - lastBatteryRead >= 5000) {
        batteryVoltage = motors.getBatteryVoltage();
        lastBatteryRead = now;
    }
    
    // Aktualizacja LCD
    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        display.update(currentMode, batteryVoltage, currentDistance,
                       cameraConnected, NULL);
        lastDisplayUpdate = now;
    }
    
    // TODO: Replace delay() with proper FreeRTOS task scheduling
    delay(10);
}
