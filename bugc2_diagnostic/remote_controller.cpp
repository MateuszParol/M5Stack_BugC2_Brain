/**
 * remote_controller.cpp — Web Server Remote Control Impl (Phase 4)
 *
 * Implementacja klasy RemoteController i responsywnej strony kontrolnej.
 */

#include "remote_controller.h"

// HTML i JS w pamięci programu, oszczędza RAM
const char RemoteController::INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no, minimal-ui">
  <title>BugC2 Dual Joystick</title>
  <style>
    body { font-family: sans-serif; background: #121212; color: #fff; margin:0; padding:10px; display:flex; flex-direction:column; align-items:center; touch-action: none; overflow: hidden; user-select: none; -webkit-user-select: none; }
    h2 { margin-top: 5px; margin-bottom: 10px; color: #00ffcc; text-shadow: 0 0 10px rgba(0,255,204,0.3); }
    .joystick-panel { display: flex; width: 100%; max-width: 600px; justify-content: space-around; margin-top: 40px; }
    .joystick-zone { position: relative; width: 140px; height: 140px; background: rgba(255,255,255,0.05); border-radius: 50%; display: flex; justify-content: center; align-items: center; border: 2px solid #444; box-shadow: inset 0 0 20px rgba(0,0,0,0.8); }
    .joystick-knob { position: absolute; width: 50px; height: 50px; background: #00ffcc; border-radius: 50%; box-shadow: 0 0 15px rgba(0, 255, 204, 0.5); pointer-events: none; }
    .status-bar { margin-top: 30px; font-family: monospace; font-size: 14px; color: #aaa; text-align: center; }
    .label { position: absolute; top: -30px; font-weight: bold; color: #888; text-transform: uppercase; font-size: 14px; }
    .stop-btn { margin-top: 30px; background: #ff3333; color: white; border: none; padding: 15px 40px; font-size: 18px; border-radius: 10px; font-weight: bold; cursor: pointer; }
    .stop-btn:active { background: #cc0000; transform: scale(0.95); }
  </style>
</head>
<body>
  <h2>🤖 BugC2 Omnidrive</h2>
  
  <div class="joystick-panel">
      <div class="joystick-zone" id="zoneL">
          <div class="label">LEWA GAŁKA</div>
          <div class="joystick-knob" id="knobL"></div>
      </div>
      <div class="joystick-zone" id="zoneR">
          <div class="label">PRAWA GAŁKA</div>
          <div class="joystick-knob" id="knobR"></div>
      </div>
  </div>

  <button class="stop-btn" onclick="hardStop()">AWARYJNY STOP</button>
  <div class="status-bar" id="statusMsg">LX:0 LY:0 | RX:0 RY:0</div>

  <script>
    class Joystick {
        constructor(zoneId, knobId, onChange) {
            this.zone = document.getElementById(zoneId);
            this.knob = document.getElementById(knobId);
            this.onChange = onChange;
            this.active = false;
            this.origin = { x: 0, y: 0 };
            this.pos = { x: 0, y: 0 };
            this.maxRadius = this.zone.offsetWidth / 2;
            
            this.zone.addEventListener('mousedown', this.start.bind(this));
            window.addEventListener('mousemove', this.move.bind(this));
            window.addEventListener('mouseup', this.end.bind(this));

            this.zone.addEventListener('touchstart', (e) => { e.preventDefault(); this.start(e.touches[0]); }, {passive: false});
            window.addEventListener('touchmove', (e) => {
                // Szukamy dotyku przypisanego do naszej osi
                for(let i=0; i<e.touches.length; i++) {
                    let r = this.zone.getBoundingClientRect();
                    // z grubsza margines łapania
                    if(e.touches[i].clientX >= r.left-50 && e.touches[i].clientX <= r.right+50 &&
                       e.touches[i].clientY >= r.top-50 && e.touches[i].clientY <= r.bottom+50) {
                        this.move(e.touches[i]);
                    }
                }
            }, {passive: false});
            window.addEventListener('touchend', this.end.bind(this));
        }

        start(e) {
            this.active = true;
            let rect = this.zone.getBoundingClientRect();
            this.origin = { x: rect.left + rect.width/2, y: rect.top + rect.height/2 };
            this.updatePos(e.clientX, e.clientY);
        }

        move(e) {
            if (!this.active) return;
            this.updatePos(e.clientX, e.clientY);
        }

        end() {
            if (!this.active) return;
            this.active = false;
            this.pos = { x: 0, y: 0 };
            this.updateKnob();
            this.onChange(0, 0);
        }

        updatePos(clientX, clientY) {
            let dx = clientX - this.origin.x;
            let dy = clientY - this.origin.y;
            let distance = Math.sqrt(dx*dx + dy*dy);
            
            if (distance > this.maxRadius) {
                dx = dx * (this.maxRadius / distance);
                dy = dy * (this.maxRadius / distance);
            }
            
            this.pos = { x: dx, y: dy };
            this.updateKnob();
            
            let normX = Math.round((dx / this.maxRadius) * 100);
            let normY = Math.round((dy / this.maxRadius) * -100); 
            
            this.onChange(normX, normY);
        }

        updateKnob() {
            this.knob.style.transform = `translate(${this.pos.x}px, ${this.pos.y}px)`;
        }
    }

    let state = { lx: 0, ly: 0, rx: 0, ry: 0 };
    let lastState = { lx: 0, ly: 0, rx: 0, ry: 0 };
    const statusBox = document.getElementById('statusMsg');

    new Joystick('zoneL', 'knobL', (x, y) => { state.lx = x; state.ly = y; });
    new Joystick('zoneR', 'knobR', (x, y) => { state.rx = x; state.ry = y; });

    function hardStop() {
        state = { lx:0, ly:0, rx:0, ry:0 };
        sendCommand();
    }

    function sendCommand() {
        if (state.lx === lastState.lx && state.ly === lastState.ly && 
            state.rx === lastState.rx && state.ry === lastState.ry) return;

        statusBox.innerHTML = `LX:${state.lx} LY:${state.ly} | RX:${state.rx} RY:${state.ry}`;
        fetch(`/api/action?lx=${state.lx}&ly=${state.ly}&rx=${state.rx}&ry=${state.ry}`)
            .catch(e => console.error(e));

        lastState = { ...state };
    }

    setInterval(sendCommand, 100);
  </script>
</body>
</html>
)rawliteral";

RemoteController::RemoteController() : _server(WEB_SERVER_PORT) {
  _hasNewTankCommand = false;
  _joyLX = 0;
  _joyLY = 0;
  _joyRX = 0;
  _joyRY = 0;
  _requestedMode = -1;
}

void RemoteController::begin() {
  Serial.println("[WiFi] Starting Access Point...");

  WiFi.mode(WIFI_AP);
  bool success = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS, 1, 0, 4);

  if (success) {
    Serial.printf("[WiFi] AP Started! SSID: %s\n", WIFI_AP_SSID);
    Serial.printf("[WiFi] AP IP Address: %s\n",
                  WiFi.softAPIP().toString().c_str());
  } else {
    Serial.println("[WiFi] ERROR: Failed to start AP!");
  }

  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  _server.on("/api/action", HTTP_GET, [this]() { handleAction(); });
  _server.on("/api/mode", HTTP_GET, [this]() { handleMode(); });
  _server.onNotFound([this]() { handleNotFound(); });

  _server.begin();
  Serial.println("[Web] HTTP Server started.");
}

void RemoteController::handleClient() { _server.handleClient(); }

String RemoteController::getIPAddress() { return WiFi.softAPIP().toString(); }

bool RemoteController::isClientConnected() {
  return WiFi.softAPgetStationNum() > 0;
}

void RemoteController::clearRequests() {
  _hasNewTankCommand = false;
  _requestedMode = -1;
}

void RemoteController::handleRoot() {
  _server.send(200, "text/html", INDEX_HTML);
}

void RemoteController::handleAction() {
  if (_server.hasArg("lx") && _server.hasArg("ly") && _server.hasArg("rx") &&
      _server.hasArg("ry")) {
    _joyLX = (int8_t)_server.arg("lx").toInt();
    _joyLY = (int8_t)_server.arg("ly").toInt();
    _joyRX = (int8_t)_server.arg("rx").toInt();
    _joyRY = (int8_t)_server.arg("ry").toInt();

    _hasNewTankCommand = true;

    _server.send(200, "application/json",
                 "{\"status\":\"OK\",\"lx\":" + String(_joyLX) + "}");
    return;
  }
  _server.send(400, "application/json",
               "{\"status\":\"ERROR\",\"msg\":\"Missing args\"}");
}

void RemoteController::handleMode() {
  if (_server.hasArg("set")) {
    int m = _server.arg("set").toInt();
    if (m >= 0 && m <= 2) {
      _requestedMode = m;
      Serial.printf("[Web] Received Mode change: %d\n", m);
      _server.send(200, "application/json",
                   "{\"status\":\"OK\",\"mode\":" + String(m) + "}");
      return;
    }
  }
  _server.send(400, "application/json",
               "{\"status\":\"ERROR\",\"msg\":\"Invalid mode\"}");
}

void RemoteController::handleNotFound() {
  _server.send(404, "text/plain", "Not Found");
}
