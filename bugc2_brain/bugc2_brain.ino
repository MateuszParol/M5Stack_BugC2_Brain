/**
 * bugc2_brain.ino — M5Stack BugC2 Brain
 * 
 * Główny firmware robota BugC2 działający na M5StickC Plus 2.
 * Phase 4: Remote Control — Sterowanie po WiFi i WebServer.
 * 
 * Hardware:
 * - M5StickC Plus 2 (ESP32-PICO-V3-02)
 * - BugC2 Motor Driver (I2C 0x38)
 * - ToF4M Distance Sensor (I2C 0x29)
 * - AtomS3R-CAM AI (I2C slave) — Phase 5
 * 
 * Sterowanie:
 * - BtnA: Przełączanie trybów / Start testu silników (długie naciśnięcie)
 * - BtnB: Emergency Stop
 * - WiFi (192.168.4.1): Panel sterowania na telefonie
 * 
 * Wersja: 1.3.0-alpha (Phase 4)
 */

#include <M5StickCPlus2.h>
#include <Wire.h>
#include "config.h"
#include "motor_driver.h"
#include "display_manager.h"
#include "driving_mode.h"
#include "i2c_scanner.h"
#include "tof_sensor.h"
#include "collision_detector.h"
#include "auto_navigator.h"
#include "remote_controller.h"
#include "vision_sensor.h"

// ============================================================
// Global Objects
// ============================================================
MotorDriver motors;
DisplayManager display;
I2CScanner i2cScanner;
ToFSensor tofSensor;
CollisionDetector collisionDetector;
AutoNavigator navigator;
RemoteController remoteCtrl;
VisionSensor visionSensor;

// ============================================================
// State
// ============================================================
DrivingMode currentMode = MODE_MANUAL;
float batteryVoltage = 0.0f;
int currentDistance = -1;
bool cameraConnected = false;     // Phase 5
unsigned long lastDisplayUpdate = 0;
unsigned long lastBatteryRead = 0;
unsigned long lastToFRead = 0;
unsigned long lastSerialLog = 0;

// ============================================================
// Setup
// ============================================================
void setup() {
    // Inicjalizacja M5StickC Plus 2
    auto cfg = M5.config();
    M5.begin(cfg);
    
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("========================================");
    Serial.println("  BugC2 Brain v1.1.0-alpha (Phase 2)");
    Serial.println("========================================");
    
    // Inicjalizacja LCD — splash screen
    display.begin();
    delay(1500);
    
    // Inicjalizacja I2C (Grove Port)
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);
    Serial.println("[I2C] Initialized on SDA=" + String(SDA_PIN) + " SCL=" + String(SCL_PIN));
    
    // Skanowanie I2C
    int deviceCount = i2cScanner.scan();
    i2cScanner.printResults();
    
    // Wyświetl wyniki na LCD
    display.clear();
    display.drawI2CStatus(i2cScanner.getFoundDevices(), deviceCount);
    delay(2000);
    
    // Inicjalizacja silników
    motors.begin();
    
    // Inicjalizacja czujnika ToF4M
    bool tofOk = tofSensor.begin();
    Serial.printf("[ToF] Status: %s\n", tofOk ? "OK" : "NOT AVAILABLE");
    
    // Inicjalizacja detektora kolizji
    collisionDetector.begin(&tofSensor, &motors);
    
    // Inicjalizacja nawigatora (Phase 3)
    navigator.begin(&motors, &collisionDetector);
    
    // Inicjalizacja Remote Controller (Phase 4)
    remoteCtrl.begin();
    
    // Sprawdź i zainicjalizuj kamerę (Phase 5)
    visionSensor.begin();
    cameraConnected = visionSensor.isConnected();
    
    // Odczyt baterii
    batteryVoltage = motors.getBatteryVoltage();
    lastBatteryRead = millis();
    
    // Wyświetl pełny HUD
    display.clear();
    display.update(currentMode, batteryVoltage, currentDistance, 
                   cameraConnected, visionSensor.getLastDetectionName(), remoteCtrl.getIPAddress());
    display.drawMessage("Ready. IP: " + remoteCtrl.getIPAddress());
    
    Serial.println("[Setup] Complete!");
    Serial.printf("[Mode] Current: %s\n", modeToString(currentMode));
    Serial.printf("[ToF] Sensor: %s\n", tofSensor.isInitialized() ? "ACTIVE" : "INACTIVE");
    Serial.println("[Controls] BtnA=Mode, BtnB=Stop");
}


// ============================================================
// Main Loop
// ============================================================
void loop() {
    M5.update();  // Odczyt przycisków
    unsigned long now = millis();
    
    // Obsługa zapytań WiFi i WebServera
    remoteCtrl.handleClient();
    
    // Obsługa systemu wizyjnego (I2C)
    visionSensor.update();
    cameraConnected = visionSensor.isConnected();
    
    // ---- Odczyt ToF co TOF_READ_INTERVAL ----
    if (tofSensor.isInitialized() && now - lastToFRead >= TOF_READ_INTERVAL) {
        currentDistance = tofSensor.readDistance();
        lastToFRead = now;
        
        // Aktualizuj detektor kolizji
        CollisionZone zone = collisionDetector.update(currentDistance, currentMode);
        
        // Serial log co 500ms (nie zaśmiecaj)
        if (now - lastSerialLog >= 500) {
            Serial.printf("[ToF] %dcm | Zone: %s | Override: %s | Vision: %s\n",
                          currentDistance,
                          CollisionDetector::zoneToString(zone),
                          collisionDetector.shouldOverrideMotors() ? "YES" : "no",
                          visionSensor.getLastDetectionName());
            lastSerialLog = now;
        }
    }
    
    // Zatrzymaj natychmiast, jeśli system wizyjny zgłasza przeszkodę
    // w trybie półautonomicznym lub autonomicznym.
    // Aby to zrobić czysto, dodajemy prosty if i override:
    bool visionOverride = visionSensor.isObstacleDetected() && currentMode != MODE_MANUAL;
    if (visionOverride) {
        motors.stop();
        display.drawMessage("VISION STOP!");
        if (navigator.isActive() && navigator.getState() == AutoNavigator::NAV_FORWARD) {
            // Wymuś uniki nawigatorowi (w przyszłości wizja powinna sterować nim wprost)
            motors.backward(SPEED_NORMAL);
            delay(300);
            motors.spinLeft(SPEED_NORMAL);
            delay(400);
            motors.stop();
        }
    }
    
    // ---- Obsługa przycisków ----
    
    // BtnA: krótkie naciśnięcie = zmiana trybu
    if (M5.BtnA.wasPressed()) {
        currentMode = nextMode(currentMode);
        Serial.printf("[Mode] Switched to: %s\n", modeToString(currentMode));
        display.drawMessage(modeToString(currentMode));
        
        // Aktywuj/deaktywuj nawigację
        navigator.setActive(currentMode == MODE_AUTONOMOUS);
        
        // LED wskazujący tryb (tylko jeśli nie w trybie kolizji)
        if (!collisionDetector.isCollisionActive()) {
            switch (currentMode) {
                case MODE_MANUAL:     motors.setLED(0, 0, 0, 30); motors.setLED(1, 0, 0, 30); break;
                case MODE_SEMI_AUTO:  motors.setLED(0, 30, 30, 0); motors.setLED(1, 30, 30, 0); break;
                case MODE_AUTONOMOUS: motors.setLED(0, 0, 30, 0); motors.setLED(1, 0, 30, 0); break;
                default: break;
            }
        }
    }
    
    // BtnB: Emergency Stop
    if (M5.BtnB.wasPressed()) {
        Serial.println("[Input] BtnB — EMERGENCY STOP");
        motors.stop();
        motors.setLED(0, 30, 0, 0);
        motors.setLED(1, 30, 0, 0);
        navigator.setActive(false);
        currentMode = MODE_MANUAL;
        display.drawMessage("EMERGENCY STOP (MANUAL)");
    }
    
    // ---- Obsługa poleceń z WiFi ----
    int requestedMode = remoteCtrl.getRequestedMode();
    if (requestedMode >= 0 && requestedMode <= 2) {
        currentMode = (DrivingMode)requestedMode;
        Serial.printf("[Mode] Switched via WiFi to: %s\n", modeToString(currentMode));
        display.drawMessage(modeToString(currentMode));
        
        navigator.setActive(currentMode == MODE_AUTONOMOUS);
        
        if (!collisionDetector.isCollisionActive()) {
            switch (currentMode) {
                case MODE_MANUAL:     motors.setLED(0, 0, 0, 30); motors.setLED(1, 0, 0, 30); break;
                case MODE_SEMI_AUTO:  motors.setLED(0, 30, 30, 0); motors.setLED(1, 30, 30, 0); break;
                case MODE_AUTONOMOUS: motors.setLED(0, 0, 30, 0); motors.setLED(1, 0, 30, 0); break;
                default: break;
            }
        }
    }
    
    String reqCmd = remoteCtrl.getRequestedCommand();
    if (reqCmd != "" && currentMode != MODE_AUTONOMOUS) {
        bool blockForward = (currentMode == MODE_SEMI_AUTO && (collisionDetector.shouldOverrideMotors() || visionSensor.isObstacleDetected()));
        
        if (reqCmd == "S" || (reqCmd == "F" && blockForward)) {
            motors.stop();
            if (reqCmd == "F" && blockForward) Serial.println("[Nav] WebCmd F blocked by Collision/Vision");
        } else if (reqCmd == "F") {
            motors.forward(SPEED_NORMAL);
        } else if (reqCmd == "B") {
            motors.backward(SPEED_NORMAL);
        } else if (reqCmd == "L") {
            motors.turnLeft(SPEED_NORMAL);
        } else if (reqCmd == "R") {
            motors.turnRight(SPEED_NORMAL);
        }
    }
    remoteCtrl.clearRequests();
    
    // ---- Autonomous Navigator Update ----
    if (navigator.isActive()) {
        navigator.update();
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
                       cameraConnected, visionSensor.getLastDetectionName(), remoteCtrl.getIPAddress());
                       
        // Pokaż stan nawigatora (nadpisz wiadomości jeśli aktywny)
        if (navigator.isActive()) {
            char navMsg[32];
            snprintf(navMsg, sizeof(navMsg), "AUTO: %s", navigator.stateToString(navigator.getState()));
            display.drawMessage(navMsg);
        }
        
        lastDisplayUpdate = now;
    }
    
    // TODO: Replace delay() with proper FreeRTOS task scheduling
    delay(10);
}
