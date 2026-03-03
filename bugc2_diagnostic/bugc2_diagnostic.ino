/**
 * bugc2_diagnostic.ino — BugC2 Diagnostic Firmware
 *
 * Służy wyłącznie do weryfikacji sprzętowej napędu na płycie bazowej M5StickC
 * BugC2. Całkowicie pomija ewentualne połączenia I2C do kamer, ToF i
 * jakiegokolwiek automatycznego omijania przeszkód.
 *
 * Sprzęt:
 * - M5StickC Plus 2
 * - BugC2 Motor Driver
 */

#include "config.h"
#include "motor_driver.h"
#include "remote_controller.h"
#include <M5StickCPlus2.h>
#include <Wire.h>

// Obiekty
MotorDriver motors;
RemoteController remoteCtrl;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  Serial.begin(115200);
  Serial.println("========================================");
  Serial.println("  BugC2 DIAGNOSTIC M5StickC Plus 2");
  Serial.println("  ONLY MOTOR DRIVER AND WIFI AP");
  Serial.println("========================================");

  // I2C dla silników (BugC2)
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  // Inicjalizacja silników
  // UWAGA: Upewnij się, że przełącznik na podwoziu BugC2 jest w pozycji ON!
  motors.begin();

  // Inicjalizacja Sieci Webowej
  remoteCtrl.begin();

  Serial.println(
      "[Setup] Ready! Connect to WiFi 'BugC2_Robot' and visit 192.168.4.1");
}

String lastWebCmd = "S";

void loop() {
  M5.update();

  // Obsługa WebServera Remote Controller
  remoteCtrl.handleClient();

  // Odbiór pełnego sterowania z dwóch joysticków 360 (LX, LY, RX, RY od -100 do
  // 100)
  if (remoteCtrl.hasNewTankCommand()) {
    int8_t lx = remoteCtrl.getRequestedJoyLX();
    int8_t ly = remoteCtrl.getRequestedJoyLY();
    int8_t rx = remoteCtrl.getRequestedJoyRX();
    int8_t ry = remoteCtrl.getRequestedJoyRY();
    remoteCtrl.clearRequests();

    // Wypisujemy rzadziej logi z joya, żeby nie zatkać portu szeregowego
    static uint32_t lastLogTime = 0;
    if (millis() - lastLogTime > 200 ||
        (lx == 0 && ly == 0 && rx == 0 && ry == 0)) {
      Serial.printf("[WWW] JOYSTICK: L(%d,%d) | R(%d,%d)\n", lx, ly, rx, ry);
      lastLogTime = millis();
    }

    // BugC2 silniki:
    // 0: Lewy Przód
    // 1: Prawy Przód
    // 2: Lewy Tył
    // 3: Prawy Tył

    // Eksperymentalny Mikser Omni-Directional per burta:
    // Oś Y to moc główna przód/tył, Oś X to różnica wymuszająca obrót Osi
    // W niektórych obudowach kół na ślizgu (skid-steer) osie lewego boku są
    // połączone.
    int m0 = ly + lx; // Lewy Przód
    int m2 = ly - lx; // Lewy Tył

    int m1 = ry + rx; // Prawy Przód
    int m3 = ry - rx; // Prawy Tył

    // Skalowanie prędkości, aby robot był wolniejszy podczas precyzyjnych
    // testów (np. max 40%)
    const float SPEED_SCALAR = 0.4f;
    m0 = (int)(m0 * SPEED_SCALAR);
    m1 = (int)(m1 * SPEED_SCALAR);
    m2 = (int)(m2 * SPEED_SCALAR);
    m3 = (int)(m3 * SPEED_SCALAR);

    // Zabezpieczenie przed przepełnieniem 8-bitowego argumentu w driverze (-100
    // do 100)
    if (m0 > 100)
      m0 = 100;
    if (m0 < -100)
      m0 = -100;
    if (m1 > 100)
      m1 = 100;
    if (m1 < -100)
      m1 = -100;
    if (m2 > 100)
      m2 = 100;
    if (m2 < -100)
      m2 = -100;
    if (m3 > 100)
      m3 = 100;
    if (m3 < -100)
      m3 = -100;

    motors.setAllMotorSpeed(m0, m1, m2, m3);

    // RGB Feedback
    if (ly > 20 || ry > 20)
      motors.setLED(0, 0, 30, 0); // Zielony
    else if (ly < -20 || ry < -20)
      motors.setLED(0, 30, 0, 0); // Czerwony
    else
      motors.setLED(0, 0, 0, 0);
  }

  // Wciśnięcie Przycisku A (Front) nadpisuje Testowo układ wprost (70, 70)
  if (M5.BtnA.isPressed()) {
    motors.setAllMotorSpeed(70, 70, 70, 70);
  } else if (M5.BtnA.wasReleased()) {
    motors.stop();
  }
}
