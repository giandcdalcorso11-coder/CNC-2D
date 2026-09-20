# CNC-2D Dashboard — ESP32

Sketch Arduino per lo Step 1 della pipeline: dashboard web servita direttamente dall'ESP32,
con controllo del LED su D2 e configurazione Wi-Fi da browser (senza dover ricompilare).

## Hardware richiesto

- ESP32 alimentato via USB o batteria+interruttore (come da bring-up già completato)
- LED rosso + resistenza 220 ohm su D2 (GPIO2)
- LED verde + resistenza 220 ohm su D18 (GPIO18)
- LED giallo + resistenza 220 ohm su D19 (GPIO19)
- Driver A4988 con motore X: STEP su D4, DIR su D26, ENABLE su D27 (GPIO4/26/27)
- **Condensatore elettrolitico da 100 µF / 35 V (minimo 47 µF) tra VMOT e GND del driver**,
  il più vicino possibile al modulo. Non è opzionale: senza di esso i picchi induttivi
  generati dalle bobine a ogni commutazione possono distruggere l'A4988.

## Librerie

Nessuna installazione extra: `WiFi.h`, `WebServer.h`, `Preferences.h`, `ArduinoOTA.h` sono
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

Frecce sinistra/destra: pilotano il motore X collegato al driver A4988. Un click singolo
fa avanzare il motore di un singolo passo; tenendo premuto (dopo ~300ms) il motore continua
a girare finché non si rilascia il pulsante. Frecce su/giù: ancora placeholder, riservate
a un futuro asse Y/Z.

Pulsante centrale del joystick: disabilitato per ora, riservato a un futuro uso.

Pulsanti "Giro completo ←/→": fanno compiere al motore un giro completo (200 passi,
corrispondenti a 1,8°/passo in modalità full-step) nella direzione indicata.

Pulsanti Rosso/Giallo/Verde: accendono/spengono rispettivamente i LED su D2, D19, D18,
per verificare che la dashboard comunichi correttamente con l'ESP32 e per avere indicatori
di stato distinti quando inizieremo a pilotare i motori.

Nota sulla direzione: se sinistra/destra risultano invertite rispetto a quanto ti aspetti,
non serve toccare i cavi — è sufficiente scambiare la mappatura HIGH/LOW di `DIR_PIN` nel
codice (`setDir()`).

### Tab Motore (diagnostica)

Permette di variare i parametri di movimento senza ricompilare, per capire dove si rompe:

- **Intervallo tra i passi**: da 300 µs (veloce) a 200000 µs (lentissimo). Se il motore gira
  a 50 ms/passo ma stalla a 8 ms/passo, il limite è la coppia disponibile (quindi corrente,
  Vref, tensione di alimentazione) e non il cablaggio.
- **Numero di passi**: 200 passi = un giro completo in full-step.
- **Rampa di accelerazione**: i primi 40 passi partono 4 volte più lenti del valore impostato
  e accelerano linearmente. Senza rampa un motore fermo può non riuscire ad agganciarsi
  alla frequenza di partenza e si limita a vibrare.
- **Driver abilitato**: agisce direttamente sul pin ENABLE. Serve anche come verifica:
  togliendo la spunta il motore deve sbloccarsi (si gira a mano liberamente). Se resta
  bloccato, il segnale ENABLE non sta arrivando al driver.

Ogni comando viene registrato nella tab Log, con il conteggio dei passi effettivamente
emessi a fine movimento: se il log dice "200 passi emessi" ma l'albero non ha fatto un giro,
il problema è meccanico/elettrico a valle del driver, non nel firmware.

### Calcolo del Vref (importante)

La corrente per fase impostata dal trimmer dipende dalle resistenze di shunt montate sul
modulo, che variano tra produttori:

    Vref = corrente_per_fase × 8 × R_shunt

I valori sono stampati sui due piccoli componenti SMD vicini al bordo del modulo:
`R050` = 0,05 Ω, `R068` = 0,068 Ω (Pololu), `R100` = 0,1 Ω, `R200` = 0,2 Ω.

Per un motore da 0,7 A/fase: 0,38 V con R068, 0,56 V con R100, **1,12 V con R200**.
Usare il valore sbagliato significa alimentare il motore a metà (o al doppio) della
corrente prevista — nel primo caso il motore fa singoli passi ma stalla appena si prova
a farlo girare in modo continuo.

## Aggiornamenti firmware via Wi-Fi (OTA)

Dopo il primo caricamento via USB, i successivi aggiornamenti del firmware possono essere
fatti via Wi-Fi, senza cavo: una volta che l'ESP32 è connesso alla rete di casa, in Arduino
IDE vai su Strumenti → Porta e seleziona la voce di rete (tipo "cnc2d at 192.168.x.x").
Premi Upload come al solito: ti verrà chiesta la password OTA, che è la stessa
dell'Access Point di emergenza (`cnc2d2026`).

Nota: l'OTA funziona solo quando l'ESP32 è già connesso alla rete di casa (modalità STA).
Se è in modalità `CNC-2D-Setup` (nessuna rete configurata o credenziali errate), serve
ancora il cavo USB per il primo caricamento.

### Tab Log

Mostra in tempo reale (aggiornamento ogni 2 secondi) gli stessi messaggi che finora
si vedevano solo sul Serial Monitor via USB — utile soprattutto dopo il primo aggiornamento
OTA, quando il Serial Monitor di Arduino IDE non è più disponibile perché la connessione
è via Wi-Fi e non via cavo. Il log tiene in memoria solo le ultime righe (circa 4000
caratteri): non è un log persistente, si azzera a ogni riavvio dell'ESP32.

### Tab Wi-Fi

Permette di cambiare SSID/password in qualsiasi momento, senza dover ricollegare l'ESP32
al PC. Se le nuove credenziali (o una password di casa cambiata) impediscono la connessione,
l'ESP32 torna automaticamente in modalità `CNC-2D-Setup` al riavvio successivo, così la
pagina resta sempre raggiungibile per correggerle.

### Tab Codice

Placeholder per ora. Il piano è di usarla in futuro per caricare direttamente un file G-code
dal browser, una volta validati motori (Step 2) e servo/pen-lift (Step 3) — non richiederà
un compilatore a bordo, solo l'upload del file che verrà poi eseguito dal firmware.
