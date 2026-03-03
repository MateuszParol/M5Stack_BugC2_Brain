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
    stop();             // Upewnij się, że silniki są zatrzymane na starcie
    setLED(0, 0, 0, 0); // Wyłącz LED 0
    setLED(1, 0, 0, 0); // Wyłącz LED 1
  } else {
    Serial.println("[MotorDriver] WARNING: BugC2 NOT found at 0x38!");
  }
}

void MotorDriver::setMotor(uint8_t motor, int8_t speed) {
  if (motor > 3)
    return; // Tylko silniki 0-3
  writeI2C(REG_MOTOR_0 + motor, (uint8_t)speed);
}

// Zgodnie z oficjalną biblioteką M5HatBugC, aby jechać do przodu (BugC)
// Wymagane są następujące polaryzacje dla 4 kanałów:
// CH0: speed
// CH1: -speed
// CH2: speed
// CH3: -speed
uint8_t formatSpeed(int8_t speed) {
  // W wielu układach I2C zbudowanych dla oszczędności na STM32 (np.
  // RoverC/BugC2 z L9110S) kierunek 'w tył' nie jest obsługiwany czystym Two's
  // Complement (-1..-127), lecz jako prędkość + 128 (ustawiony najstarszy bit,
  // np. -50 = 128 + 50 = 178).
  if (speed < 0) {
    return 128 + (-speed);
  }
  return speed;
}

void MotorDriver::setAllMotorSpeed(int8_t speed_0, int8_t speed_1,
                                   int8_t speed_2, int8_t speed_3) {
  writeI2C(REG_MOTOR_0, formatSpeed(speed_0));
  writeI2C(REG_MOTOR_1, formatSpeed(speed_1));
  writeI2C(REG_MOTOR_2, formatSpeed(speed_2));
  writeI2C(REG_MOTOR_3, formatSpeed(speed_3));
}

void MotorDriver::forward(int8_t speed) {
  // GÓRA: Pchają tylko lewe koła (Lewy rykoszet)
  setAllMotorSpeed(speed, 0, speed, 0);
}

void MotorDriver::backward(int8_t speed) {
  // DÓŁ: Pchają tylko prawe koła (Prawy rykoszet)
  setAllMotorSpeed(0, speed, 0, speed);
}

void MotorDriver::turnLeft(int8_t speed) {
  // LEWO: Wszystkie "do przodu" używając nowego formatowania znaków
  setAllMotorSpeed(speed, -speed, speed, -speed);
}

void MotorDriver::turnRight(int8_t speed) {
  // PRAWO: Wszystkie "na odwrót" (powinien cofać jak rakieta)
  setAllMotorSpeed(-speed, speed, -speed, speed);
}

void MotorDriver::spinLeft(int8_t speed) {
  // ROTATE
  setAllMotorSpeed(speed, speed, speed, speed);
}

void MotorDriver::spinRight(int8_t speed) {
  // REVERSE ROTATE
  setAllMotorSpeed(-speed, -speed, -speed, -speed);
}

void MotorDriver::stop() { setAllMotorSpeed(0, 0, 0, 0); }

float MotorDriver::getBatteryVoltage() {
  // Odczyt surowej wartości napięcia z rejestru 0x10
  // BugC2 zwraca wartość w mV lub zakodowaną — wymaga kalibracji
  Wire.beginTransmission(BUGC2_ADDR);
  Wire.write(REG_BATTERY);
  Wire.endTransmission(false); // Restart, nie stop

  Wire.requestFrom((uint8_t)BUGC2_ADDR, (uint8_t)2);
  if (Wire.available() >= 2) {
    uint8_t low = Wire.read();
    uint8_t high = Wire.read();
    uint16_t raw = (high << 8) | low;
    // Konwersja: raw → volty (BugC2 zwraca mV)
    return raw / 1000.0f;
  }
  return 0.0f; // Błąd odczytu
}

void MotorDriver::setLED(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
  if (led > 1)
    return; // Tylko LED 0 i 1

  uint8_t baseReg = (led == 0) ? REG_LED0_R : REG_LED1_R;
  writeI2C(baseReg, r);
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
