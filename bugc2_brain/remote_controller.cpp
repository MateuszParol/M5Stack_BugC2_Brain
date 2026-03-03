/**
 * remote_controller.cpp — Web Server Remote Control Impl (Phase 4)
 * 
 * Implementacja klasy RemoteController i responsywnej strony kontrolnej.
 */

#include "remote_controller.h"

// HTML i JS w pamięci programu, oszczędza RAM
const char RemoteController::INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>BugC2 Brain Control</title>
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #121212; color: #fff; margin:0; padding:10px; display:flex; flex-direction:column; align-items:center; }
    h2 { margin-top: 5px; margin-bottom: 15px; text-shadow: 0 0 10px rgba(255,255,255,0.3); }
    .mode-panel { display:flex; gap: 10px; margin-bottom: 20px; width: 100%; max-width: 350px; justify-content: center; }
    .btn-mode { flex: 1; padding: 12px 5px; background: #333; border: 2px solid #555; color: white; border-radius: 8px; font-weight: bold; font-size: 14px; cursor: pointer; transition: all 0.2s; }
    .btn-mode.active-0 { background: #005f73; border-color: #0a9396; }
    .btn-mode.active-1 { background: #ca6702; border-color: #ee9b00; }
    .btn-mode.active-2 { background: #ae2012; border-color: #bb3e03; }
    
    .dpad { display: grid; grid-template-columns: 80px 80px 80px; grid-template-rows: 80px 80px 80px; gap: 10px; margin-top: 10px; }
    .btn-dir { background: #222; border: 2px solid #444; border-radius: 15px; color: #fff; font-size: 24px; font-weight: bold; display:flex; justify-content:center; align-items:center; user-select:none; }
    .btn-dir:active { background: #0a9396; border-color: #94d2bd; transform: scale(0.95); }
    .btn-dir.empty { background: transparent; border: none; }
    
    .status-bar { margin-top: 30px; padding: 10px; background: #1e1e1e; border-radius: 8px; width: 100%; max-width: 350px; text-align: center; border: 1px solid #333; }
    
    #stopBtn { grid-column: 2; grid-row: 2; border-radius: 50%; background: #ae2012; border-color: #9b2226; color: white; font-size: 16px;}
    #stopBtn:active { background: #cf2a27; }
  </style>
</head>
<body>
  <h2>🤖 BugC2 Control</h2>
  
  <div class="mode-panel">
    <button class="btn-mode" id="btnMode0" onclick="setMode(0)">MANUAL</button>
    <button class="btn-mode" id="btnMode1" onclick="setMode(1)">SEMI<br>AUTO</button>
    <button class="btn-mode" id="btnMode2" onclick="setMode(2)">AUTO</button>
  </div>

  <div class="dpad">
    <div class="btn-dir empty"></div>
    <div class="btn-dir" id="btnF">↑</div>
    <div class="btn-dir empty"></div>
    <div class="btn-dir" id="btnL">←</div>
    <div class="btn-dir" id="stopBtn">STOP</div>
    <div class="btn-dir" id="btnR">→</div>
    <div class="btn-dir empty"></div>
    <div class="btn-dir" id="btnB">↓</div>
    <div class="btn-dir empty"></div>
  </div>
  
  <div class="status-bar" id="statusMsg">Connected</div>

  <script>
    // Update active button locally based on last click
    function updateModeUI(m) {
      document.getElementById('btnMode0').className = "btn-mode";
      document.getElementById('btnMode1').className = "btn-mode";
      document.getElementById('btnMode2').className = "btn-mode";
      document.getElementById('btnMode'+m).className = "btn-mode active-"+m;
    }

    function setMode(m) {
      fetch('/api/mode?set=' + m)
        .then(res => res.json())
        .then(data => {
            if(data.status == "OK") {
                updateModeUI(m);
                document.getElementById('statusMsg').innerText = "Mode changed successfully";
            }
        }).catch(e => {
            document.getElementById('statusMsg').innerText = "Error changing mode";
        });
    }

    // Default UI
    updateModeUI(0);

    function sendCmd(cmd) {
      fetch('/api/action?cmd=' + cmd).catch(e => { console.log("Cmd error", e); });
    }

    function bindButton(id, cmd) {
      const el = document.getElementById(id);
      // Mouse events
      el.addEventListener('mousedown', (e) => { e.preventDefault(); sendCmd(cmd); });
      el.addEventListener('mouseup', (e) => { e.preventDefault(); sendCmd('S'); });
      el.addEventListener('mouseleave', (e) => { e.preventDefault(); sendCmd('S'); });
      // Touch events (mobile)
      el.addEventListener('touchstart', (e) => { e.preventDefault(); sendCmd(cmd); });
      el.addEventListener('touchend', (e) => { e.preventDefault(); sendCmd('S'); });
      el.addEventListener('touchcancel', (e) => { e.preventDefault(); sendCmd('S'); });
    }

    bindButton('btnF', 'F');
    bindButton('btnB', 'B');
    bindButton('btnL', 'L');
    bindButton('btnR', 'R');
    
    // Stop button (click explicitly instead of touch and hold)
    const elStop = document.getElementById('stopBtn');
    elStop.addEventListener('touchstart', (e) => { e.preventDefault(); sendCmd('S'); });
    elStop.addEventListener('mousedown', (e) => { e.preventDefault(); sendCmd('S'); });
  </script>
</body>
</html>
)rawliteral";


RemoteController::RemoteController() : _server(WEB_SERVER_PORT) {
    _requestedCommand = "";
    _requestedMode = -1;
}

void RemoteController::begin() {
    Serial.println("[WiFi] Starting Access Point...");
    
    // Ustawienie AP
    WiFi.mode(WIFI_AP);
    // WPA2, max 4 połączenia
    bool success = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS, 1, 0, 4);
    
    if (success) {
        Serial.printf("[WiFi] AP Started! SSID: %s\n", WIFI_AP_SSID);
        Serial.printf("[WiFi] AP IP Address: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println("[WiFi] ERROR: Failed to start AP!");
    }

    // Definiowanie endpointów
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/api/action", HTTP_GET, [this]() { handleAction(); });
    _server.on("/api/mode", HTTP_GET, [this]() { handleMode(); });
    _server.onNotFound([this]() { handleNotFound(); });

    _server.begin();
    Serial.println("[Web] HTTP Server started.");
}

void RemoteController::handleClient() {
    _server.handleClient();
}

String RemoteController::getIPAddress() {
    return WiFi.softAPIP().toString();
}

bool RemoteController::isClientConnected() {
    return WiFi.softAPgetStationNum() > 0;
}

void RemoteController::clearRequests() {
    _requestedCommand = "";
    _requestedMode = -1;
}

// =========================================================
// Wewnętrzne Handlery Serwera WWW
// =========================================================

void RemoteController::handleRoot() {
    // Serwowanie strony HTML z pamięci Flash
    _server.send(200, "text/html", INDEX_HTML);
}

void RemoteController::handleAction() {
    if (_server.hasArg("cmd")) {
        String cmd = _server.arg("cmd");
        cmd.toUpperCase();
        
        if (cmd == "F" || cmd == "B" || cmd == "L" || cmd == "R" || cmd == "S") {
            _requestedCommand = cmd;
            // Żeby nie śmiecić logów ciągle
            // Serial.printf("[Web] Received Command: %s\n", cmd.c_str());
            _server.send(200, "application/json", "{\"status\":\"OK\",\"cmd\":\"" + cmd + "\"}");
            return;
        }
    }
    _server.send(400, "application/json", "{\"status\":\"ERROR\",\"msg\":\"Invalid command\"}");
}

void RemoteController::handleMode() {
    if (_server.hasArg("set")) {
        int m = _server.arg("set").toInt();
        if (m >= 0 && m <= 2) {
            _requestedMode = m;
            Serial.printf("[Web] Received Mode change: %d\n", m);
            _server.send(200, "application/json", "{\"status\":\"OK\",\"mode\":" + String(m) + "}");
            return;
        }
    }
    _server.send(400, "application/json", "{\"status\":\"ERROR\",\"msg\":\"Invalid mode\"}");
}

void RemoteController::handleNotFound() {
    _server.send(404, "text/plain", "Not Found");
}
