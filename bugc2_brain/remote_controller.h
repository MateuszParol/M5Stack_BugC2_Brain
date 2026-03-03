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

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"

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
     * Zwraca aktualnie wciśnięty kierunek ("F", "B", "L", "R", "S").
     * Wartością domyślną powinno być pusty łańcuch lub poprzednia wartość,
     * jednak przyjmiemy model odczytaj i wyczyść w API, albo stały stan z pollingiem.
     */
    String getRequestedCommand() { return _requestedCommand; }

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
    
    String _requestedCommand = "";
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
