/**
 * motor_driver.h — BugC2 Motor Driver
 *
 * Sterownik silników DC robota BugC2 przez I2C (adres 0x38).
 * Obsługuje: jazda przód/tył, skręty, stop, LED RGB, odczyt baterii.
 */

#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "config.h"
#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

class MotorDriver {
public:
  /**
   * Inicjalizacja sterownika silników.
   * UWAGA: Wire.begin() musi być wywołane PRZED begin()!
   */
  void begin();

  /**
   * Ustaw prędkość pojedynczego silnika.
   * @param motor  Numer silnika (0-3), użyj MOTOR_LEFT_FRONT itp.
   * @param speed  Prędkość (-127..127), ujemna = wstecz
   */
  void setMotor(uint8_t motor, int8_t speed);

  /**
   * Ustawia prędkość 4 silników w jednej transakcji I2C.
   */
  void setAllMotorSpeed(int8_t speed_0, int8_t speed_1, int8_t speed_2,
                        int8_t speed_3);

  /**
   * Jazda do przodu — wszystkie silniki.
   * @param speed  Prędkość (1..127)
   */
  void forward(int8_t speed);

  /**
   * Jazda do tyłu — wszystkie silniki.
   * @param speed  Prędkość (1..127), zostanie zanegowana wewnętrznie
   */
  void backward(int8_t speed);

  /**
   * Skręt w lewo — prawe silniki naprzód, lewe stop.
   * @param speed  Prędkość skrętu (1..127)
   */
  void turnLeft(int8_t speed);

  /**
   * Skręt w prawo — lewe silniki naprzód, prawe stop.
   * @param speed  Prędkość skrętu (1..127)
   */
  void turnRight(int8_t speed);

  /**
   * Skręt w lewo w obrocie — prawe naprzód, lewe wstecz.
   * @param speed  Prędkość obrotu (1..127)
   */
  void spinLeft(int8_t speed);

  /**
   * Skręt w prawo w obrocie — lewe naprzód, prawe wstecz.
   * @param speed  Prędkość obrotu (1..127)
   */
  void spinRight(int8_t speed);

  /**
   * Zatrzymaj wszystkie silniki (prędkość = 0).
   */
  void stop();

  /**
   * Odczytaj napięcie baterii BugC2.
   * @return Napięcie w woltach (typowo 3.0 - 4.2V)
   */
  float getBatteryVoltage();

  /**
   * Ustaw kolor LED RGB na BugC2.
   * @param led  Numer LED (0 lub 1)
   * @param r    Czerwony (0-255)
   * @param g    Zielony (0-255)
   * @param b    Niebieski (0-255)
   */
  void setLED(uint8_t led, uint8_t r, uint8_t g, uint8_t b);

  /**
   * Sprawdź czy BugC2 jest dostępny na magistrali I2C.
   * @return true jeśli urządzenie odpowiada
   */
  bool isConnected();

private:
  /**
   * Zapis jednego bajtu do rejestru I2C BugC2.
   */
  void writeI2C(uint8_t reg, uint8_t data);

  /**
   * Odczyt jednego bajtu z rejestru I2C BugC2.
   */
  uint8_t readI2C(uint8_t reg);
};

#endif // MOTOR_DRIVER_H
