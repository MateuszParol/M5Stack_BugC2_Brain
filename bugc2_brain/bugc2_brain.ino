/**
 * bugc2_brain.ino — M5Stack BugC2 Brain
 * 
 * Główny firmware robota BugC2 działający na M5StickC Plus 2.
 * Phase 1: Foundation — bazowa kontrola silników i struktura projektu.
 * 
 * Hardware:
 * - M5StickC Plus 2 (ESP32-PICO-V3-02)
 * - BugC2 Motor Driver (I2C 0x38)
 * - ToF4M Distance Sensor (I2C 0x29) — Phase 2
 * - AtomS3R-CAM AI (I2C slave) — Phase 5
 * 
 * Autor: M5Stack BugC2 Brain Project
 * Wersja: 1.0.0-alpha (Phase 1)
 */

#include <M5StickCPlus2.h>
#include <Wire.h>
#include "config.h"
#include "motor_driver.h"

// ============================================================
// Global Objects
// ============================================================
MotorDriver motors;

// ============================================================
// Test State
// ============================================================
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
    
    // Inicjalizacja I2C (Grove Port)
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);  // 100kHz — bezpieczna prędkość dla BugC2
    Serial.println("[I2C] Initialized on SDA=" + String(SDA_PIN) + " SCL=" + String(SCL_PIN));
    
    // Inicjalizacja silników
    motors.begin();
    
    // LCD — splash screen
    M5.Lcd.setRotation(1);  // Landscape (robot jedzie "od ekranu")
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 20);
    M5.Lcd.println("BugC2 Brain");
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.println("v1.0.0-alpha (Phase 1)");
    
    // Status BugC2
    M5.Lcd.setCursor(10, 70);
    if (motors.isConnected()) {
        M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Lcd.println("BugC2: CONNECTED");
        // Zielone LED na potwierdzenie
        motors.setLED(0, 0, 30, 0);
        motors.setLED(1, 0, 30, 0);
    } else {
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.println("BugC2: NOT FOUND!");
    }
    
    // Odczyt baterii
    float voltage = motors.getBatteryVoltage();
    M5.Lcd.setCursor(10, 90);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.printf("Battery: %.2fV", voltage);
    Serial.printf("[Battery] Voltage: %.2fV\n", voltage);
    
    // Instrukcje
    M5.Lcd.setCursor(10, 115);
    M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Lcd.println("BtnA: Start motor test");
    
    Serial.println("[Setup] Complete. Press BtnA to start motor test.");
}

// ============================================================
// Motor Test Sequence
// ============================================================
void runMotorTest() {
    unsigned long elapsed = millis() - testTimer;
    
    switch (currentTest) {
        case TEST_FORWARD:
            if (elapsed == 0) {
                Serial.println("[Test] Forward...");
                M5.Lcd.fillRect(10, 115, 220, 15, TFT_BLACK);
                M5.Lcd.setCursor(10, 115);
                M5.Lcd.setTextColor(TFT_CYAN, TFT_BLACK);
                M5.Lcd.println(">> FORWARD");
                motors.forward(SPEED_SLOW);
                motors.setLED(0, 0, 0, 30);  // Blue = forward
                motors.setLED(1, 0, 0, 30);
            }
            if (elapsed >= 2000) { currentTest = TEST_STOP1; testTimer = millis(); }
            break;
            
        case TEST_STOP1:
            if (elapsed == 0) {
                Serial.println("[Test] Stop 1...");
                motors.stop();
                motors.setLED(0, 30, 0, 0);  // Red = stop
                motors.setLED(1, 30, 0, 0);
                M5.Lcd.fillRect(10, 115, 220, 15, TFT_BLACK);
                M5.Lcd.setCursor(10, 115);
                M5.Lcd.println(">> STOP");
            }
            if (elapsed >= 1000) { currentTest = TEST_LEFT; testTimer = millis(); }
            break;
            
        case TEST_LEFT:
            if (elapsed == 0) {
                Serial.println("[Test] Turn Left...");
                motors.turnLeft(SPEED_SLOW);
                motors.setLED(0, 30, 30, 0);  // Yellow = turning
                motors.setLED(1, 0, 0, 0);
                M5.Lcd.fillRect(10, 115, 220, 15, TFT_BLACK);
                M5.Lcd.setCursor(10, 115);
                M5.Lcd.println(">> TURN LEFT");
            }
            if (elapsed >= 1500) { currentTest = TEST_STOP2; testTimer = millis(); }
            break;
            
        case TEST_STOP2:
            if (elapsed == 0) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_RIGHT; testTimer = millis(); }
            break;
            
        case TEST_RIGHT:
            if (elapsed == 0) {
                Serial.println("[Test] Turn Right...");
                motors.turnRight(SPEED_SLOW);
                motors.setLED(0, 0, 0, 0);
                motors.setLED(1, 30, 30, 0);
                M5.Lcd.fillRect(10, 115, 220, 15, TFT_BLACK);
                M5.Lcd.setCursor(10, 115);
                M5.Lcd.println(">> TURN RIGHT");
            }
            if (elapsed >= 1500) { currentTest = TEST_STOP3; testTimer = millis(); }
            break;
            
        case TEST_STOP3:
            if (elapsed == 0) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_BACKWARD; testTimer = millis(); }
            break;
            
        case TEST_BACKWARD:
            if (elapsed == 0) {
                Serial.println("[Test] Backward...");
                motors.backward(SPEED_SLOW);
                motors.setLED(0, 30, 0, 30);  // Purple = backward
                motors.setLED(1, 30, 0, 30);
                M5.Lcd.fillRect(10, 115, 220, 15, TFT_BLACK);
                M5.Lcd.setCursor(10, 115);
                M5.Lcd.println(">> BACKWARD");
            }
            if (elapsed >= 2000) { currentTest = TEST_STOP4; testTimer = millis(); }
            break;
            
        case TEST_STOP4:
            if (elapsed == 0) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_SPIN_LEFT; testTimer = millis(); }
            break;
            
        case TEST_SPIN_LEFT:
            if (elapsed == 0) {
                Serial.println("[Test] Spin Left...");
                motors.spinLeft(SPEED_SLOW);
                motors.setLED(0, 0, 30, 30);
                motors.setLED(1, 0, 30, 30);
                M5.Lcd.fillRect(10, 115, 220, 15, TFT_BLACK);
                M5.Lcd.setCursor(10, 115);
                M5.Lcd.println(">> SPIN LEFT");
            }
            if (elapsed >= 1500) { currentTest = TEST_STOP5; testTimer = millis(); }
            break;
            
        case TEST_STOP5:
            if (elapsed == 0) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_SPIN_RIGHT; testTimer = millis(); }
            break;
            
        case TEST_SPIN_RIGHT:
            if (elapsed == 0) {
                Serial.println("[Test] Spin Right...");
                motors.spinRight(SPEED_SLOW);
                M5.Lcd.fillRect(10, 115, 220, 15, TFT_BLACK);
                M5.Lcd.setCursor(10, 115);
                M5.Lcd.println(">> SPIN RIGHT");
            }
            if (elapsed >= 1500) { currentTest = TEST_STOP6; testTimer = millis(); }
            break;
            
        case TEST_STOP6:
            if (elapsed == 0) { motors.stop(); }
            if (elapsed >= 1000) { currentTest = TEST_DONE; testTimer = millis(); }
            break;
            
        case TEST_DONE:
            Serial.println("[Test] Motor test COMPLETE!");
            motors.stop();
            motors.setLED(0, 0, 30, 0);  // Green = done
            motors.setLED(1, 0, 30, 0);
            M5.Lcd.fillRect(10, 115, 220, 15, TFT_BLACK);
            M5.Lcd.setCursor(10, 115);
            M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
            M5.Lcd.println("TEST COMPLETE!");
            testRunning = false;
            currentTest = TEST_FORWARD;  // Reset for next run
            break;
    }
}

// ============================================================
// Main Loop
// ============================================================
void loop() {
    M5.update();  // Odczyt przycisków
    
    // BtnA — start/restart motor test
    if (M5.BtnA.wasPressed()) {
        Serial.println("[Input] BtnA pressed — starting motor test");
        testRunning = true;
        currentTest = TEST_FORWARD;
        testTimer = millis();
    }
    
    // BtnB — emergency stop
    if (M5.BtnB.wasPressed()) {
        Serial.println("[Input] BtnB pressed — EMERGENCY STOP");
        motors.stop();
        motors.setLED(0, 30, 0, 0);
        motors.setLED(1, 30, 0, 0);
        testRunning = false;
        M5.Lcd.fillRect(10, 115, 220, 15, TFT_BLACK);
        M5.Lcd.setCursor(10, 115);
        M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
        M5.Lcd.println("EMERGENCY STOP!");
    }
    
    // Uruchom test silników jeśli aktywny
    if (testRunning) {
        runMotorTest();
    }
    
    // TODO: Replace delay() with proper FreeRTOS task scheduling
    delay(10);
}
