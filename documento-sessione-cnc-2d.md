# Documento di Sessione — CNC 2D Plotter

**Versione:** 3
**Ultimo aggiornamento:** 2026-09-20 09:45

## Vision

Macchina CNC 2D per disegno/plotter, con:
- Assi X/Y motorizzati (NEMA17 modello 17HS4023 + driver A4988)
- Asse Z tramite servomotore per alzare/abbassare la penna
- Controllo firmware via ESP32 con FluidNC
- Invio G-code via Wi-Fi (WebUI di FluidNC), senza necessità di cavo USB durante il disegno
- In prospettiva, un software "stile slicer" per convertire SVG/immagini in G-code e caricarlo sulla macchina

## Pipeline

### Step 1 — Bring-up alimentazione e logica ESP32

**Stato:** completato

**Obiettivo:** ESP32 funzionante e alimentabile sia da USB che da batteria, con GPIO verificati

**Decisioni progettuali:**
- Ramo alimentazione logica (5V, ESP32) separato dal ramo 24V dei motori
- Batteria 18650 + modulo boost/carica 5V Type-C (supporta "discharge while charging", quindi utilizzabile anche mentre è in carica)
- Interruttore SPST inserito solo sulla linea 5V+ verso il pin VIN dell'ESP32 (non sulla batteria), per non interrompere la ricarica quando la macchina è spenta
- GND comune tra guida breadboard, modulo batteria ed ESP32, verificato in continuità

**Criterio di completamento:** LED collegato a D2 lampeggia correttamente sia alimentato da USB sia da batteria+interruttore

**Note (cronologia dello step):**
- [2026-09-19] Verificati componenti hardware (batteria NASTIMA 12V, motori 17HS4023, driver A4988, alimentatore 24V riciclato da striscia LED) tramite confronto con i link Amazon e datasheet
- [2026-09-19] Test ESP32 standalone via USB-C con sketch Blink su D2 (GPIO2) e resistenza 220 ohm — riuscito
- [2026-09-19] Aggiunto modulo batteria 18650 (boost 5V + carica Type-C) e interruttore sulla linea 5V+ verso VIN — testato e funzionante

#### Step 1.1 — Dashboard web di controllo e test

**Stato:** completato

**Obiettivo:** dashboard web autonoma ospitata direttamente sull'ESP32, raggiungibile dalla rete Wi-Fi di casa, con un controllo di test sul LED e configurazione Wi-Fi senza dover ricompilare/riflashare

**Decisioni progettuali:**
- Web server nativo (`WebServer.h`, incluso nel core ESP32) invece di `ESPAsyncWebServer`, per non richiedere librerie esterne da installare
- Credenziali Wi-Fi salvate in NVS (`Preferences`) e configurabili da una tab dedicata nella dashboard; fallback automatico ad Access Point di emergenza (`CNC-2D-Setup`) se la connessione alla rete salvata fallisce, così la pagina resta sempre raggiungibile per correggerle
- mDNS (`cnc2d.local`) per raggiungere la dashboard senza dipendere dall'IP assegnato dal router
- Frecce direzionali presenti in UI ma non collegate a nessuna funzione, in previsione del controllo assi X/Y (Step 2)
- Tab "Codice" lasciata come placeholder: in futuro servirà per caricare direttamente un file G-code dal browser (non per compilare sketch — l'ESP32 non può compilare C++ da solo), da attivare dopo aver validato motori (Step 2) e servo/pen-lift (Step 3)

**Criterio di completamento:** dashboard raggiungibile da browser sulla rete di casa, pulsante centrale che accende/spegne il LED su D2 in tempo reale, funzionamento verificato sia con ESP32 alimentato da USB sia da batteria+interruttore

**Note (cronologia dello step):**
- [2026-09-19] Implementato sketch `firmware/cnc2d_dashboard/cnc2d_dashboard.ino` con web server, pagina a 3 tab (Controllo, Wi-Fi, Codice), salvataggio credenziali Wi-Fi su NVS e fallback ad AP di emergenza
- [2026-09-19] Bug: errore di compilazione "unterminated raw string" — causato da copia-incolla manuale del codice in un nuovo sketch anziché apertura diretta del file `.ino`, con probabile perdita dell'ultima riga di chiusura della stringa raw; risolto scaricando il file raw direttamente da GitHub
- [2026-09-19] Test completo riuscito: provisioning Wi-Fi di casa dalla tab dedicata, dashboard raggiungibile su `http://cnc2d.local`, LED su D2 controllato correttamente sia da alimentazione USB sia da batteria 18650+interruttore
- [2026-09-20] Aggiunti alla dashboard: 2 nuovi LED di stato (verde su D18, giallo su D19) con pulsanti dedicati Rosso/Giallo/Verde sotto le frecce (il pulsante centrale del joystick è stato disabilitato, non usa più il LED); supporto ArduinoOTA per aggiornare il firmware via Wi-Fi dopo il primo upload USB; tab "Log" che mostra in tempo reale gli stessi messaggi del Serial Monitor (utile perché il Serial Monitor via USB non è più disponibile quando si aggiorna via OTA)
- [2026-09-20] Frecce sinistra/destra collegate al motore X (STEP/DIR/ENABLE): click singolo = un passo, pressione prolungata = rotazione continua non bloccante; aggiunti pulsanti "Giro completo ←/→" (200 passi) per test rapido — dettagli del cablaggio e del debug nello Step 2

### Step 2 — Bring-up driver A4988 e motori (X/Y)

**Stato:** in corso

**Obiettivo:** un motore NEMA17 pilotato correttamente da un driver A4988 tramite ESP32, poi due motori in parallelo

**Decisioni progettuali:**
- Vref target ~0.5-0.56V per 0.7A per fase (corrente nominale reale del 17HS4023, non 1.0A come inizialmente indicato da una fonte esterna consultata)
- Alimentatore 24V/1.5A riciclato da striscia LED considerato sufficiente per 1-2 motori, margine stretto oltre — da monitorare la corrente assorbita durante il test
- Pin driver: STEP su D4, DIR su D26 (era D16), ENABLE su D27 (era D17) — spostati da D16/D17 per un problema di instabilità non ancora confermato con certezza (vedi bug nello storico sessioni)
- Bobine motore identificate via continuità: bobina 1 = fili rosso+blu, bobina 2 = fili nero+verde; mappate sul driver rispettivamente su 1A/1B e 2A/2B (l'assegnazione "1"/"2" rispetto ai colori è invertita rispetto al primo tentativo ma elettricamente corretta, non causa malfunzionamenti — cambia solo il verso di rotazione)
- Alimentatore 24V riciclato da striscia LED RGB: verificato con multimetro che il filo bianco è il vero 24V+ e il filo rosso è il GND (contro-intuitivo rispetto al colore); i fili blu e verde sono ridondanti (stesso nodo del bianco) e isolati, non utilizzati
- Scheda "Nano Terminal Adapter" (pensata per Arduino Nano) riadattata come morsettiera generica per i collegamenti di potenza (24V e fili motore), sfruttando il fatto che è puramente passiva (screw terminal + header pin sullo stesso nodo) — alternativa scartata: morsetti a vite dedicati, non disponibili al momento
- Velocità di step di partenza volutamente prudente (senza rampa di accelerazione) per il primo test, da aumentare più avanti con una rampa vera

**Criterio di completamento:** rotazione controllata e affidabile di entrambi i motori tramite comandi step/dir dall'ESP32

**Note (cronologia dello step):**
- [2026-09-20] Cablaggio completo del driver A4988 (logica + potenza) e del motore X, con verifica pin ENABLE tramite conteggio sull'etichettatura reale del modulo (foto)
- [2026-09-20] Bug: primo driver A4988 non muoveva il motore
  Sintomo: nessuna reazione del motore a "Giro completo" né alle frecce, solo un leggero ronzio; bloccato con forza al tatto (normale per uno stepper abilitato, non diagnostico da solo)
  Causa: sospetto danneggiamento del chip driver (probabile durante le prove di cablaggio in tensione) — tutti i controlli di continuità su STEP/DIR/ENABLE/VDD/RESET-SLEEP/VMOT/GND risultati corretti
  Fix applicato: sostituito con un secondo modulo A4988 di scorta — con il nuovo modulo il motore ha iniziato a muoversi (parzialmente), confermando che il primo era guasto
- [2026-09-20] Bug: Vref bloccato a 0,20V sul secondo modulo, il potenziometro sembrava non avere effetto
  Sintomo: valore instabile e comunque fermo a 0,20V (target 0,50–0,56V) girando il trimmer
  Causa: misura instabile a causa del motore collegato durante la prova (rumore/carico sul riferimento)
  Fix applicato: misurato Vref a motore scollegato — tarato correttamente a 0,55V
- [2026-09-20] Bug: il motore gira solo con passi singoli, non con pressione prolungata né con "Giro completo"
  Sintomo: ogni click singolo produce un piccolo movimento pulito senza perdere passi; in sequenza continua il motore non si muove (con solo un leggero ronzio) oppure avanza a scatti di circa 1/5 di giro senza completare la rotazione
  Causa: individuata con un test del "wiggle" — muovendo il filo che collega ENABLE (D17) durante il funzionamento, il motore si mette a girare; il problema persiste anche sostituendo il filo con uno nuovo o cambiando i fori della breadboard, e la continuità elettrica su quel filo risulta comunque corretta a riposo. Causa non ancora confermata con certezza: si sospetta un comportamento specifico dei pin GPIO16/GPIO17 (usati per DIR/ENABLE), che su alcune varianti di modulo ESP32 con PSRAM (WROVER) sono riservati e non utilizzabili come GPIO liberi — dalla foto del modulo (etichettato solo "ESP-32", nessuna scritta "WROVER" visibile) non è stato possibile confermare con certezza se questo sia il caso
  Fix applicato: non ancora confermato — spostati DIR (D16→D26) ed ENABLE (D17→D27) su pin sicuramente liberi da questo vincolo su qualunque variante ESP32; il test dopo lo spostamento non è ancora stato riportato a fine sessione — **da riprendere nella prossima sessione**

### Step 3 — Configurazione FluidNC

**Stato:** da fare

**Obiettivo:** file YAML FluidNC funzionante con assi X/Y sui due driver A4988 e pen-lift Z via uscita PWM spindle/laser (M3/M5), non come asse stepper vero

**Decisioni progettuali:** pen-lift implementato riutilizzando l'uscita PWM dello spindle/laser di FluidNC, non un asse Z fisico

**Criterio di completamento:** jog coordinato X/Y funzionante dalla WebUI di FluidNC, upload G-code via Wi-Fi verificato

**Note (cronologia dello step):** nessuna ancora

### Step 4 — Software "slicer" per generazione G-code

**Stato:** da fare

**Obiettivo:** possibilità di caricare un SVG/disegno e ottenere G-code pronto per la macchina, senza dover collegare via cavo

**Decisioni progettuali:** nessuna ancora — opzioni considerate: Inkscape con plugin G-code esistente, oppure sviluppo di una web app custom

**Criterio di completamento:** da definire

**Note (cronologia dello step):** nessuna ancora

## Storico sessioni

### [2026-09-20 09:45] Dashboard estesa (LED, OTA, log) e avvio debug driver A4988/motore X

**Riepilogo:** Ampliata la dashboard con più LED, aggiornamento firmware via Wi-Fi (OTA) e una tab di log da remoto; avviato lo Step 2 con il cablaggio completo del driver A4988 e del motore X, un primo driver risultato guasto e sostituito, Vref tarato correttamente, ma il motore gira in modo affidabile solo a passi singoli — bug non ancora risolto a fine sessione, in attesa di verifica dopo aver spostato DIR/ENABLE dai pin D16/D17 a D26/D27.

**Cosa è stato fatto:**
- Aggiunti alla dashboard 2 nuovi LED (verde D18, giallo D19) con pulsanti dedicati Rosso/Giallo/Verde; disabilitato il pulsante centrale del joystick (non più usato per il LED)
- Aggiunto supporto ArduinoOTA per aggiornare il firmware via Wi-Fi dopo il primo upload via USB (password uguale a quella dell'AP di emergenza)
- Aggiunta una tab "Log" nella dashboard che mostra via web gli stessi messaggi altrimenti visibili solo sul Serial Monitor — necessaria perché il Serial Monitor via USB non è utilizzabile quando si carica il firmware via OTA
- Collegato fisicamente il driver A4988 al motore X (fase logica: GND/VDD/RESET-SLEEP/STEP/DIR/ENABLE; fase di potenza: VMOT/GND da alimentatore 24V riciclato da striscia LED RGB, identificando con il multimetro che il vero 24V+ è il filo bianco e il GND è il filo rosso, non il contrario)
- Riadattata una scheda "Nano Terminal Adapter" (per Arduino Nano) a morsettiera generica per i collegamenti di potenza, sfruttando la sua natura puramente passiva
- Identificate le due bobine del motore (rosso+blu, nero+verde) via continuità e collegate al driver
- Collegati sulla dashboard i controlli motore: frecce sinistra/destra (passo singolo al click, rotazione continua non bloccante tenendo premuto) e due pulsanti "Giro completo" per un test rapido da 200 passi
- **Bug: primo driver A4988 non muoveva il motore** — Sintomo: nessuna reazione a "Giro completo", solo un leggero ronzio. Causa: sospetto danneggiamento del chip (tutti i controlli di continuità risultati corretti). Fix applicato: sostituito con un secondo modulo di scorta, che ha mostrato un primo movimento (parziale)
- **Bug: Vref bloccato a 0,20V, il trimmer sembrava non avere effetto** — Causa: misura instabile per il motore collegato durante la prova. Fix applicato: misurato a motore scollegato, tarato correttamente a 0,55V
- **Bug: il motore gira solo a passi singoli, non in sequenza continua** — Sintomo: click singoli puliti e senza perdita di passi, ma pressione prolungata o "Giro completo" non muovono il motore (o lo muovono a scatti di ~1/5 di giro senza completare la rotazione). Causa non confermata: un test del "wiggle" ha mostrato che muovere il filo ENABLE (D17) durante il funzionamento fa partire il motore, anche sostituendo il filo o cambiando i fori della breadboard, pur con continuità elettrica corretta a riposo — si sospetta un comportamento specifico dei pin GPIO16/17 su alcune varianti ESP32 con PSRAM (WROVER), non confermato con certezza dalla foto del modulo (etichettato solo "ESP-32"). Fix applicato: spostati DIR (D16→D26) ed ENABLE (D17→D27); esito del test non ancora riportato a fine sessione

**Decisioni prese:**
- Contesto: serviva un modo per vedere i log del firmware una volta passati all'aggiornamento via OTA, dato che il Serial Monitor USB non è più disponibile in quel caso
- Decisione: aggiunta una tab "Log" nella dashboard che replica via HTTP i messaggi altrimenti mandati solo su Serial
- Alternative scartate: nessuna, soluzione diretta senza alternative valutate
- Contesto: servivano morsetti a vite per i collegamenti di potenza (24V e motore) ma non erano disponibili morsetti dedicati
- Decisione: riadattata una scheda "Nano Terminal Adapter" (per Arduino Nano) come morsettiera generica, usando ogni colonna come nodo elettrico indipendente
- Alternative scartate: acquistare morsetti dedicati — scartata per procedere subito con quello che si aveva già disponibile
- Da rivedere se: si rendono disponibili morsetti a vite dedicati, più adatti per un assemblaggio permanente

**File consegnati/modificati:**
- `firmware/cnc2d_dashboard/cnc2d_dashboard.ino` (modificato: LED aggiuntivi, OTA, log, controlli motore, pin DIR/ENABLE spostati)
- `firmware/cnc2d_dashboard/README.md` (aggiornato di pari passo)

**Impatto su Vision/Pipeline:** Step 2 passato da "da fare" a "in corso"; aggiunte le decisioni progettuali su pin driver, mappatura bobine, alimentatore riciclato e uso della morsettiera adattata. Bug del motore non ancora risolto: da riprendere nella prossima sessione verificando l'esito dello spostamento pin DIR/ENABLE.

---

### [2026-09-19 09:25] Dashboard web ESP32 con controllo LED e configurazione Wi-Fi

**Riepilogo:** Creata e validata una dashboard web autonoma sull'ESP32 (frecce direzionali placeholder, pulsante centrale per il LED, tab Wi-Fi e tab Codice) con provisioning Wi-Fi da browser e fallback ad Access Point di emergenza; testata con successo sia via USB sia a batteria.

**Cosa è stato fatto:**
- Analizzata la fattibilità di una dashboard web ospitata direttamente sull'ESP32 e di un "tab codice" stile Arduino IDE nel browser; chiarito che l'ESP32 non può compilare sketch C++ da solo, quindi il tab Codice sarà usato in futuro per l'upload diretto di file G-code (non di sorgenti da compilare), da attivare dopo aver validato motori/servo (Step 2-3)
- Implementato lo sketch `firmware/cnc2d_dashboard/cnc2d_dashboard.ino`: web server nativo (`WebServer.h`) con pagina unica a 3 tab (Controllo, Wi-Fi, Codice), pulsante centrale che accende/spegne il LED su D2, frecce direzionali presenti ma non collegate
- Aggiunta gestione Wi-Fi via web: credenziali salvate in NVS (`Preferences`), form nella tab Wi-Fi per cambiarle senza ricompilare, fallback automatico ad Access Point `CNC-2D-Setup` se la connessione alla rete salvata fallisce, mDNS (`cnc2d.local`) per raggiungere la dashboard senza dipendere dall'IP assegnato dal router
- **Bug: errore di compilazione "unterminated raw string"**
  **Sintomo:** Arduino IDE segnalava stringa raw non terminata durante la compilazione dello sketch
  **Causa:** l'utente aveva copiato il codice a mano in un nuovo sketch anziché aprire il file `.ino` originale, con probabile perdita dell'ultima riga di chiusura della stringa raw durante il copia-incolla
  **Fix applicato:** scaricato il file raw direttamente da GitHub e incollato per intero — verificato e compilazione riuscita
- Testata l'intera dashboard: provisioning Wi-Fi di casa dalla tab dedicata, raggiungibilità su `http://cnc2d.local`, controllo LED su D2 funzionante sia con ESP32 alimentato da USB sia da batteria 18650+interruttore

**File consegnati/modificati:**
- `firmware/cnc2d_dashboard/cnc2d_dashboard.ino` (nuovo)
- `firmware/cnc2d_dashboard/README.md` (nuovo)

**Impatto su Vision/Pipeline:** Aggiunto il nuovo sotto-step "Step 1.1 — Dashboard web di controllo e test" (completato) all'interno dello Step 1, non previsto esplicitamente nella pipeline originale ma propedeutico al controllo assi (Step 2) e al futuro upload G-code (Step 4).

---

### [2026-09-19 10:03] Bring-up hardware ESP32: alimentazione USB e batteria completati

**Riepilogo:** Verificata la compatibilità dei componenti scelti (batteria, motori, driver, alimentatore), completato il bring-up dell'ESP32 con test LED su D2 sia via USB che via batteria 18650+interruttore; prossimo passo il codice per driver A4988/motori.

**Cosa è stato fatto:**
- Analizzata e corretta la conversazione tecnica avuta con Gemini sulla compatibilità batteria NASTIMA / motori NEMA17 17HS4023 / driver A4988 / alimentatore 24V riciclato da striscia LED (corretta imprecisione sulla corrente nominale motore: 0.7A/fase, non 1.0A; segnalata mancanza di un regolatore dedicato per la logica ESP32; chiarito l'uso del PWM spindle/laser per il pen-lift invece di un asse Z vero)
- Verificato il modulo di carica/boost per 18650 (Type-C, boost 5V, supporto "discharge while charging")
- Pianificata la sequenza di bring-up: GND comune → ESP32 standalone → driver a vuoto → un motore → due motori → config FluidNC
- Montato l'ESP32 su breadboard e identificate le etichette reali sulla scheda fisica (prefisso D, es. D2, VIN, GND — diverse dal pinout generico GPIO/ADC del listing Amazon)
- Test funzionale con sketch Blink su D2 + LED + resistenza 220 ohm via Arduino IDE — riuscito
- Aggiunto interruttore sulla linea 5V+ (tra modulo batteria 18650 e VIN dell'ESP32, non sulla batteria) e verificato il funzionamento su alimentazione a batteria
- Preparato uno sketch base con ArduinoOTA per l'upload firmware via Wi-Fi (il primo upload resta comunque necessario via USB)

**Decisioni prese:**
- Contesto: bisognava scegliere come alimentare la logica ESP32 separatamente dai 24V dei motori
- Decisione: batteria 18650 con modulo boost/carica 5V Type-C, interruttore solo sulla linea 5V+ (non sulla batteria), per permettere la ricarica anche a macchina spenta
- Alternative scartate: alimentare l'ESP32 direttamente dai 24V tramite un regolatore dedicato — scartata per ora in favore della soluzione a batteria separata, più semplice da testare in questa fase
- Da rivedere se: emergono problemi di autonomia o di stabilità della tensione 5V sotto carico Wi-Fi

**File consegnati/modificati:** nessun file di codice consegnato in questa sessione (solo sketch inline in chat: Blink su D2, sketch base ArduinoOTA)

**Impatto su Vision/Pipeline:** Prima creazione del documento — Vision e Pipeline stabilite sulla base delle decisioni già prese nelle sessioni precedenti (non tracciate qui perché antecedenti alla creazione di questo documento).

---
