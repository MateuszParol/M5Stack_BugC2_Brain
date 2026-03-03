/**
 * display_manager.cpp — LCD Display Manager Implementation
 * 
 * Layout ekranu (landscape 240x135):
 * ┌──────────────────────────────────────┐
 * │ [MODE] MANUAL          BAT: 3.85V   │  <- Status Bar (y: 0-20)
 * │──────────────────────────────────────│
 * │ Dist: 45 cm  ████████████████░░░░░░ │  <- Distance Bar (y: 25-50)
 * │──────────────────────────────────────│
 * │ CAM: --          WiFi: 192.168.4.1  │  <- Camera/WiFi Status (y: 55-75)
 * │──────────────────────────────────────│
 * │ I2C: 0x38 ✓  0x29 ✓  0x40 ✗         │  <- I2C Status (y: 80-100)
 * │──────────────────────────────────────│
 * │ [message area]                      │  <- Messages (y: 105-130)
 * └──────────────────────────────────────┘
 */

#include "display_manager.h"

// Layout constants (landscape: width=240, height=135)
#define STATUS_BAR_Y       2
#define STATUS_BAR_HEIGHT  18
#define DISTANCE_BAR_Y     25
#define DISTANCE_BAR_H     25
#define CAM_STATUS_Y       55
#define I2C_STATUS_Y       80
#define MSG_AREA_Y         105
#define BAR_X              10
#define BAR_WIDTH          220

void DisplayManager::begin() {
    M5.Lcd.setRotation(1);        // Landscape
    M5.Lcd.fillScreen(TFT_BLACK);
    _needsFullRedraw = true;
    drawSplash();
}

void DisplayManager::clear() {
    M5.Lcd.fillScreen(TFT_BLACK);
    _needsFullRedraw = true;
}

void DisplayManager::drawSplash() {
    M5.Lcd.fillScreen(TFT_BLACK);
    
    // Logo / Title
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(40, 30);
    M5.Lcd.println("BugC2 Brain");
    
    // Version
    M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(65, 60);
    M5.Lcd.println("v1.0.0 - Phase 1");
    
    // Decorative line
    M5.Lcd.drawLine(30, 80, 210, 80, TFT_GREEN);
    
    // Loading message
    M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5.Lcd.setCursor(70, 95);
    M5.Lcd.println("Initializing...");
}

void DisplayManager::drawStatusBar(DrivingMode mode, float batteryVoltage) {
    // Clear status bar area
    M5.Lcd.fillRect(0, STATUS_BAR_Y, 240, STATUS_BAR_HEIGHT, TFT_BLACK);
    
    // Mode indicator (left side)
    uint16_t modeColor = modeToColor(mode);
    M5.Lcd.fillRoundRect(BAR_X, STATUS_BAR_Y, 8, STATUS_BAR_HEIGHT - 2, 2, modeColor);
    M5.Lcd.setTextColor(modeColor, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(BAR_X + 12, STATUS_BAR_Y + 5);
    M5.Lcd.print(modeToString(mode));
    
    // Battery (right side)
    uint16_t batColor = batteryColor(batteryVoltage);
    M5.Lcd.setTextColor(batColor, TFT_BLACK);
    M5.Lcd.setCursor(170, STATUS_BAR_Y + 5);
    M5.Lcd.printf("%.2fV", batteryVoltage);
    
    // Battery icon (simple bar)
    int batLevel = constrain(map((int)(batteryVoltage * 100), 300, 420, 0, 20), 0, 20);
    M5.Lcd.drawRect(218, STATUS_BAR_Y + 3, 14, 10, batColor);
    M5.Lcd.fillRect(219, STATUS_BAR_Y + 4, batLevel * 12 / 20, 8, batColor);
    
    // Separator line
    M5.Lcd.drawLine(0, STATUS_BAR_Y + STATUS_BAR_HEIGHT, 240, STATUS_BAR_Y + STATUS_BAR_HEIGHT, TFT_DARKGREY);
}

void DisplayManager::drawDistanceBar(int distanceCm) {
    // Clear area
    M5.Lcd.fillRect(0, DISTANCE_BAR_Y, 240, DISTANCE_BAR_H, TFT_BLACK);
    
    if (distanceCm < 0) {
        M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(BAR_X, DISTANCE_BAR_Y + 5);
        M5.Lcd.println("ToF: --");
        return;
    }
    
    uint16_t color = distanceColor(distanceCm);
    
    // Distance text
    M5.Lcd.setTextColor(color, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(BAR_X, DISTANCE_BAR_Y + 2);
    M5.Lcd.printf("Dist: %d cm", distanceCm);
    
    // Progress bar (inverted — shorter = closer = more danger)
    int barMaxDist = 100;  // 100cm = full bar
    int barLen = constrain(map(distanceCm, 0, barMaxDist, 0, BAR_WIDTH), 0, BAR_WIDTH);
    M5.Lcd.drawRect(BAR_X, DISTANCE_BAR_Y + 14, BAR_WIDTH, 8, TFT_DARKGREY);
    if (barLen > 0) {
        M5.Lcd.fillRect(BAR_X + 1, DISTANCE_BAR_Y + 15, barLen - 1, 6, color);
    }
    
    // Threshold markers
    int stopMarker = map(TOF_STOP_CM, 0, barMaxDist, BAR_X, BAR_X + BAR_WIDTH);
    int slowMarker = map(TOF_SLOW_CM, 0, barMaxDist, BAR_X, BAR_X + BAR_WIDTH);
    M5.Lcd.drawLine(stopMarker, DISTANCE_BAR_Y + 13, stopMarker, DISTANCE_BAR_Y + 23, TFT_RED);
    M5.Lcd.drawLine(slowMarker, DISTANCE_BAR_Y + 13, slowMarker, DISTANCE_BAR_Y + 23, TFT_YELLOW);
    
    // Separator
    M5.Lcd.drawLine(0, DISTANCE_BAR_Y + DISTANCE_BAR_H, 240, DISTANCE_BAR_Y + DISTANCE_BAR_H, TFT_DARKGREY);
}

void DisplayManager::drawCameraStatus(bool connected, const char* lastDetection) {
    M5.Lcd.fillRect(0, CAM_STATUS_Y, 120, 20, TFT_BLACK); // Tylko lewa połowa
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(BAR_X, CAM_STATUS_Y + 2);
    
    if (connected) {
        M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
        M5.Lcd.print("CAM: OK");
    } else {
        M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
        M5.Lcd.print("CAM: --");
    }
}

void DisplayManager::drawWiFiStatus(String ipAddress) {
    M5.Lcd.fillRect(120, CAM_STATUS_Y, 120, 20, TFT_BLACK); // Tylko prawa połowa
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(120, CAM_STATUS_Y + 2);
    
    if (ipAddress.length() > 0) {
        M5.Lcd.setTextColor(TFT_CYAN, TFT_BLACK);
        M5.Lcd.printf("AP: %s", ipAddress.c_str());
    } else {
        M5.Lcd.setTextColor(TFT_DARKGREY, TFT_BLACK);
        M5.Lcd.print("AP: OFF");
    }
    
    M5.Lcd.drawLine(0, CAM_STATUS_Y + 20, 240, CAM_STATUS_Y + 20, TFT_DARKGREY);
}

void DisplayManager::drawI2CStatus(uint8_t* devices, int count) {
    M5.Lcd.fillRect(0, I2C_STATUS_Y, 240, 20, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(BAR_X, I2C_STATUS_Y + 2);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.print("I2C:");
    
    // Check expected devices
    struct { uint8_t addr; const char* name; } expected[] = {
        { BUGC2_ADDR, "MOT" },
        { TOF4M_ADDR, "ToF" },
        { CAM_ADDR,   "CAM" }
    };
    
    int x = BAR_X + 30;
    for (int i = 0; i < 3; i++) {
        bool found = false;
        for (int j = 0; j < count; j++) {
            if (devices[j] == expected[i].addr) { found = true; break; }
        }
        
        M5.Lcd.setCursor(x, I2C_STATUS_Y + 2);
        if (found) {
            M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
            M5.Lcd.printf("%s", expected[i].name);
        } else {
            M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
            M5.Lcd.printf("%s", expected[i].name);
        }
        x += 45;
    }
    
    M5.Lcd.drawLine(0, I2C_STATUS_Y + 20, 240, I2C_STATUS_Y + 20, TFT_DARKGREY);
}

void DisplayManager::drawMessage(const char* msg) {
    M5.Lcd.fillRect(0, MSG_AREA_Y, 240, 30, TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(BAR_X, MSG_AREA_Y + 5);
    M5.Lcd.println(msg);
}

void DisplayManager::update(DrivingMode mode, float battery, int distance,
                            bool camConnected, const char* camDetection, String wifiIP) {
    // Optymalizacja: rysuj tylko zmienione elementy
    bool modeChanged = (mode != _lastMode) || _needsFullRedraw;
    bool batteryChanged = (abs(battery - _lastBattery) > 0.05f) || _needsFullRedraw;
    bool distanceChanged = (distance != _lastDistance) || _needsFullRedraw;
    bool camChanged = (camConnected != _lastCamConnected) || _needsFullRedraw;
    bool wifiChanged = (wifiIP != _lastWifiIP) || _needsFullRedraw;
    
    if (modeChanged || batteryChanged) {
        drawStatusBar(mode, battery);
    }
    
    if (distanceChanged) {
        drawDistanceBar(distance);
    }
    
    if (camChanged || wifiChanged) {
        drawCameraStatus(camConnected, camDetection);
        drawWiFiStatus(wifiIP);
    }
    
    // Zapamiętaj stan
    _lastMode = mode;
    _lastBattery = battery;
    _lastDistance = distance;
    _lastCamConnected = camConnected;
    _lastWifiIP = wifiIP;
    _needsFullRedraw = false;
}

uint16_t DisplayManager::batteryColor(float voltage) {
    if (voltage > 3.7f) return TFT_GREEN;
    if (voltage > 3.3f) return TFT_YELLOW;
    return TFT_RED;
}

uint16_t DisplayManager::distanceColor(int distanceCm) {
    if (distanceCm < 0) return TFT_DARKGREY;
    if (distanceCm <= TOF_STOP_CM) return TFT_RED;
    if (distanceCm <= TOF_SLOW_CM) return TFT_YELLOW;
    return TFT_GREEN;
}
