// CNC-2D — Dashboard web ESP32 (Step 1-2: LED, Wi-Fi via web, OTA, log, controllo motori X/Y)
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

#define STEP_X_PIN 4
#define DIR_X_PIN 26
#define STEP_Y_PIN 25
#define DIR_Y_PIN 33
#define ENABLE_PIN 27 // condiviso tra i due driver

#define WIFI_CONNECT_TIMEOUT_MS 15000
#define STEPS_PER_REV 200

const unsigned long STEP_PULSE_US = 10;
const unsigned long DIR_SETUP_US = 20;
const unsigned long STEP_INTERVAL_MIN_US = 300;
const unsigned long STEP_INTERVAL_MAX_US = 200000;
const long RAMP_LEN = 40; // passi su cui si distribuisce la rampa di accelerazione

const char* AP_SSID = "CNC-2D-Setup";
const char* AP_PASS = "cnc2d2026";

Preferences prefs;
WebServer server(80);

bool ledRedState = false;
bool ledYellowState = false;
bool ledGreenState = false;
bool apMode = false;

// stepsRemaining: 0 = fermo, >0 = passi ancora da fare, -1 = rotazione continua.
struct Motor {
  const char* name;
  uint8_t stepPin;
  uint8_t dirPin;
  long stepsRemaining;
  long stepsDone;
  int dir;
  unsigned long lastStepMicros;
};

Motor motorX = {"X", STEP_X_PIN, DIR_X_PIN, 0, 0, 1, 0};
Motor motorY = {"Y", STEP_Y_PIN, DIR_Y_PIN, 0, 0, 1, 0};

unsigned long stepIntervalUs = 15000;
bool rampEnabled = true;
bool driverEnabled = true;

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

void setDriverEnabled(bool on) {
  driverEnabled = on;
  digitalWrite(ENABLE_PIN, on ? LOW : HIGH); // A4988: ENABLE attivo basso
}

// DIR va impostato una sola volta a inizio movimento, non ad ogni passo:
// l'A4988 richiede che sia stabile prima del fronte di salita di STEP.
void setDir(Motor& m, int dir) {
  m.dir = dir;
  digitalWrite(m.dirPin, dir > 0 ? HIGH : LOW);
  delayMicroseconds(DIR_SETUP_US);
}

void pulseStep(Motor& m) {
  digitalWrite(m.stepPin, HIGH);
  delayMicroseconds(STEP_PULSE_US);
  digitalWrite(m.stepPin, LOW);
}

// Con la rampa attiva i primi RAMP_LEN passi partono 4 volte più lenti
// del target e accelerano linearmente fino ad esso.
unsigned long currentIntervalUs(const Motor& m) {
  if (!rampEnabled || m.stepsDone >= RAMP_LEN) return stepIntervalUs;
  return stepIntervalUs * (RAMP_LEN + 3 * (RAMP_LEN - m.stepsDone)) / RAMP_LEN;
}

void startMove(Motor& m, int dir, long steps) {
  setDir(m, dir);
  m.stepsDone = 0;
  m.stepsRemaining = steps;
  m.lastStepMicros = micros();
}

void serviceMotor(Motor& m) {
  if (m.stepsRemaining == 0 || !driverEnabled) return;

  unsigned long now = micros();
  if (now - m.lastStepMicros < currentIntervalUs(m)) return;

  m.lastStepMicros = now;
  pulseStep(m);
  m.stepsDone++;

  if (m.stepsRemaining > 0) {
    m.stepsRemaining--;
    if (m.stepsRemaining == 0) {
      logMsg("Motore " + String(m.name) + ": movimento completato, " + String(m.stepsDone) + " passi emessi");
    }
  }
}

void serviceMotors() {
  serviceMotor(motorX);
  serviceMotor(motorY);
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
  .tabs{display:flex;justify-content:center;gap:8px;padding:12px;flex-wrap:wrap;}
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
  .led-row,.motor-row{display:flex;justify-content:center;gap:8px;margin:12px auto 0;}
  .led-row button,.motor-row button{padding:10px 16px;border-radius:10px;border:1px solid #333;background:var(--panel);color:var(--text);cursor:pointer;}
  #btn-led-red.on{background:#ef4444;border-color:#ef4444;color:#2a0505;}
  #btn-led-yellow.on{background:#eab308;border-color:#eab308;color:#2a2205;}
  #btn-led-green.on{background:#22c55e;border-color:#22c55e;color:#04150a;}
  .status{text-align:center;color:var(--muted);font-size:.9rem;margin-top:8px;}
  form label,.field label{display:block;margin:12px 0 4px;font-size:.9rem;color:var(--muted);}
  input[type=text],input[type=password],input[type=number],select{width:100%;padding:10px;border-radius:8px;border:1px solid #333;background:#0e0e11;color:var(--text);}
  button.primary{margin-top:16px;width:100%;padding:12px;border-radius:8px;border:none;background:var(--accent);color:#fff;font-size:1rem;cursor:pointer;}
  button.ghost{padding:8px 12px;border-radius:8px;border:1px solid #333;background:var(--panel);color:var(--text);cursor:pointer;font-size:.85rem;}
  .presets{display:flex;gap:6px;flex-wrap:wrap;margin-top:8px;}
  .info{background:var(--panel);border:1px solid #333;border-radius:8px;padding:12px;margin-bottom:12px;font-size:.9rem;line-height:1.5;}
  .msg{margin-top:12px;font-size:.9rem;text-align:center;}
  .placeholder{color:var(--muted);text-align:center;padding:32px 0;}
  .log-view{background:#0e0e11;border:1px solid #333;border-radius:8px;padding:10px;height:300px;overflow-y:auto;font-family:monospace;font-size:.8rem;white-space:pre-wrap;word-break:break-word;margin:0;}
  .check{display:flex;align-items:center;gap:8px;margin-top:12px;font-size:.9rem;color:var(--muted);}
  .hint{font-size:.8rem;color:var(--muted);margin-top:6px;line-height:1.4;}
</style>
</head>
<body>
<header><h1>CNC-2D &mdash; Pannello di controllo</h1></header>
<div class="tabs">
  <button class="tab-btn active" data-tab="controllo">Controllo</button>
  <button class="tab-btn" data-tab="motore">Motore</button>
  <button class="tab-btn" data-tab="wifi">Wi-Fi</button>
  <button class="tab-btn" data-tab="log">Log</button>
  <button class="tab-btn" data-tab="codice">Codice</button>
</div>

<section id="controllo" class="tab-content active">
  <div class="pad">
    <button id="btn-up"
      onmousedown="pressStart('y','up')" onmouseup="pressEnd('y')" onmouseleave="pressEnd('y')"
      ontouchstart="pressStart('y','up')" ontouchend="pressEnd('y')">&uarr;</button>
    <button id="btn-left"
      onmousedown="pressStart('x','left')" onmouseup="pressEnd('x')" onmouseleave="pressEnd('x')"
      ontouchstart="pressStart('x','left')" ontouchend="pressEnd('x')">&larr;</button>
    <button id="btn-center" disabled>LED</button>
    <button id="btn-right"
      onmousedown="pressStart('x','right')" onmouseup="pressEnd('x')" onmouseleave="pressEnd('x')"
      ontouchstart="pressStart('x','right')" ontouchend="pressEnd('x')">&rarr;</button>
    <button id="btn-down"
      onmousedown="pressStart('y','down')" onmouseup="pressEnd('y')" onmouseleave="pressEnd('y')"
      ontouchstart="pressStart('y','down')" ontouchend="pressEnd('y')">&darr;</button>
  </div>
  <div class="led-row">
    <button id="btn-led-red" onclick="toggleLed('red')">Rosso</button>
    <button id="btn-led-yellow" onclick="toggleLed('yellow')">Giallo</button>
    <button id="btn-led-green" onclick="toggleLed('green')">Verde</button>
  </div>
  <div class="motor-row">
    <button onclick="fullTurn('x','left')">X: giro &larr;</button>
    <button onclick="fullTurn('x','right')">X: giro &rarr;</button>
  </div>
  <div class="motor-row">
    <button onclick="fullTurn('y','down')">Y: giro &darr;</button>
    <button onclick="fullTurn('y','up')">Y: giro &uarr;</button>
  </div>
  <div class="motor-row">
    <button onclick="motorStop()" style="border-color:#ef4444;color:#ef4444">STOP</button>
  </div>
  <p class="status" id="led-status">Stato LED: --</p>
  <p class="status" id="motor-status">Motori: --</p>
</section>

<section id="motore" class="tab-content">
  <div class="info" id="motor-info">Motori: --</div>

  <div class="field">
    <label>Asse su cui agire</label>
    <select id="axis">
      <option value="x">X (STEP D4 / DIR D26)</option>
      <option value="y">Y (STEP D25 / DIR D33)</option>
      <option value="both">Entrambi insieme</option>
    </select>
  </div>

  <div class="field">
    <label>Intervallo tra i passi (microsecondi)</label>
    <input type="number" id="step-us" min="300" max="200000" step="100" value="15000">
    <div class="presets">
      <button class="ghost" onclick="setUs(50000)">50 ms</button>
      <button class="ghost" onclick="setUs(25000)">25 ms</button>
      <button class="ghost" onclick="setUs(15000)">15 ms</button>
      <button class="ghost" onclick="setUs(8000)">8 ms</button>
      <button class="ghost" onclick="setUs(4000)">4 ms</button>
      <button class="ghost" onclick="setUs(2000)">2 ms</button>
    </div>
    <p class="hint">Valori alti = motore lento e con pi&ugrave; coppia. L'intervallo vale per entrambi gli assi.</p>
  </div>

  <div class="field">
    <label>Numero di passi</label>
    <input type="number" id="step-count" min="1" max="20000" value="200">
  </div>

  <div class="check">
    <input type="checkbox" id="ramp" checked onchange="pushConfig()">
    <label for="ramp" style="margin:0">Rampa di accelerazione (primi 40 passi pi&ugrave; lenti)</label>
  </div>

  <div class="check">
    <input type="checkbox" id="drv-enabled" checked onchange="pushEnable()">
    <label for="drv-enabled" style="margin:0">Driver abilitati (pin ENABLE basso, condiviso X/Y)</label>
  </div>
  <p class="hint">Togliendo la spunta entrambi i motori si sbloccano e smettono di scaldare.</p>

  <div class="field">
    <label>Test pin &mdash; forza un livello fisso e misuralo col multimetro sul pin del driver</label>
    <div class="presets">
      <button class="ghost" onclick="setPin('stepx',0)">STEP X 0 V</button>
      <button class="ghost" onclick="setPin('stepx',1)">STEP X 3,3 V</button>
      <button class="ghost" onclick="setPin('dirx',0)">DIR X 0 V</button>
      <button class="ghost" onclick="setPin('dirx',1)">DIR X 3,3 V</button>
    </div>
    <div class="presets">
      <button class="ghost" onclick="setPin('stepy',0)">STEP Y 0 V</button>
      <button class="ghost" onclick="setPin('stepy',1)">STEP Y 3,3 V</button>
      <button class="ghost" onclick="setPin('diry',0)">DIR Y 0 V</button>
      <button class="ghost" onclick="setPin('diry',1)">DIR Y 3,3 V</button>
    </div>
    <div class="presets">
      <button class="ghost" onclick="setPin('en',0)">EN 0 V</button>
      <button class="ghost" onclick="setPin('en',1)">EN 3,3 V</button>
    </div>
    <p class="hint">Puntale nero su un GND, puntale rosso sul pin del driver. Dopo aver usato questi
      pulsanti riporta gli STEP a 0 V, altrimenti il primo impulso successivo va perso.</p>
  </div>

  <button class="primary" onclick="runTest('pos')">Esegui verso &rarr; / &uarr;</button>
  <button class="primary" onclick="runTest('neg')" style="margin-top:8px">Esegui verso &larr; / &darr;</button>
  <button class="primary" onclick="motorStop()" style="margin-top:8px;background:#ef4444">STOP</button>
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

var holdTimer = null;
var holding = null;

function pressStart(axis, dir){
  holding = null;
  fetch('/api/motor/step?axis=' + axis + '&dir=' + dir);
  holdTimer = setTimeout(function(){
    holding = axis;
    fetch('/api/motor/start?axis=' + axis + '&dir=' + dir);
  }, 300);
}

function pressEnd(axis){
  clearTimeout(holdTimer);
  if (holding === axis) {
    fetch('/api/motor/stop?axis=' + axis);
    holding = null;
  }
}

function fullTurn(axis, dir){
  fetch('/api/motor/full?axis=' + axis + '&dir=' + dir);
}

function setUs(v){
  $('step-us').value = v;
  pushConfig();
}

function pushConfig(){
  fetch('/api/motor/config?us=' + $('step-us').value + '&ramp=' + ($('ramp').checked ? 1 : 0));
}

function pushEnable(){
  fetch('/api/motor/enable?on=' + ($('drv-enabled').checked ? 1 : 0));
}

function setPin(pin, level){
  fetch('/api/pin/set?pin=' + pin + '&level=' + level);
}

function runTest(sign){
  pushConfig();
  var dir = (sign === 'pos') ? 'right' : 'left';
  fetch('/api/motor/run?axis=' + $('axis').value + '&dir=' + dir + '&steps=' + $('step-count').value);
}

function motorStop(){
  fetch('/api/motor/stop?axis=both');
}

function axisDesc(a){
  return a.done + ' passi, ' + (a.remaining < 0 ? 'continuo' : a.remaining + ' mancanti');
}

function applyState(s){
  $('btn-led-red').classList.toggle('on', s.ledRed);
  $('btn-led-yellow').classList.toggle('on', s.ledYellow);
  $('btn-led-green').classList.toggle('on', s.ledGreen);
  $('led-status').textContent = 'LED — Rosso: ' + (s.ledRed ? 'ACCESO' : 'SPENTO') +
    ' | Giallo: ' + (s.ledYellow ? 'ACCESO' : 'SPENTO') +
    ' | Verde: ' + (s.ledGreen ? 'ACCESO' : 'SPENTO');

  var moving = (s.x.remaining !== 0 || s.y.remaining !== 0);
  $('motor-status').textContent = 'Motori: ' + (moving ? 'in movimento' : 'fermi') +
    ' | driver ' + (s.enabled ? 'abilitati' : 'disabilitati');
  $('motor-info').innerHTML =
    'Driver: <b>' + (s.enabled ? 'abilitati' : 'disabilitati') + '</b><br>' +
    'Intervallo: <b>' + s.stepUs + ' us</b> — Rampa: <b>' + (s.ramp ? 'on' : 'off') + '</b><br>' +
    'X: ' + axisDesc(s.x) + '<br>' +
    'Y: ' + axisDesc(s.y);

  if (document.activeElement !== $('step-us')) $('step-us').value = s.stepUs;
  $('ramp').checked = s.ramp;
  $('drv-enabled').checked = s.enabled;

  $('wifi-info').innerHTML = 'Modalità: <b>' + s.mode + '</b><br>Rete: <b>' + s.ssid + '</b><br>IP: <b>' + s.ip + '</b>';
}

function refreshState(){
  fetch('/api/state').then(function(r){return r.json();}).then(applyState).catch(function(){});
}

function toggleLed(color){
  fetch('/api/led/' + color + '/toggle').then(function(r){return r.json();}).then(applyState);
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
setInterval(refreshState, 2000);
refreshLog();
setInterval(refreshLog, 3000);
</script>
</body>
</html>
)rawliteral";

String axisJson(const Motor& m) {
  return "{\"remaining\":" + String(m.stepsRemaining) + ",\"done\":" + String(m.stepsDone) + "}";
}

void sendStateJson() {
  String json = "{\"ledRed\":" + String(ledRedState ? "true" : "false") +
                ",\"ledYellow\":" + String(ledYellowState ? "true" : "false") +
                ",\"ledGreen\":" + String(ledGreenState ? "true" : "false") +
                ",\"enabled\":" + String(driverEnabled ? "true" : "false") +
                ",\"ramp\":" + String(rampEnabled ? "true" : "false") +
                ",\"stepUs\":" + String(stepIntervalUs) +
                ",\"x\":" + axisJson(motorX) +
                ",\"y\":" + axisJson(motorY) +
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
  logMsg(String("LED rosso: ") + (ledRedState ? "ACCESO" : "SPENTO"));
  sendStateJson();
}

void handleLedYellowToggle() {
  ledYellowState = !ledYellowState;
  digitalWrite(YELLOW_LED_PIN, ledYellowState ? HIGH : LOW);
  logMsg(String("LED giallo: ") + (ledYellowState ? "ACCESO" : "SPENTO"));
  sendStateJson();
}

void handleLedGreenToggle() {
  ledGreenState = !ledGreenState;
  digitalWrite(GREEN_LED_PIN, ledGreenState ? HIGH : LOW);
  logMsg(String("LED verde: ") + (ledGreenState ? "ACCESO" : "SPENTO"));
  sendStateJson();
}

void handleState() {
  sendStateJson();
}

void handleLog() {
  server.send(200, "text/plain", logBuffer);
}

// "right" e "up" sono il verso positivo, tutto il resto è negativo.
int dirFromArg() {
  String d = server.arg("dir");
  return (d == "right" || d == "up") ? 1 : -1;
}

bool wantsX() {
  String a = server.arg("axis");
  return a != "y";
}

bool wantsY() {
  String a = server.arg("axis");
  return a == "y" || a == "both";
}

void handleMotorStep() {
  int d = dirFromArg();
  if (wantsX()) { setDir(motorX, d); pulseStep(motorX); }
  if (wantsY()) { setDir(motorY, d); pulseStep(motorY); }
  server.send(200, "text/plain", "ok");
}

void handleMotorStart() {
  int d = dirFromArg();
  logMsg("Motore " + server.arg("axis") + ": avvio continuo verso " + (d > 0 ? "+" : "-") +
         " a " + String(stepIntervalUs) + " us/passo");
  if (wantsX()) startMove(motorX, d, -1);
  if (wantsY()) startMove(motorY, d, -1);
  server.send(200, "text/plain", "ok");
}

void handleMotorStop() {
  if (wantsX()) motorX.stepsRemaining = 0;
  if (wantsY()) motorY.stepsRemaining = 0;
  logMsg("Motori: stop (X " + String(motorX.stepsDone) + " passi, Y " + String(motorY.stepsDone) + " passi)");
  server.send(200, "text/plain", "ok");
}

void handleMotorFull() {
  int d = dirFromArg();
  logMsg("Motore " + server.arg("axis") + ": giro completo verso " + (d > 0 ? "+" : "-") +
         " (" + String(STEPS_PER_REV) + " passi a " + String(stepIntervalUs) + " us/passo)");
  if (wantsX()) startMove(motorX, d, STEPS_PER_REV);
  if (wantsY()) startMove(motorY, d, STEPS_PER_REV);
  server.send(200, "text/plain", "ok");
}

void handleMotorRun() {
  int d = dirFromArg();
  long steps = server.arg("steps").toInt();
  if (steps < 1) steps = 1;
  if (steps > 20000) steps = 20000;
  logMsg("Test " + server.arg("axis") + ": " + String(steps) + " passi verso " + (d > 0 ? "+" : "-") +
         " a " + String(stepIntervalUs) + " us/passo, rampa " + (rampEnabled ? "on" : "off"));
  if (wantsX()) startMove(motorX, d, steps);
  if (wantsY()) startMove(motorY, d, steps);
  server.send(200, "text/plain", "ok");
}

void handleMotorConfig() {
  unsigned long us = (unsigned long)server.arg("us").toInt();
  if (us < STEP_INTERVAL_MIN_US) us = STEP_INTERVAL_MIN_US;
  if (us > STEP_INTERVAL_MAX_US) us = STEP_INTERVAL_MAX_US;
  stepIntervalUs = us;
  rampEnabled = (server.arg("ramp") == "1");
  logMsg("Motori: intervallo " + String(stepIntervalUs) + " us, rampa " + (rampEnabled ? "on" : "off"));
  server.send(200, "text/plain", "ok");
}

void handleMotorEnable() {
  bool on = (server.arg("on") == "1");
  setDriverEnabled(on);
  logMsg(String("Driver A4988: ") + (on ? "abilitati (ENABLE basso)" : "disabilitati (ENABLE alto)"));
  server.send(200, "text/plain", "ok");
}

// Forza un singolo pin a un livello fisso, per poterlo misurare col multimetro
// direttamente sul pin del driver. Qualsiasi movimento in corso viene fermato.
void handlePinSet() {
  String pin = server.arg("pin");
  bool high = (server.arg("level") == "1");
  motorX.stepsRemaining = 0;
  motorY.stepsRemaining = 0;

  if (pin == "stepx") {
    digitalWrite(STEP_X_PIN, high ? HIGH : LOW);
  } else if (pin == "dirx") {
    digitalWrite(DIR_X_PIN, high ? HIGH : LOW);
  } else if (pin == "stepy") {
    digitalWrite(STEP_Y_PIN, high ? HIGH : LOW);
  } else if (pin == "diry") {
    digitalWrite(DIR_Y_PIN, high ? HIGH : LOW);
  } else if (pin == "en") {
    setDriverEnabled(!high); // ENABLE attivo basso: livello alto = driver disabilitati
  } else {
    server.send(400, "text/plain", "pin sconosciuto");
    return;
  }

  logMsg("Test pin: " + pin + " forzato a " + (high ? "3.3 V (alto)" : "0 V (basso)"));
  server.send(200, "text/plain", "ok");
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
    motorX.stepsRemaining = 0; // niente passi durante un aggiornamento firmware
    motorY.stepsRemaining = 0;
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

  // I pin STEP vanno portati a livello basso il prima possibile: finché restano
  // ingressi flottanti raccolgono rumore e il driver lo interpreta come passi.
  pinMode(STEP_X_PIN, OUTPUT);
  pinMode(STEP_Y_PIN, OUTPUT);
  digitalWrite(STEP_X_PIN, LOW);
  digitalWrite(STEP_Y_PIN, LOW);
  pinMode(DIR_X_PIN, OUTPUT);
  pinMode(DIR_Y_PIN, OUTPUT);
  digitalWrite(DIR_X_PIN, LOW);
  digitalWrite(DIR_Y_PIN, LOW);
  pinMode(ENABLE_PIN, OUTPUT);
  setDriverEnabled(true);

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
  server.on("/api/motor/step", HTTP_GET, handleMotorStep);
  server.on("/api/motor/start", HTTP_GET, handleMotorStart);
  server.on("/api/motor/stop", HTTP_GET, handleMotorStop);
  server.on("/api/motor/full", HTTP_GET, handleMotorFull);
  server.on("/api/motor/run", HTTP_GET, handleMotorRun);
  server.on("/api/motor/config", HTTP_GET, handleMotorConfig);
  server.on("/api/motor/enable", HTTP_GET, handleMotorEnable);
  server.on("/api/pin/set", HTTP_GET, handlePinSet);
  server.on("/api/wifi/save", HTTP_POST, handleWifiSave);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  // Il movimento ha priorità sul web: server.handleClient() può bloccare anche per
  // centinaia di millisecondi, quindi i passi vanno emessi prima e dopo, non solo dopo.
  serviceMotors();
  server.handleClient();
  ArduinoOTA.handle();
  serviceMotors();
}
