/**
 * remote_controller.h — Web Server Remote Control (Phase 4)
 *
 * Implementuje własny Access Point (BugC2_Robot) oraz serwer HTTP
 * pod adresem 192.168.4.1 (domyślnie dla AP ESP32).
 * Wystawia prosty responsywny interfejs webowy (HTML/JS)
 * na komórki do sterowania kierunkiem jazdy i zmianą trybów.
 */

#ifndef REMOTE_CONTROLLER_H
#define REMOTE_CONTROLLER_H

#include "config.h"
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

class RemoteController {
public:
  RemoteController();

  /**
   * Inicjalizuje WiFi w trybie AP i konfiguruje endpointy WebServera.
   */
  void begin();

  /**
   * Główna obsługa logiki klienta HTTP. Powinna być wołana często w loop().
   */
  void handleClient();

  /**
   * Zwraca adres IP aktualnego Access Pointu.
   */
  String getIPAddress();

  /**
   * Sprawdza, czy są jakieś połączone stacje (urządzenia klienta).
   */
  bool isClientConnected();

  /**
   * Zwraca prawdę, jeśli otrzymano nowe komendy z suwaków tzn. nowy stan TANK
   */
  bool hasNewTankCommand() { return _hasNewTankCommand; }

  int8_t getRequestedJoyLX() { return _joyLX; }
  int8_t getRequestedJoyLY() { return _joyLY; }
  int8_t getRequestedJoyRX() { return _joyRX; }
  int8_t getRequestedJoyRY() { return _joyRY; }

  /**
   * Zwraca żądany z zewnątrz tryb jazdy (0=MANUAL, 1=SEMI_AUTO, 2=AUTONOMOUS).
   * Jeśli brak zmiany, zwraca -1.
   */
  int getRequestedMode() { return _requestedMode; }

  /**
   * Czyści flagi po odczytaniu przez pętlę główną.
   */
  void clearRequests();

private:
  WebServer _server;

  bool _hasNewTankCommand = false;
  int8_t _joyLX = 0;
  int8_t _joyLY = 0;
  int8_t _joyRX = 0;
  int8_t _joyRY = 0;

  int _requestedMode = -1;

  // Handlery endpointów
  void handleRoot();
  void handleAction();
  void handleMode();
  void handleNotFound();

  // UI Strona HTML w pamięci Flash
  static const char INDEX_HTML[] PROGMEM;
};

#endif // REMOTE_CONTROLLER_H
