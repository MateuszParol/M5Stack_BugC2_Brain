/**
 * motor_driver.cpp — BugC2 Motor Driver Implementation
 * 
 * Komunikacja I2C z BugC2 (STM32F030, adres 0x38):
 * - Rejestry 0x00-0x03: prędkość silników (-127..127)
 * - Rejestr 0x10: napięcie baterii
 * - Rejestry 0x20-0x25: LED RGB
 */

#include "motor_driver.h"

void MotorDriver::begin() {
    // Wire.begin() powinno być już wywołane w main setup()
    // Sprawdź połączenie z BugC2
    if (isConnected()) {
        Serial.println("[MotorDriver] BugC2 connected at 0x38");
        stop();                    // Upewnij się, że silniki są zatrzymane na starcie
        setLED(0, 0, 0, 0);       // Wyłącz LED 0
        setLED(1, 0, 0, 0);       // Wyłącz LED 1
    } else {
        Serial.println("[MotorDriver] WARNING: BugC2 NOT found at 0x38!");
    }
}

void MotorDriver::setMotor(uint8_t motor, int8_t speed) {
    if (motor > 3) return;  // Tylko silniki 0-3
    writeI2C(REG_MOTOR_0 + motor, (uint8_t)speed);
}

void MotorDriver::forward(int8_t speed) {
    // Wszystkie silniki naprzód
    // UWAGA: Mapowanie może wymagać kalibracji!
    // Niektóre silniki mogą być zamontowane odwrotnie
    setMotor(MOTOR_LEFT_FRONT,  speed);
    setMotor(MOTOR_RIGHT_FRONT, speed);
    setMotor(MOTOR_LEFT_REAR,   speed);
    setMotor(MOTOR_RIGHT_REAR,  speed);
}

void MotorDriver::backward(int8_t speed) {
    // Wszystkie silniki wstecz (negacja prędkości)
    int8_t rev = -speed;
    setMotor(MOTOR_LEFT_FRONT,  rev);
    setMotor(MOTOR_RIGHT_FRONT, rev);
    setMotor(MOTOR_LEFT_REAR,   rev);
    setMotor(MOTOR_RIGHT_REAR,  rev);
}

void MotorDriver::turnLeft(int8_t speed) {
    // Prawe silniki naprzód, lewe stop
    // Robot skręca w lewo z łukiem
    setMotor(MOTOR_LEFT_FRONT,  0);
    setMotor(MOTOR_RIGHT_FRONT, speed);
    setMotor(MOTOR_LEFT_REAR,   0);
    setMotor(MOTOR_RIGHT_REAR,  speed);
}

void MotorDriver::turnRight(int8_t speed) {
    // Lewe silniki naprzód, prawe stop
    // Robot skręca w prawo z łukiem
    setMotor(MOTOR_LEFT_FRONT,  speed);
    setMotor(MOTOR_RIGHT_FRONT, 0);
    setMotor(MOTOR_LEFT_REAR,   speed);
    setMotor(MOTOR_RIGHT_REAR,  0);
}

void MotorDriver::spinLeft(int8_t speed) {
    // Obrót w miejscu: prawe naprzód, lewe wstecz
    int8_t rev = -speed;
    setMotor(MOTOR_LEFT_FRONT,  rev);
    setMotor(MOTOR_RIGHT_FRONT, speed);
    setMotor(MOTOR_LEFT_REAR,   rev);
    setMotor(MOTOR_RIGHT_REAR,  speed);
}

void MotorDriver::spinRight(int8_t speed) {
    // Obrót w miejscu: lewe naprzód, prawe wstecz
    int8_t rev = -speed;
    setMotor(MOTOR_LEFT_FRONT,  speed);
    setMotor(MOTOR_RIGHT_FRONT, rev);
    setMotor(MOTOR_LEFT_REAR,   speed);
    setMotor(MOTOR_RIGHT_REAR,  rev);
}

void MotorDriver::stop() {
    setMotor(MOTOR_LEFT_FRONT,  0);
    setMotor(MOTOR_RIGHT_FRONT, 0);
    setMotor(MOTOR_LEFT_REAR,   0);
    setMotor(MOTOR_RIGHT_REAR,  0);
}

float MotorDriver::getBatteryVoltage() {
    // Odczyt surowej wartości napięcia z rejestru 0x10
    // BugC2 zwraca wartość w mV lub zakodowaną — wymaga kalibracji
    Wire.beginTransmission(BUGC2_ADDR);
    Wire.write(REG_BATTERY);
    Wire.endTransmission(false);  // Restart, nie stop
    
    Wire.requestFrom((uint8_t)BUGC2_ADDR, (uint8_t)2);
    if (Wire.available() >= 2) {
        uint8_t low  = Wire.read();
        uint8_t high = Wire.read();
        uint16_t raw = (high << 8) | low;
        // Konwersja: raw → volty (BugC2 zwraca mV)
        return raw / 1000.0f;
    }
    return 0.0f;  // Błąd odczytu
}

void MotorDriver::setLED(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
    if (led > 1) return;  // Tylko LED 0 i 1
    
    uint8_t baseReg = (led == 0) ? REG_LED0_R : REG_LED1_R;
    writeI2C(baseReg,     r);
    writeI2C(baseReg + 1, g);
    writeI2C(baseReg + 2, b);
}

bool MotorDriver::isConnected() {
    Wire.beginTransmission(BUGC2_ADDR);
    return (Wire.endTransmission() == 0);
}

// ============================================================
// Private methods
// ============================================================

void MotorDriver::writeI2C(uint8_t reg, uint8_t data) {
    Wire.beginTransmission(BUGC2_ADDR);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

uint8_t MotorDriver::readI2C(uint8_t reg) {
    Wire.beginTransmission(BUGC2_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    
    Wire.requestFrom((uint8_t)BUGC2_ADDR, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}
