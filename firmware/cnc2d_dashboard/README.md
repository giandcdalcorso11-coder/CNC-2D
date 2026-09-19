# CNC-2D Dashboard — ESP32

Sketch Arduino per lo Step 1 della pipeline: dashboard web servita direttamente dall'ESP32,
con controllo del LED su D2 e configurazione Wi-Fi da browser (senza dover ricompilare).

## Hardware richiesto

- ESP32 alimentato via USB o batteria+interruttore (come da bring-up già completato)
- LED + resistenza 220 ohm su D2 (GPIO2)

## Librerie

Nessuna installazione extra: `WiFi.h`, `WebServer.h`, `Preferences.h`, `ESPmDNS.h` sono
incluse nel core ESP32 per Arduino (Boards Manager → "esp32" di Espressif).

## Primo upload

Il primo caricamento del firmware richiede sempre il cavo USB (come previsto dalla vision
del progetto). Seleziona la board ESP32 corretta (es. "ESP32 Dev Module"), la porta seriale,
e carica lo sketch.

## Primo avvio (nessuna rete Wi-Fi salvata)

Alla primissima accensione l'ESP32 non ha ancora credenziali Wi-Fi salvate, quindi apre
un proprio Access Point di emergenza:

- SSID: `CNC-2D-Setup`
- Password: `cnc2d2026`

Collegati a questa rete da telefono o PC e apri `http://192.168.4.1` (indirizzo mostrato
anche sul Serial Monitor a 115200 baud). Vai nella tab **Wi-Fi**, inserisci nome e password
della rete di casa e premi "Salva e riavvia": l'ESP32 salva le credenziali in memoria non
volatile e si riavvia connettendosi alla rete di casa.

## Uso normale (connesso alla rete di casa)

Una volta collegato alla rete di casa, la dashboard è raggiungibile da qualsiasi dispositivo
sulla stessa rete su:

- `http://cnc2d.local` (via mDNS, consigliato — resta valido anche se cambia l'IP)
- oppure l'IP mostrato sul Serial Monitor / nella pagina del router

### Tab Controllo

Frecce su/giù/sx/dx: presenti ma non collegate a nessuna funzione per ora (verranno
usate per il controllo assi X/Y quando saranno pronti i driver A4988, Step 2).

Pulsante centrale: accende/spegne il LED su D2, per verificare che la dashboard comunichi
correttamente con l'ESP32.

### Tab Wi-Fi

Permette di cambiare SSID/password in qualsiasi momento, senza dover ricollegare l'ESP32
al PC. Se le nuove credenziali (o una password di casa cambiata) impediscono la connessione,
l'ESP32 torna automaticamente in modalità `CNC-2D-Setup` al riavvio successivo, così la
pagina resta sempre raggiungibile per correggerle.

### Tab Codice

Placeholder per ora. Il piano è di usarla in futuro per caricare direttamente un file G-code
dal browser, una volta validati motori (Step 2) e servo/pen-lift (Step 3) — non richiederà
un compilatore a bordo, solo l'upload del file che verrà poi eseguito dal firmware.
