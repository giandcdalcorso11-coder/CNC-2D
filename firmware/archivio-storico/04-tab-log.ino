// CNC-2D — Dashboard web ESP32 (Step 1: LED su D2/D18/D19 + configurazione Wi-Fi via web)
//
// Librerie usate: tutte incluse nel core ESP32 per Arduino (nessuna installazione extra):
// WiFi.h, WebServer.h, Preferences.h, ArduinoOTA.h
//
// Comportamento al boot:
// - Se sono salvate credenziali Wi-Fi valide, si connette alla rete di casa (modalità STA)
//   ed è raggiungibile su http://cnc2d.local oppure sull'IP mostrato sul Serial Monitor.
// - Se non ci sono credenziali salvate, o la connessione fallisce, apre un Access Point
//   di emergenza (SSID/PASS sotto) su cui è comunque raggiungibile la stessa pagina,
//   per poter impostare/correggere le credenziali dalla tab Wi-Fi.
//
// Una volta connesso alla rete di casa, i successivi aggiornamenti firmware possono
// essere caricati via Wi-Fi (OTA) da Arduino IDE selezionando la porta di rete "cnc2d",
// senza bisogno del cavo USB. Password OTA: la stessa dell'Access Point di emergenza.

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoOTA.h>

#define RED_LED_PIN 2
#define YELLOW_LED_PIN 19
#define GREEN_LED_PIN 18
#define WIFI_CONNECT_TIMEOUT_MS 15000

const char* AP_SSID = "CNC-2D-Setup";
const char* AP_PASS = "cnc2d2026";

Preferences prefs;
WebServer server(80);

bool ledRedState = false;
bool ledYellowState = false;
bool ledGreenState = false;
bool apMode = false;

String logBuffer = "";
const size_t LOG_MAX_LEN = 4000;

void logMsg(const String& msg) {
  Serial.println(msg);
  logBuffer += msg;
  logBuffer += "\n";
  if (logBuffer.length() > LOG_MAX_LEN) {
    logBuffer = logBuffer.substring(logBuffer.length() - LOG_MAX_LEN);
  }
}

const char PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="it">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>CNC-2D Dashboard</title>
<style>
  :root{--bg:#111;--panel:#1b1b1f;--accent:#3b82f6;--text:#eee;--muted:#999;}
  *{box-sizing:border-box;}
  body{margin:0;font-family:system-ui,sans-serif;background:var(--bg);color:var(--text);}
  header{padding:16px;text-align:center;border-bottom:1px solid #222;}
  h1{margin:0;font-size:1.2rem;}
  .tabs{display:flex;justify-content:center;gap:8px;padding:12px;}
  .tab-btn{background:var(--panel);border:1px solid #333;color:var(--text);padding:8px 16px;border-radius:8px;cursor:pointer;}
  .tab-btn.active{background:var(--accent);border-color:var(--accent);}
  .tab-content{display:none;max-width:420px;margin:0 auto;padding:16px;}
  .tab-content.active{display:block;}
  .pad{display:grid;grid-template-columns:64px 64px 64px;grid-template-rows:64px 64px 64px;gap:8px;justify-content:center;margin:24px auto;}
  .pad button{font-size:1.4rem;border-radius:12px;border:1px solid #333;background:var(--panel);color:var(--text);cursor:pointer;}
  .pad button:active{background:#2a2a30;}
  #btn-up{grid-column:2;grid-row:1;}
  #btn-left{grid-column:1;grid-row:2;}
  #btn-center{grid-column:2;grid-row:2;font-weight:bold;}
  #btn-center:disabled{opacity:.35;cursor:not-allowed;}
  #btn-right{grid-column:3;grid-row:2;}
  #btn-down{grid-column:2;grid-row:3;}
  .led-row{display:flex;justify-content:center;gap:8px;margin:16px auto 0;}
  .led-row button{padding:10px 18px;border-radius:10px;border:1px solid #333;background:var(--panel);color:var(--text);cursor:pointer;}
  #btn-led-red.on{background:#ef4444;border-color:#ef4444;color:#2a0505;}
  #btn-led-yellow.on{background:#eab308;border-color:#eab308;color:#2a2205;}
  #btn-led-green.on{background:#22c55e;border-color:#22c55e;color:#04150a;}
  .status{text-align:center;color:var(--muted);font-size:.9rem;margin-top:8px;}
  form label{display:block;margin:12px 0 4px;font-size:.9rem;color:var(--muted);}
  input[type=text],input[type=password]{width:100%;padding:10px;border-radius:8px;border:1px solid #333;background:#0e0e11;color:var(--text);}
  button.primary{margin-top:16px;width:100%;padding:12px;border-radius:8px;border:none;background:var(--accent);color:#fff;font-size:1rem;cursor:pointer;}
  .info{background:var(--panel);border:1px solid #333;border-radius:8px;padding:12px;margin-bottom:12px;font-size:.9rem;}
  .msg{margin-top:12px;font-size:.9rem;text-align:center;}
  .placeholder{color:var(--muted);text-align:center;padding:32px 0;}
  .log-view{background:#0e0e11;border:1px solid #333;border-radius:8px;padding:10px;height:300px;overflow-y:auto;font-family:monospace;font-size:.8rem;white-space:pre-wrap;word-break:break-word;margin:0;}
</style>
</head>
<body>
<header><h1>CNC-2D &mdash; Pannello di controllo</h1></header>
<div class="tabs">
  <button class="tab-btn active" data-tab="controllo">Controllo</button>
  <button class="tab-btn" data-tab="wifi">Wi-Fi</button>
  <button class="tab-btn" data-tab="log">Log</button>
  <button class="tab-btn" data-tab="codice">Codice</button>
</div>

<section id="controllo" class="tab-content active">
  <div class="pad">
    <button id="btn-up" onclick="arrowPress('up')">&uarr;</button>
    <button id="btn-left" onclick="arrowPress('left')">&larr;</button>
    <button id="btn-center" disabled>LED</button>
    <button id="btn-right" onclick="arrowPress('right')">&rarr;</button>
    <button id="btn-down" onclick="arrowPress('down')">&darr;</button>
  </div>
  <div class="led-row">
    <button id="btn-led-red" onclick="toggleLed('red')">Rosso</button>
    <button id="btn-led-yellow" onclick="toggleLed('yellow')">Giallo</button>
    <button id="btn-led-green" onclick="toggleLed('green')">Verde</button>
  </div>
  <p class="status" id="led-status">Stato LED: --</p>
</section>

<section id="wifi" class="tab-content">
  <div class="info" id="wifi-info">Caricamento stato rete...</div>
  <form id="wifi-form">
    <label>Nome rete (SSID)</label>
    <input type="text" id="ssid" placeholder="Nome rete Wi-Fi" required>
    <label>Password</label>
    <input type="password" id="password" placeholder="Nuova password">
    <button type="submit" class="primary">Salva e riavvia</button>
  </form>
  <p class="msg" id="wifi-msg"></p>
</section>

<section id="log" class="tab-content">
  <pre id="log-view" class="log-view"></pre>
</section>

<section id="codice" class="tab-content">
  <div class="placeholder">
    Caricamento G-code via web: disponibile dopo il test di motori e servo (Step 2-3 della pipeline).
  </div>
</section>

<script>
function $(id){return document.getElementById(id);}

document.querySelectorAll('.tab-btn').forEach(function(btn){
  btn.addEventListener('click', function(){
    document.querySelectorAll('.tab-btn').forEach(function(b){b.classList.remove('active');});
    document.querySelectorAll('.tab-content').forEach(function(c){c.classList.remove('active');});
    btn.classList.add('active');
    $(btn.dataset.tab).classList.add('active');
  });
});

function arrowPress(dir){
  // Placeholder: nessuna azione finché i motori non sono collegati.
  console.log('arrow', dir);
}

function applyLedState(s){
  $('btn-led-red').classList.toggle('on', s.ledRed);
  $('btn-led-yellow').classList.toggle('on', s.ledYellow);
  $('btn-led-green').classList.toggle('on', s.ledGreen);
  $('led-status').textContent = 'LED — Rosso: ' + (s.ledRed ? 'ACCESO' : 'SPENTO') +
    ' | Giallo: ' + (s.ledYellow ? 'ACCESO' : 'SPENTO') +
    ' | Verde: ' + (s.ledGreen ? 'ACCESO' : 'SPENTO');
}

function refreshState(){
  fetch('/api/state').then(function(r){return r.json();}).then(function(s){
    applyLedState(s);
    $('wifi-info').innerHTML = 'Modalità: <b>' + s.mode + '</b><br>Rete: <b>' + s.ssid + '</b><br>IP: <b>' + s.ip + '</b>';
  }).catch(function(){});
}

function toggleLed(color){
  fetch('/api/led/' + color + '/toggle').then(function(r){return r.json();}).then(applyLedState);
}

function refreshLog(){
  fetch('/api/log').then(function(r){return r.text();}).then(function(t){
    var el = $('log-view');
    var atBottom = (el.scrollTop + el.clientHeight) >= (el.scrollHeight - 20);
    el.textContent = t;
    if (atBottom) el.scrollTop = el.scrollHeight;
  }).catch(function(){});
}

$('wifi-form').addEventListener('submit', function(e){
  e.preventDefault();
  var ssid = $('ssid').value;
  var password = $('password').value;
  $('wifi-msg').textContent = 'Salvataggio in corso...';
  fetch('/api/wifi/save', {
    method:'POST',
    headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password)
  }).then(function(r){return r.text();}).then(function(t){
    $('wifi-msg').textContent = t + ' Il dispositivo si riavvierà tra pochi secondi.';
  }).catch(function(){
    $('wifi-msg').textContent = 'Errore di rete durante il salvataggio.';
  });
});

refreshState();
setInterval(refreshState, 4000);
refreshLog();
setInterval(refreshLog, 2000);
</script>
</body>
</html>
)rawliteral";

void sendStateJson() {
  String json = "{\"ledRed\":" + String(ledRedState ? "true" : "false") +
                ",\"ledYellow\":" + String(ledYellowState ? "true" : "false") +
                ",\"ledGreen\":" + String(ledGreenState ? "true" : "false") +
                ",\"mode\":\"" + String(apMode ? "AP" : "STA") + "\"" +
                ",\"ssid\":\"" + (apMode ? String(AP_SSID) : WiFi.SSID()) + "\"" +
                ",\"ip\":\"" + (apMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString()) + "\"}";
  server.send(200, "application/json", json);
}

void handleRoot() {
  server.send_P(200, "text/html", PAGE_HTML);
}

void handleLedRedToggle() {
  ledRedState = !ledRedState;
  digitalWrite(RED_LED_PIN, ledRedState ? HIGH : LOW);
  sendStateJson();
}

void handleLedYellowToggle() {
  ledYellowState = !ledYellowState;
  digitalWrite(YELLOW_LED_PIN, ledYellowState ? HIGH : LOW);
  sendStateJson();
}

void handleLedGreenToggle() {
  ledGreenState = !ledGreenState;
  digitalWrite(GREEN_LED_PIN, ledGreenState ? HIGH : LOW);
  sendStateJson();
}

void handleState() {
  sendStateJson();
}

void handleLog() {
  server.send(200, "text/plain", logBuffer);
}

void handleWifiSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  if (ssid.length() == 0) {
    server.send(400, "text/plain", "SSID richiesto.");
    return;
  }
  prefs.putString("ssid", ssid);
  prefs.putString("pass", password);
  server.send(200, "text/plain", "Credenziali salvate.");
  delay(500);
  ESP.restart();
}

void handleNotFound() {
  server.send(404, "text/plain", "Non trovato");
}

bool connectToWifi(const String& ssid, const String& password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
  }
  return WiFi.status() == WL_CONNECTED;
}

void startSetupAP() {
  apMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
}

void setupOTA() {
  ArduinoOTA.setHostname("cnc2d");
  ArduinoOTA.setPassword(AP_PASS);
  ArduinoOTA.onStart([]() {
    logMsg("OTA: aggiornamento avviato...");
  });
  ArduinoOTA.onEnd([]() {
    logMsg("OTA: completato, riavvio.");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    logMsg("OTA errore [" + String((int)error) + "]");
  });
  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  prefs.begin("wifi_cfg", false);
  String savedSsid = prefs.getString("ssid", "");
  String savedPass = prefs.getString("pass", "");

  if (savedSsid.length() > 0 && connectToWifi(savedSsid, savedPass)) {
    apMode = false;
    logMsg("Connesso. IP: " + WiFi.localIP().toString());
    setupOTA();
    logMsg("mDNS/OTA attivi: http://cnc2d.local");
  } else {
    logMsg("Connessione Wi-Fi fallita o non configurata. Avvio modalita' configurazione.");
    startSetupAP();
    logMsg("Connettiti alla rete '" + String(AP_SSID) + "' e apri http://" + WiFi.softAPIP().toString());
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/led/red/toggle", HTTP_GET, handleLedRedToggle);
  server.on("/api/led/yellow/toggle", HTTP_GET, handleLedYellowToggle);
  server.on("/api/led/green/toggle", HTTP_GET, handleLedGreenToggle);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/log", HTTP_GET, handleLog);
  server.on("/api/wifi/save", HTTP_POST, handleWifiSave);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();
}
