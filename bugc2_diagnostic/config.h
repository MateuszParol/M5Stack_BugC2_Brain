/**
 * config.h — BugC2 Brain Configuration
 *
 * Centralne definicje stałych dla całego projektu:
 * - Adresy I2C urządzeń
 * - Piny GPIO
 * - Parametry silników
 * - Progi czujników
 * - Stałe LCD
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// I2C Addresses
// ============================================================
#define BUGC2_ADDR 0x38 // BugC2 motor driver (STM32F030)
#define TOF4M_ADDR 0x29 // ToF4M distance sensor (VL53L1X)
#define CAM_ADDR 0x40   // AtomS3R-CAM AI (I2C slave — do potwierdzenia)

// ============================================================
// I2C Pins (M5StickC Plus 2 HAT Connector)
// ============================================================
// Hat devices (like BugC2) connect to the top interface
#define SDA_PIN 0
#define SCL_PIN 26

// ============================================================
// Button Pins (M5StickC Plus 2)
// ============================================================
#define BTN_A_PIN 37 // Front button
#define BTN_B_PIN 39 // Side button

// ============================================================
// Motor Speeds (-127 to 127)
// ============================================================
#define SPEED_SLOW 40
#define SPEED_NORMAL 70
#define SPEED_FAST 100
#define SPEED_MAX 127

// ============================================================
// Motor Mapping (BugC2 — do weryfikacji empirycznej!)
// Silniki 0-3 mapowane na pozycje kół
// ============================================================
#define MOTOR_LEFT_FRONT 0
#define MOTOR_RIGHT_FRONT 1
#define MOTOR_LEFT_REAR 2
#define MOTOR_RIGHT_REAR 3

// ============================================================
// BugC2 I2C Registers
// ============================================================
#define REG_MOTOR_0 0x00 // Motor 0 speed (-127..127)
#define REG_MOTOR_1 0x01 // Motor 1 speed
#define REG_MOTOR_2 0x02 // Motor 2 speed
#define REG_MOTOR_3 0x03 // Motor 3 speed
#define REG_BATTERY 0x10 // Battery voltage readout
#define REG_LED0_R 0x20  // LED 0 Red
#define REG_LED0_G 0x21  // LED 0 Green
#define REG_LED0_B 0x22  // LED 0 Blue
#define REG_LED1_R 0x23  // LED 1 Red
#define REG_LED1_G 0x24  // LED 1 Green
#define REG_LED1_B 0x25  // LED 1 Blue

// ============================================================
// ToF Sensor Thresholds (cm)
// ============================================================
#define TOF_STOP_CM 15 // Emergency stop distance
#define TOF_SLOW_CM 30 // Slow down distance
#define TOF_MAX_CM 400 // Maximum sensor range

// ============================================================
// LCD (M5StickC Plus 2 — 1.14" TFT ST7789V2)
// ============================================================
#define LCD_WIDTH 135
#define LCD_HEIGHT 240

// ============================================================
// Timing (ms)
// ============================================================
#define DISPLAY_UPDATE_INTERVAL 200 // LCD refresh rate
#define TOF_READ_INTERVAL 50        // ToF sensor read rate (20Hz)
#define MOTOR_UPDATE_INTERVAL 50    // Motor command rate
#define SERIAL_BAUD_RATE 115200

// ============================================================
// WiFi & Web Server (Phase 4)
// ============================================================
#define WIFI_AP_SSID "BugC2_Robot"
#define WIFI_AP_PASS "12345678" // min 8 znaków dla WPA2
#define WEB_SERVER_PORT 80

#endif // CONFIG_H
