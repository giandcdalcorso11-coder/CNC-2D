# Documento di Sessione — CNC 2D Plotter

**Versione:** 16
**Ultimo aggiornamento:** 2026-10-01 18:00

## Regole di co-editing

Questo documento è scritto da **due sessioni Claude Code in parallelo sullo stesso branch**: una sul fronte **meccanica/firmware**, una sul fronte **contenuti e serie social**. Per non perdere voci a vicenda:

1. **`git fetch` e rebase prima di scrivere, push subito dopo.** Non lasciare modifiche locali non pushate: la finestra di collisione è tutta lì.
2. **Marca la voce di storico con il fronte di lavoro**, es. `### [2026-09-26 17:48] (meccanica) …`. Due sessioni diverse sono due voci diverse: la regola "una voce per sessione" non impone di fonderle.
3. **Le note dei singoli Step si appendono in fondo al proprio Step**, quindi non collidono mai finché le due sessioni lavorano su Step diversi.
4. **Se la Versione va in conflitto**, chi risolve mette `max(A, B) + 1` e tiene **entrambe** le voci, ordinate per data decrescente. Mai scartarne una.
5. Gli unici punti di contesa reali sono **le due righe di testata** e **la prima riga dopo `## Storico sessioni`**. Tutto il resto è partizionato per costruzione.

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
- [2026-09-26] **L'alimentatore non è più il 24 V recuperato dalla striscia LED**, ma un DVE DSA-36W-12: **12 V, 3 A, 36 W**. Supera il dato del 2026-09-19. Nota importante sulle conseguenze: a corrente impostata uguale, **la tensione di alimentazione non cambia né la coppia né la temperatura dei motori**, perché l'A4988 è un driver a corrente controllata — la tensione decide solo quanto in fretta la corrente sale, quindi conta alle alte velocità. Bilancio a 12 V con tre motori a 0,6 A: 0,28 A ciascuno, 0,85 A in totale, ~14 W su 36 disponibili (28% della corrente). L'unico punto in cui i 12 V stringono è il collegamento di due motori in serie su un solo driver: 6,0 V dei 12 se ne vanno fra caduta resistiva, induttanza e forza controelettromotrice, contro i 3,0 V di un motore per driver

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
- [2026-09-20] Rifattorizzato il motore della dashboard: il movimento non è più eseguito dentro l'handler HTTP (che bloccava per 3 secondi affamando `server.handleClient()` e l'OTA) ma da una coda di passi servita in `loop()`, prima e dopo `handleClient()`; DIR impostato una sola volta a inizio movimento con tempo di assestamento invece che a ogni impulso; aggiunta rampa di accelerazione opzionale sui primi 40 passi
- [2026-09-20] Aggiunta tab "Motore": intervallo tra i passi regolabile da browser (300 µs – 200 ms), numero di passi, rampa on/off, abilitazione diretta del driver e sezione "Test pin" che forza STEP/DIR/EN a un livello fisso per poterli misurare col multimetro. Ogni movimento registra nel log i passi effettivamente emessi, così si distingue subito un problema firmware da uno elettrico

### Step 2 — Bring-up driver A4988 e motori (X/Y)

**Stato:** completato

**Obiettivo:** un motore NEMA17 pilotato correttamente da un driver A4988 tramite ESP32, poi due motori in parallelo

**Decisioni progettuali:**
- Vref target ~0.5-0.56V per 0.7A per fase (corrente nominale reale del 17HS4023, non 1.0A come inizialmente indicato da una fonte esterna consultata)
- Alimentazione di potenza: alimentatore switching DC **12V/3A** (DVE DSA-36W-12). Sostituisce l'alimentatore riciclato dalla striscia LED RGB, che si è rivelato inutilizzabile (vedi bug sotto). 12V sono ampiamente sufficienti per il plotter e fanno scaldare molto meno il driver rispetto a 24V
- **Condensatore elettrolitico 100 µF / 50 V obbligatorio tra VMOT e GND del driver**, il più vicino possibile al modulo: senza, i picchi induttivi generati dalle bobine a ogni commutazione distruggono l'A4988 (è la causa della perdita dei primi due moduli)
- Collaudo di accettazione di ogni nuovo modulo A4988 prima di alimentarlo: continuità tra STEP, DIR, EN e GND deve essere **muta** su tutti e tre. Venti secondi che smascherano subito un ingresso in corto
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
- [2026-09-20] Il bug sopra è stato **risolto**: lo spostamento su D26/D27 non c'entrava nulla e l'ipotesi PSRAM/WROVER era sbagliata. La causa reale era l'alimentatore di potenza (vedi bug successivo). I pin sono stati comunque lasciati su D26/D27, che vanno benissimo
- [2026-09-20] Bug: alimentatore di potenza inutilizzabile — **causa radice di tutta la sessione**
  Sintomo: tensione su VMOT del driver oscillante tra 1 V e 20 V invece che ferma; coppia di tenuta debolissima (l'albero si girava con due dita, mentre a 0,69 A/fase dovrebbe essere immobile); passi singoli occasionalmente corretti, rotazione continua mai
  Causa: l'alimentatore riciclato è un **controller per strisce LED RGB**, non un alimentatore DC. Dei 4 fili, il bianco è il +24V comune e rosso/verde/blu sono le tre uscite di canale, cioè MOSFET pilotati in PWM. Usare il filo rosso come massa significava alimentare il driver attraverso un interruttore che si apriva e chiudeva continuamente. Verificato misurando sotto carico su tutti e tre i canali: instabili tutti
  Fix applicato: sostituito con un alimentatore switching DC 12V/3A, misurato stabile a 12,2 V. **Confermato**
- [2026-09-20] Bug: secondo driver A4988 con ingresso STEP in corto verso massa
  Sintomo: dopo aver risolto l'alimentazione, l'albero era finalmente duro e il test ENABLE rispondeva, ma nessun comando produceva movimento e il motore restava in silenzio assoluto. Misurando il pin STEP sul driver si leggeva sempre 0 V pur avendo continuità verso D4
  Causa: individuata dividendo il nodo STEP in due metà e misurandole separatamente — lato ESP32 muto (GPIO4 sano), lato driver in corto a 0,2 Ω, e sfilando il modulo la breadboard tornava muta. L'ingresso STEP era quindi in corto **dentro il chip**, inchiodato a 0 V. Danno provocato dallo stesso alimentatore RGB
  Fix applicato: sostituito con il terzo modulo A4988, collaudato prima dell'installazione con il test di continuità STEP/DIR/EN verso GND. **Confermato**
- [2026-09-20] Motore X funzionante: Vref tarato a 0,54 V sul nuovo modulo, test ENABLE corretto (albero bloccato con spunta, libero senza) e movimento comandato dalla dashboard riuscito
- [2026-09-21] Sweep di velocità completato su tutta la gamma disponibile fino a 500 passi/s: nessuna banda di risonanza, nessun microstepping necessario per la stabilità
- [2026-09-21] Secondo asse cablato: driver Y con STEP su D25 e DIR su D33, ENABLE condiviso con X su D27. Al primo avvio il motore Y girava da solo senza controllo — causa: D25/D33 non erano ancora configurati come uscite nel firmware e, restando ingressi flottanti, raccoglievano rumore che il driver interpretava come impulsi di STEP. Risolto portando i pin STEP a livello basso come prima istruzione di `setup()`
- [2026-09-21] **Step chiuso:** entrambi gli assi pilotati correttamente, singolarmente e in contemporanea

### Step 3 — Configurazione FluidNC

**Stato:** completato

**Obiettivo:** FluidNC installato e configurato come firmware definitivo della macchina, con movimento coordinato X/Y e pen-lift sul servo

**Decisioni progettuali:**
- **FluidNC è il firmware definitivo**, lo sketch `cnc2d_dashboard` retrocede a strumento di collaudo hardware. Motivo: interpolazione coordinata, pianificazione dell'accelerazione fra segmenti e interpretazione del G-code sono settimane di lavoro già risolte e collaudate su migliaia di macchine
- **Pen-lift come asse Z vero** tramite il tipo motore `rc_servo`, non tramite l'uscita PWM dello spindle con M3/M5. Supera la decisione originale dello Step 3: trattare la penna come un asse permette al generatore di G-code di gestirla come un movimento qualsiasi
- **Microstepping 1/16** (MS1/MS2/MS3 a VDD su entrambi i driver): a passo intero la risoluzione sarebbe di circa 0,2 mm per passo, uno scalino visibile a occhio sul tratto. A 1/16 si scende attorno ai 0,02 mm. Il valore esatto di `steps_per_mm` dipende dal pignone scelto (vedi Step 5) e va comunque tarato sulla macchina
- **Nessun azzeramento automatico**: niente finecorsa di riferimento, l'origine si imposta a mano con `G92 X0 Y0`. Conseguenza: `soft_limits` resta disattivo, perché FluidNC lo consente solo su macchina azzerata, e la protezione contro le uscite dall'area passa all'interfaccia web
- **Driver mai disabilitati** (`idle_ms: 255`): con l'azzeramento manuale, un asse che si ammorbidisce e viene spostato a mano farebbe perdere la posizione senza che nessuno se ne accorga. Annulla l'idea, discussa e poi scartata, di un auto-spegnimento dei driver dopo inattività
- **Due pulsanti di emergenza** su D13/D14 nell'angolo in basso a sinistra, configurati come finecorsa rigidi: in teoria non vengono premuti mai, se succede FluidNC ferma tutto e va in allarme. Cablati normalmente aperti verso massa — accettabile per un backstop, da rivedere con microswitch veri
- **LED di stato come `user_outputs`**, comandati dall'interfaccia web con M62/M63: verde = tutto bene, giallo = attenzione (non calibrato, posizione non attendibile, in pausa), rosso = errore

**Criterio di completamento:** la macchina disegna un quadrato e un cerchio con movimento coordinato dei due assi, verificabile a vista sui motori anche senza struttura montata

**Note (cronologia dello step):**
- [2026-09-21] Scritta la configurazione di partenza `firmware/fluidnc/cnc2d-config.yaml` con la mappa dei pin già validata. Restano da tarare `steps_per_mm`, `max_travel_mm` e gli estremi dell'impulso del servo, tutti dipendenti dalla meccanica
- [2026-09-21] Installazione FluidNC avviata dal web installer (`installer.fluidnc.com`, richiede Chrome o Edge perché usa WebSerial). Scelte: versione **v4.1.0** (ultima non pre-release), processore **esp32** (DevKit WROOM-32 a 30 pin), variante **wifi** (l'unica con WebUI), tipo **fresh-install**, interfaccia **WebUI-2** — scelta fra le tre disponibili perché è quella su cui è costruita la documentazione del wiki, e se ne può installare una sola
- [2026-09-21] Da verificare alla prima accensione: la configurazione è stata scritta sullo schema FluidNC 3.x, mentre la versione installata è la 4.1.0. Eventuali nomi di campo cambiati vengono segnalati dal validatore all'avvio
- [2026-09-21] **Lo schema 4.1.0 ha accettato la configurazione scritta sul 3.x senza una sola modifica.** Il dubbio segnalato nella nota precedente è risolto: nessun nome di campo è cambiato fra le due versioni per i costrutti che usiamo
- [2026-09-21] Attivazione della configurazione: il pulsante "Seleziona configurazione" del browser dei file non ha avuto effetto, si è dovuto usare `$Config/Filename=cnc2d-config.yaml` dal terminale. Il comando di modifica a runtime `$/axes/shared_stepper_disable_pin=...` non restituisce nulla sulla 4.1.0, quindi le correzioni alla configurazione si applicano ricaricando il file e riavviando con `$Bye`
- [2026-09-21] WiFi configurato in modalità STA, hostname `cnc2d`, raggiungibile su `http://cnc2d.local` e `http://192.168.1.11`. Da qui in avanti l'installer non serve più: si lavora dalla WebUI-2
- [2026-09-21] LED verificati funzionanti con `M62 P0/P1/P2` e `M63 P0/P1/P2` (P0 verde gpio.18, P1 giallo gpio.19, P2 rosso gpio.2). La variante immediata M64/M65 non è stata necessaria. Il canale per legare le spie allo stato della macchina è quindi disponibile
- [2026-09-21] Microstepping 1/16 verificato su entrambi gli assi: 40 mm comandati (3200 microscatti con `steps_per_mm: 80`) producono un giro esatto dell'albero. A passo intero ne avrebbero prodotti sedici, quindi il test è inequivocabile a occhio. Ponticelli realizzati a catena RST→MS3→MS2→MS1, sfruttando il fatto che RST è già a VDD ed è adiacente a MS3: tre ponticelli cortissimi per driver, nessun filo lungo
- [2026-09-21] **Step completato**: quadrato da 40 mm e cerchio da raggio 20 mm eseguiti correttamente, con ritorno alle coordinate di partenza. Il criterio di completamento è soddisfatto

### Step 4 — Interfaccia web e generazione G-code

**Stato:** in corso

**Obiettivo:** un'unica pagina web da cui caricare un disegno, vederne l'anteprima sul foglio, generare il G-code e mandarlo in esecuzione

**Decisioni progettuali:**
- **Tutta l'elaborazione nel browser, l'ESP32 solo esegue.** L'ESP32 ha circa 300 KB di RAM utilizzabile: vettorializzare un'immagine e generare G-code è fuori portata. Il browser produce il file, l'ESP32 lo conserva ed esegue — così il disegno prosegue anche se si chiude il portatile o cade il Wi-Fi
- **Vettorializzazione con algoritmi deterministici** (Potrace o equivalente in JavaScript), non con un modello generativo. Alternativa scartata: passare l'immagine a un AI esterna con prompt da copiare e incollare — un modello linguistico non ricalca un'immagine, inventa percorsi plausibili, diversi a ogni tentativo e senza controllo su dove passa la penna. L'AI resta utile a monte, per preparare il soggetto come disegno a tratto
- **Si parte dall'SVG e basta.** I percorsi sono già vettoriali, non serve tracciamento: è il caso d'uso più frequente e costa una frazione del lavoro. Line art e planimetrie (soglia + tracciamento) vengono dopo. Le foto richiedono algoritmi dedicati — stippling, campi di flusso, retinatura — e sono un progetto a sé
- **Interfaccia servita dall'ESP32 come file separati**, non più come stringa dentro lo sketch: il codice resta leggibile e l'interfaccia si aggiorna dal browser senza ricompilare
- **Layout**: colonna menu a sinistra, foglio centrale con formati A4/A5/A6 e dimensioni modificabili, drop box con anteprima, colonna a destra per scorrere l'anteprima del movimento — che durante il disegno vero diventa indicatore di avanzamento. Tab separate per log e impostazioni
- L'anteprima distingue **tratti disegnati e spostamenti a vuoto** con colori diversi: vedere gli spostamenti a vuoto è il modo per accorgersi che il disegno è ordinato male
- **L'interfaccia dovrà essere servita dall'ESP32 per comandare la macchina.** FluidNC espone il suo HTTP senza intestazioni CORS, e una pagina in HTTPS non può comunque interrogare un dispositivo in HTTP semplice: online-ovunque più comando diretto è una combinazione che il browser vieta. Conseguenza operativa: niente framework, niente librerie, niente processo di build, perché la pagina deve stare nella memoria dell'ESP32
- **Il calcolo resta nel browser anche quando la pagina è servita dall'ESP32.** Il microcontrollore consegna il file, il processore del PC o del telefono fa il lavoro. L'ottimizzazione del percorso del logo di prova ha impiegato 1 ms
- **Il criterio dell'ottimizzazione è il tempo, non la distanza.** Un'alzata di penna costa circa mezzo secondo fra movimento del servo e assestamento; cinquanta millimetri di spostamento a 2000 mm/min ne costano uno e mezzo. Saldare i tratti contigui vale quindi più che accorciare gli spostamenti, e viene prima nella catena
- **Modalità finta sempre disponibile**: l'interfaccia simula le risposte della macchina, così ci si lavora senza accendere nulla — che è la maggior parte del tempo

**Criterio di completamento:** caricare un SVG, vederlo posizionato nel foglio, generare il G-code e farlo eseguire alla macchina

**Note (cronologia dello step):**
- [2026-09-24] Costruita l'impalcatura completa (`webui/`, dodici file): tre colonne, tab Disegno/Log/Impostazioni, foglio con formati, cursore verticale per scorrere l'anteprima del percorso
- [2026-09-24] Rifatta l'area centrale: il riquadro è ora **il piano della macchina**, fisso, con reticolo da 10 mm e lo zero macchina marcato; il foglio è un rettangolo disegnato dentro, trascinabile col puntatore, e il suo bordo diventa rosso quando esce dall'area. Il disegno è tenuto in coordinate del foglio, quindi lo segue per costruzione
- [2026-09-24] Importazione SVG funzionante: l'appiattimento delle curve lo fa il browser campionando `getPointAtLength` per lunghezza d'arco, e lo stesso campionamento riconosce i sotto-tracciati (due campioni non possono distare più del passo, quindi un salto più lungo è uno stacco di penna dentro lo stesso elemento — il foro di una "o")
- [2026-09-24] Ottimizzazione e G-code funzionanti. Sul logo dell'utente: 11 tratti, 294 punti, spostamenti da 769 a 424 mm, circa 1 min 15 s stimati, calcolo in 1 ms. La stima del tempo usa un profilo trapezoidale per polilinea, perché il look-ahead di FluidNC mantiene la velocità fra segmenti consecutivi
- [2026-09-24] Resta da fare: invio del programma alla macchina, avanzamento in tempo reale, caricamento della pagina sull'ESP32, conversione da fotografia a tracciato

### Step 5 — Struttura meccanica

**Stato:** in corso (progettazione)

**Obiettivo:** struttura a portale che porta la penna su tutta l'area di un foglio A4 in orizzontale, con i due assi motorizzati e il pen-lift solidale al carrello Y

**Decisioni progettuali:**
- **Trasmissione a cremagliera e pignone**, non a cinghia. Motivo: si stampa in 3D a costo quasi nullo, mentre le cinghie richiedono cinghia, pulegge e tenditori acquistati. Alternativa scartata: cinghia GT2, che avrebbe gioco quasi nullo e motori fissi — resta la via di ripiego se il gioco della cremagliera risultasse ingestibile, e non richiederebbe di rifare la struttura ma solo i supporti dei motori
- **Motore solidale al carrello**, conseguenza intrinseca della cremagliera: per tenere il motore fermo dovrebbe muoversi la cremagliera, raddoppiando lo spazio necessario. Comporta ~300 g di massa mobile in più sull'asse X, da compensare con velocità moderate
- **Gioco gestito assottigliando il dente della cremagliera di 0,2 mm, non con un precarico a molla.** Supera la decisione del precarico a molla: il gioco effettivo si regola con la profondità di ingranamento (per una cremagliera `j = 2·Δc·tan20°`, quindi 0,2747 mm di avvicinamento azzerano lo 0,2), e il metodo pratico è la striscia di carta da 0,10 mm nel punto più stretto. Una molla avrebbe aggiunto un grado di libertà su un carrello che ne ha già abbastanza. Impostazione precedente: precarico a molla del pignone contro la cremagliera: il motore non è fissato rigido ma su una piastrina che oscilla attorno a un perno, tirata verso i denti da una molla. È la contromisura al gioco fra denti stampati, che su un plotter — che inverte direzione a ogni segmento — si tradurrebbe in contorni che non chiudono e tratti sdoppiati
- **Due vincoli a livelli diversi**: il carrello è catturato dalla guida e non può sollevarsi, il motore resta libero di flottare sul carrello. I due vincoli agiscono su corpi diversi e non si annullano a vicenda
- **Cuscinetti 608ZZ (8 × 22 × 7) per la pinza sui labbri, MR63 (3 × 6 × 2,5) per la guida laterale.** Supera la scelta dei 623ZZ del 2026-09-25: i 608 erano già in casa, e il profilo reale acquistato ha labbri sporgenti che si prestano a essere pinzati da una ruota sopra e una sotto. Otto 608 (due per stazione, due stazioni per fianco) più quattro MR63 dentro il canale. La pressione di contatto di un 608 a 3 N è 27 MPa contro i 150 a cui cede l'alluminio: margine 5×. Impostazione precedente, mantenuta come principio: cuscinetti 623ZZ (3 × 10 × 4) che rotolano su guide metalliche, non cuscinetti lineari su barre tonde. Il diametro è scelto sulla pressione di contatto: a 3 N un Ø10 preme sull'alluminio con 53 MPa contro i 27 di un 608 e gli 86 di un MR62, e l'alluminio cede attorno ai 150. Il foro da 3 mm è M3, **la stessa vite di tutto il resto della macchina** — motore, cremagliera, regolazioni. Le superfici di rotolamento devono essere metallo: l'anello esterno è acciaio temprato e in poche ore scava un solco nella plastica stampata
- **Ruote impilate a coppie, non sfalsate**, possibile perché due Ø10 stanno in 20 mm. La ruota di sopra e quella di sotto pinzano lo stesso punto del canale, quindi la regolazione è diretta invece che a distanza
- **Regolazione con asole M3, non con eccentrici.** Un eccentrico da mezzo millimetro di offset copre un millimetro di corsa; il gioco da recuperare può essere di due o tre. L'asola dà tutta la corsa che serve e usa M3 invece di M8
- **Perni stampati Ø3 per le ruote fisse, viti M3 con tratto liscio per quelle regolabili.** Il conto sul perno stampato dà 5,7 MPa contro i 50 del PLA. Il cuscinetto non deve mai girare sulle creste di un filetto: appoggia su tre punti e balla
- **Un profilo unico a U al posto di due staffe separate.** Le due pareti di un estruso sono parallele per costruzione, mentre due staffe avvitate a mano divergono di qualche decimo lungo la corsa. Questo toglie alla radice il problema dell'iper-vincolo, e sull'asse X rende superflua la regola della guida maestra qui sotto — che resta però valida sull'asse Y, dove le due guide sono per forza pezzi distinti a distanza di 385 mm
- **Una guida comanda, l'altra sostiene** (vale sull'asse Y, dove le guide restano due pezzi separati). La maestra vincola quota verticale, beccheggio, posizione laterale e imbardata; la secondaria vincola solo il rollio e resta libera lateralmente. Due guide montate a mano sul legno non sono mai parallele entro un decimo, e se entrambe vincolassero il laterale quell'errore diventerebbe attrito variabile lungo la corsa — il tipo di errore che non si riesce nemmeno a compensare
- **Un eccentrico per carrello**, sul cuscinetto laterale di contrasto: stampando non si azzecca mai il gioco al primo colpo. Il gioco verticale si regola invece con le asole dei profili superiori, che agiscono su tutta la lunghezza in una volta sola
- **Motore, pignone con cremagliera e cuscinetti occupano bande laterali distinte.** Il corpo del motore è 42 mm e scende quindi 21 mm sotto l'asse, mentre la cremagliera arriva a 15 mm sotto: si sovrappongono in altezza e devono per forza stare affiancati. Circa 110 mm di larghezza complessiva del carrello. Conseguenza: la flangia di fissaggio della cremagliera va su un lato solo, quello opposto al motore
- **Formato A4 in orizzontale**, corsa X ~230 mm (cremagliera da 59 denti, corsa utile ~250) e corsa Y ~317 mm. Supera il formato A5 del 2026-09-21: l'A4 in orizzontale copre l'A4 senza bisogno della corsa X da 317, perché il disegno si ruota di 90° nel software. Il vincolo che teneva la macchina piccola era la massa mobile, e si è rivelato inesistente (tetto 3410 g contro ~1150 stimati). Impostazione precedente: formato massimo A5, corse utili 200 mm su X e 300 mm su Y; un portale corto riduce anche la tendenza del ponte a mettersi di traverso, essendo spinto da un lato solo
- **Ponte in alluminio rigido**: un ponte che non flette non può sbandare
- **Base in compensato da 10 mm, 385 × 455 mm** (acquistata). Rigida e pesante: la massa assorbe le vibrazioni invece di trasmetterle alla penna. Supera l'indicazione precedente dell'MDF da 15-18 mm, scelta prima di sapere cosa si trovasse
- **Ogni millimetro di lunghezza del carrello è un millimetro di corsa in meno**, mentre la larghezza trasversale non costa corsa. Le due dimensioni vanno quindi trattate in modo diverso: stringere in larghezza quanto si vuole, non accorciare lungo la corsa per guadagnare foglio che non serve
- **Pignone modulo 1,5 con 20 denti, tutto stampato in PLA+.** Diametro primitivo 30 mm, 33,95 passi/mm, risoluzione 0,029 mm. La risoluzione non è il fattore limitante — l'errore reale della macchina sarà attorno ai 0,2 mm, dieci volte tanto — quindi quel margine conviene spenderlo in robustezza del dente
- **La compensazione dimensionale sta nello slicer, non nei modelli** (`X-Y contour compensation` a circa −0,10, `hole compensation` a 0). È una proprietà della stampante, non del pezzo: nel modello andrebbe replicata su pignone e cremagliera e si sommerebbe a quella dello slicer
- **Foro sagomato sull'albero, senza grano, da verificare sul campo.** Il calcolo dà 0,6 MPa contro i circa 50 a cui cede il PLA, quindi la coppia non è un problema; i rischi veri sono lo sfilamento assiale e l'arrotondamento dello spigolo del piatto dopo migliaia di inversioni. Verifica prevista: una riga di pennarello che attraversa pignone e albero, controllata dopo un'ora di movimento
- **Cavi mobili da progettare subito**: sette fili fra motore X e servo seguono il carrello per migliaia di cicli, servono catena portacavi o ansa flessibile
- **Motore dell'asse Y sotto la trave, a cavallo dell'anima.** La gabbia del motore è centrata sul piano dell'anima (interno Y da −6,750 a +17,250), quindi il corpo può stare solo sotto il bordo inferiore dell'anima: la quota dell'asse non è un parametro libero, è una conseguenza. Alternative verificate e scartate: **motore sopra il carrello** (costa 44 mm di franco verticale alla macchina, perché il punto più basso passerebbe da −7 a −51 — ma il modulo penna verrebbe più corto, quindi il conto vero è quasi in pari e ha deciso la comodità di attacco del modulo punta); **motore tutto fuori sul lato −Y** con la cremagliera direttamente sotto l'ala, che geometricamente funziona ma porta il baricentro a Y ≈ −37, fuori dalla carreggiata, e l'ala finisce a −42,05 quindi non c'è dove allargare l'appoggio. **Conseguenza accettata: la cremagliera non può stare sull'ala della trave, deve scendere su una base alta ~44 mm**
- **Anti-sollevamento sul bordo inferiore dell'anima, non sotto l'ala.** Due MR63 ad asse Y spingono verso l'alto contro il bordo inferiore dell'anima. Alternativa scartata: una seconda coppia di 608 sotto l'ala, che avrebbe leva migliore (Y −19,45 invece di Y 0) ma cadrebbe esattamente dove scende la cremagliera. Il carico lo giustifica: la forza tangenziale agisce sul punto primitivo a Z −20,881 mentre l'attrito che la contrasta agisce sui 608 a Z 23,500, cioè **44,381 mm di braccio** scaricati sui 60 mm di passo perni — a stallo sono ±5,4 N contro 2,15 N di peso per stazione, e una stazione si stacca

**Criterio di completamento:** gioco misurato sotto 0,1 mm e `steps_per_mm` tarato sulla macchina reale

**Note (cronologia dello step):**
- [2026-09-21] Definito il metodo di taratura in loco, da usare a struttura montata: muovere quasi tutta la corsa (non 100 mm: l'errore di misura è sempre mezzo millimetro, quindi più lunga è la corsa più la taratura è precisa), misurare lo **spostamento del carrello** con un calibro e non la lunghezza della linea disegnata, e partire sempre nello stesso verso per non includere il gioco nella misura. Formula: `nuovi passi/mm = vecchi × comandata / misurata`
- [2026-09-21] Definita la misura del gioco: comandare +200 mm, poi −200 mm, e misurare di quanto il carrello non è tornato al punto di partenza. Sotto 0,1 mm il precarico funziona, sopra 0,3 mm va ristudiato prima di procedere
- [2026-09-24] **Primo pignone stampato e provato.** Foro sagomato disegnato con 0,2 mm di gioco: "entra a fatica ma entra", che è il risultato voluto. Stampata anche la versione a doppia elica: la dentatura è venuta pulita, quindi la stampante regge il modulo 1,5
- [2026-09-24] **Bilancio dell'errore atteso** con questa costruzione: ripetibilità ±0,1÷0,2 mm, precisione assoluta ±0,3÷0,5 mm dopo la taratura. Sotto i due decimi tutto sparisce dentro la larghezza del tratto di un pennarello. L'errore più insidioso non è il gioco ma **l'ortogonalità fra i due assi**: mezzo grado fa 1,3 mm di sbieco sull'angolo di un A5, più di tutti gli altri messi insieme. Si misura confrontando le diagonali di un quadrato disegnato, e il residuo si compensa via software
- [2026-09-24] Segmenti delle cremagliere fissati: asse X un pezzo da 53 denti (249,76 mm) in diagonale sul piatto, asse Y due pezzi da 37 denti (174,36 mm) dritti. Ogni segmento è un multiplo esatto del passo con il primo dente a mezzo passo dall'estremità, così il passo si mantiene attraverso il giunto
- [2026-09-24] Fissaggio della cremagliera: **un solo foro tondo al centro dell'intera cremagliera, asole dappertutto altrove**, una ogni 50 mm alternate sui due lati. Il PLA si dilata tre volte più del legno e su 350 mm cresce di oltre tre decimi: bloccato rigidamente in più punti si inarcherebbe, cambiando l'interasse col pignone nel mezzo della corsa
- [2026-09-24] Geometria del carrello ancora aperta: l'utente ha proposto due staffe a T affacciate avvitate al legno, con il carrello stampato in mezzo. Da chiarire se i cuscinetti appoggiano sull'ala orizzontale o sul gambo verticale. Problema già individuato: le staffe avvitate solo al piede sono mensole che flettono sotto il precarico, che inverte verso a ogni cambio di direzione — rimedio previsto, viti ogni 40-50 mm più un dorso stampato dietro la gamba verticale
- [2026-09-25] **Materiali acquistati**, e il progetto si adatta a quello che si trova in ferramenta invece del contrario: base in compensato 385 × 455 × 10; viti M3 da 10, 16 e 25 mm **con tratto liscio sotto la testa** (indispensabile per i perni dei cuscinetti); angolare forato 23,5 × 23,5 × 1,5; profilo a U 67,5 × 23,5 × 1,5
- [2026-09-25] **Il profilo a U più due angolari formano il canale chiuso già progettato**, con misure che coincidono: interno 64,5 × 22 mm, e due ruote Ø10 impilate ne occupano 20, lasciando 2 mm da recuperare con le asole. Gli angolari bullonati sopra le ali, flangia verso l'interno, lasciano una fessura centrale di 20,5 mm per il collo del carrello. Quattro superfici di rotolamento in due pezzi comprati
- [2026-09-25] **Verifica delle corse sulla base reale**: con un carrello da 90 mm restano circa 325 mm di corsa in Y e 230 in X, contro i 260 × 200 che servono per un A5 con margine. Circa 65 mm di margine in Y — abbastanza da permettersi un carrello fino a 150 mm senza perdere foglio
- [2026-09-25] Geometria del carrello X definita: il carrello abbraccia **una guida centrale sola**, che va bene perché il profilo è largo 67,5 mm. Sull'asse Y non funzionerà: il ponte è largo 385 mm e su una guida sola si metterebbe di traverso
- [2026-09-25] Da verificare con una calamita: i profili sono acciaio o alluminio. Un metro di quel profilo pesa 1,35 kg in acciaio contro 465 g in alluminio, e sul ponte mobile dell'asse X quel peso lo muove il motore a ogni riga
- [2026-09-25] **La guida va rialzata dal pannello**, scoperto disegnando il CAD: le ruote inferiori sporgono sotto il profilo, quindi con la guida appoggiata direttamente al legno striscerebbero sul pannello. Serve un distanziale continuo sotto tutta la guida — non blocchetti isolati, perché il profilo da 1,5 mm flette fra un appoggio e l'altro sotto il precarico
- [2026-09-25] **Conseguenza da non dimenticare: il rialzo della guida alza tutto.** Il carrello sale, quindi sale l'asse del motore, quindi la cremagliera deve salire dello stesso identico valore perché il pignone continui a ingranare. La relazione resta `fondo cremagliera = asse motore − 26,875`: se si rialza la guida di 5 mm e ci si dimentica della cremagliera, il pignone non tocca più i denti
- [2026-09-25] Il rialzo va misurato, non stimato per eccesso: ogni millimetro in più allontana la penna dal piano delle ruote e amplifica l'errore di beccheggio e rollio di circa l'1% al millimetro. Regola: sporgenza reale delle ruote più 2 mm di franco
- [2026-09-26] **Sezione reale del binario ricavata dallo STEP**, e non è un U come si pensava: è un profilo a cappello, 66,4 largo × 23,2 alto, scatola centrale 23,7, canale interno 20,0 × 21,9, fondo e labbri 1,3 mm, pareti 1,85. I labbri sporgono in fuori da X 11,85 a 33,2: sono loro la superficie di rotolamento, pinzata fra un 608 sopra e uno sotto
- [2026-09-26] **Architettura del carrello: U rovesciata che cavalca il binario.** Due fianchi verticali da 3 mm fuori dal binario (X ±35 ÷ ±38, con 1,8 mm di aria dal bordo del labbro), perni Ø8,1 con collare Ø10,1 che puntano verso l'interno, e un ponte a Z ≈ 52 che lega i fianchi al blocco centrale. Verificato sullo STEP: i fianchi sono piastre **continue su tutti gli 80 mm** da Z 10,8 a Z 34,2, quindi l'anello di forza fra perno basso e perno alto sta interamente dentro un unico pezzo di materiale, e le due stazioni sono legate rigidamente
- [2026-09-26] **L'ordine dei pezzi lungo l'asse del motore è obbligato:** corpo motore → flangia → pignone → cremagliera. L'albero esce dal lato flangia, quindi mettendo il motore fuori dai cuscinetti il pignone finirebbe dentro, dove c'è il binario. Conseguenza: il corpo del motore deve stare verso l'interno, cioè sopra il carrello, e questo fissa l'altezza minima dell'asse
- [2026-09-26] **Interasse dei 608 a 23,40 mm** (labbro 1,3 + due raggi da 11 = 23,30), cioè 0,10 mm di gioco nella pinza. Validato: mezzo carrello scorre su tutta la lunghezza in entrambi i versi. Da rifare con il carrello completo, perché con entrambi i lati montati conta la complanarità dei due labbri lungo la corsa
- [2026-09-26] **Doppia elica a 30° su pignone e cremagliera, confermata funzionante.** Verificate sullo STEP la compatibilità geometrica (stesso modulo trasversale, stesso angolo, apice a metà fascia su entrambi) e la dentatura (spessore in testa 1,065 e vano al piede 1,191 sulla cremagliera, spessore su primitiva 2,3562 nominale sul pignone: gioco totale 0,20 tutto sulla cremagliera). Prezzo della doppia elica: il pignone va centrato sull'apice della cremagliera entro **±0,17 mm**, contro i ±2 mm che darebbero i denti retti
- [2026-09-26] **Il pignone non ha vite di fermo, e va bene così.** L'accoppiamento a pressione è talmente serrato che per smontare la prova sono serviti morsetti concatenati. La vite serviva a poter *regolare* la posizione assiale per il centraggio della doppia elica, ma quella regolazione si è spostata sulla cremagliera. Da ricontrollare fra qualche settimana di funzionamento, perché il PLA sotto tensione circonferenziale si rilassa; il sintomo di uno slittamento è un disegno progressivamente sfalsato senza alcun errore segnalato
- [2026-09-26] **Alzare il piano del foglio è la leva più efficace di tutto il progetto**, non un dettaglio ergonomico. Con 0,10 mm di gioco su 60 mm di interasse ruote l'inclinazione è 1,67 mrad, e l'errore sulla punta scala linearmente con l'altezza della penna: 0,250 mm a 150 mm di sbalzo, 0,117 a 70 mm. La risoluzione della trasmissione è 0,029 mm, quindi a 150 mm l'errore geometrico è otto volte la risoluzione. Vincolo: il carrello spazza una striscia larga ±38 mm attorno all'asse del binario da Z 2,7 a Z 74, e il piano non può entrare in quel volume
- [2026-09-26] Metodo per misurare l'attrito, da usare come metrica di qualità confrontabile: cercare l'**angolo minimo a cui il carrello si muove da solo** sul binario inclinato. La tangente è il μ effettivo — 5° = 0,09 (buono), 30° = 0,58 (qualcosa striscia). Con 1,5 kg in movimento, μ 0,09 costa 20 N·mm al pignone e μ 0,58 ne costa 130, su un NEMA 17 che alle velocità di lavoro ne dà 150÷200 utili
- [2026-09-26] **Dati reali del motore** (17HS4023 iMetrx, etichetta): coppia di tenuta **130 mN·m**, corrente nominale **1,0 A/fase**, resistenza **~4,0 Ω/fase** (4,1 V a 1,0 A), peso **204 g**. È un pancake 42 × 42 × **23 mm**, non un NEMA 17 standard: circa **un terzo** della coppia di un 40 mm
- [2026-09-26] **Corrente, calore e coppia sono la stessa manopola.** La coppia scala con la corrente; la corrente che si può dare la decide il PLA della culla. Con P = 2·I²·R sui 74 cm² di superficie del motore, e tenuto conto che la culla come disegnata copre il **51%** di quella superficie: a 0,6 A la carcassa sta a **74 °C con la culla piena** e a **58 °C con la culla alleggerita**. Alleggerire la culla (nervature invece di superfici piene sul fondo e sui due fianchi) vale quindi **0,1 A di corrente in più, cioè il 13% di coppia** — non è rifinitura
- [2026-09-26] **Bilancio di coppia al punto di lavoro** (0,6 A, culla alleggerita): coppia 78 mN·m, e con il fattore 0,85 per la caduta a 21 giri/min su un raggio primitivo di 15 mm restano **4,42 N alla cremagliera**. Togliendo la quota per accelerare, l'angolo di distacco ammesso è 14° a 1,3 kg e 10° a 1,6 kg; **il target con margine 2× è sotto i 7°**. Il fattore 2 non è pignoleria: senza encoder una perdita di passo non viene segnalata da nulla e il disegno esce sbagliato da lì in poi
- [2026-09-26] **Leve in riserva se la coppia risulta stretta**, in ordine di convenienza: (1) togliere massa, che agisce su entrambi i termini; (2) accelerazione da 1000 a 400 mm/s², che restituisce 0,9 N di budget attrito e si paga solo in tempo; (3) pignone z17 invece di z20 — **+18% di forza e risoluzione da 33,95 a 39,9 passi/mm, con la stessa cremagliera** perché dipende solo dal modulo, al costo di rifare la catena delle quote (l'asse scende di 2,25 mm); (4) motore da 40 mm, che triplica la coppia per ~10 € e +140 g
- [2026-09-26] **La massa punisce due volte** (aumenta insieme l'attrito e la forza per accelerare), quindi il test della calamita sui profili pesa più di quanto sembri: la trave da 385 mm vale 174 g in alluminio contro 506 in acciaio, cioè **circa due gradi** di angolo ammesso in meno

- [2026-09-27] **Carrello X completo misurato:** angolo di distacco ~10°, moto costante su tutta la corsa ~15°, massa 290 g. Tradotto in forza sono **0,50 N e 0,76 N** contro i 4,42 N disponibili al pignone: margine 5,8×. Il μ apparente di 0,27 non descrive un attrito grande, descrive un attrito piccolo diviso per un peso ancora più piccolo
- [2026-09-27] **Il drag è attrito interno dei cuscinetti, quindi costante e non proporzionale al carico.** Otto 608ZZ con coppia di spunto 0,3÷1,0 mN·m su raggio 11 danno 0,25÷0,83 N, che copre esattamente il misurato. Conseguenza operativa: **caricando il portale il μ effettivo scende**, perché il numeratore resta fermo e il denominatore cresce. A 1,1 kg totali il μ scende da 0,27 a ~0,082
- [2026-09-27] **Test della calamita fatto: i profili NON sono magnetici, quindi alluminio.** 452 g/m sulla sezione a cappello (167 mm²), contro i 1314 g/m che avrebbe l'acciaio. Chiude il punto aperto dal 2026-09-25
- [2026-09-27] **Il tetto di massa mobile non è più un vincolo di progetto.** Con drag da rotolamento (μ 0,05) il tetto è 3410 g a 0,6 A contro i ~1150 g stimati per la macchina completa. Fra il formato più piccolo e il più grande in tabella ballavano 84 g: la massa ha smesso di essere il criterio di scelta del formato
- [2026-09-27] **Cremagliera v2 controllata sullo STEP, tre difetti trovati e corretti in v3.** (a) Due fori Ø8 passanti dall'alto tagliavano la dentatura fino in testa: a Y=0 mancavano due denti interi a X≈14 (salto di 11,551 invece di 4,712) e uno a X≈256, con la fascia portante che crollava da 17,32 a 9,32 mm; (b) restavano due schegge di dente spesse **0,030 mm**, sotto la larghezza dell'ugello, che sarebbero diventate bave dentro un vano; (c) il Ø2,5 passante avrebbe autofilettato nel PLA impedendo il serraggio, e il cono si apriva solo a Ø4,5, troppo poco per una testa svasata M3
- [2026-09-27] **Cremagliera v3 verificata pulita:** 59 denti, tutti spessi 2,1562 sulla primitiva, tutti i passi 4,7124, nessuna anomalia. Profilo confermato al centesimo a sette altezze (testa 1,0643, primitiva 2,1562, piede 3,5211, pendenza fianco 2·tan20° = 0,728). **Chevron simmetrico a ogni altezza di fascia**: le coppie ±Y danno posizioni identiche fino all'ultima cifra. Lunghezza 284,223 = 60,314 passi, avanzo 1,480 — da portare a **282,743** (60 passi esatti) perché il pezzo resti giuntabile in futuro
- [2026-09-27] **Il rettangolo ruotato a 45° sul piatto è davvero l'ottimo**, verificato: con piatto 256 e larghezza 60 il massimo è 302,04 mm esatti a 45°. Dettaglio controintuitivo da ricordare allo slicer: **ruotare poco peggiora**, fra 5° e 25° l'ingombro ammesso scende sotto i 256 mm che si avrebbero a 0°. O si va a 45 o non si ruota
- [2026-09-27] **Il riempimento non raggiunge mai i denti.** Con ugello 0,4 e 4 perimetri il dente è pieno ovunque (2,52 mm di pareti contro 2,156 di spessore sulla primitiva). Il pattern influenza solo la soletta, che regge 4,4 N su 11 viti: la scelta va fatta sulla **stabilità dimensionale**, non sulla resistenza
- [2026-09-27] **Altezza del pignone ricavata dal carrello:** i fori NEMA sono a Z 58,650 con interasse 31,00 verificato, quindi **asse pignone Z 74,150**, primitiva cremagliera Z 59,150, faccia inferiore della cremagliera Z 47,275. Equazione di vincolo della catena verticale: **D = t_binario + 59,150 − t_cremagliera**, dove D è la distanza dalla faccia inferiore della cremagliera alla primitiva (oggi 11,875). Da confermare sul pezzo reale: l'asse del motore deve venire a filo del cielo del carrello, che in CAD sta a Z 74,159
- [2026-09-27] **La catena delle quote non chiude sui nominali, e va chiusa in montaggio.** Sensibilità: 1 mm di errore in altezza = 0,728 mm di gioco, quindi il budget è ±0,275 mm. Le tolleranze reali sommano ±0,60 mm col multistrato e ±0,89 col massello — due e tre volte il budget. I contributi: taglio del legno ±0,30, **planarità del riporto su 284 mm ±0,50**, movimento stagionale ±0,05 (multistrato di costa) contro ±0,66 (massello di costa), stampa ±0,10, alluminio ±0,10
- [2026-09-27] **Il regolatore della catena verticale sono i fori Ø4,0 del motore con viti M3**: ±0,5 mm di gioco radiale, cioè ±0,364 mm di gioco d'ingranamento. Basta ad assorbire tutto quello che resta dopo aver misurato il legno
- [2026-09-27] **Architettura dell'asse Y aperta.** Frequenze proprie della trave da 437 mm con 430 g al centro: **145 Hz in verticale, 388 Hz in orizzontale** (la direzione di corsa è quella larga del profilo). Freccia 0,013 mm, e comunque **la freccia non tocca l'ingranamento** perché cremagliera e pignone stanno entrambi sulla trave e scendono insieme. La rigidezza non è un criterio di scelta del profilo
- [2026-09-27] **Il piano di simmetria della sezione azzera due errori su tre.** Con penna e pignone sull'asse dell'anima, l'imbardata non sposta la punta (una rotazione non muove ciò che sta sull'asse) e la torsione è nulla per definizione (forza sul piano di simmetria). Fuori asse si paga doppio: a 150 mm di offset sono 0,250 mm di imbardata **più** 0,209 di torsione della trave, contro uno spessore di tratto di 0,3÷0,5 mm
- [2026-09-27] **Il motore non deve contendere lo spazio alla penna nella sezione: può essere sfalsato lungo la corsa.** In sezione si sovrappongono, in pianta stanno a 40÷60 mm di distanza. Così penna, pignone e cremagliera restano tutti sul piano di simmetria
- [2026-09-27] **Un perno stampato non si può ingrossare fra due cuscinetti coassiali**, perché il cuscinetto interno si infila dalla punta e non passerebbe. La soluzione è quella già usata sull'asse X: **due tronchetti contrapposti**, uno per cuscinetto, radicati ciascuno nella propria parete. Lunghezza 8÷10 mm invece di 30, e la freccia scende col cubo: da 0,0149 a 0,0004 mm. Se invece il perno deve restare passante, va tenuto Ø8 pieno e **appoggiato a entrambe le pareti** (0,0009 mm contro 0,0149 a sbalzo, 16× in rigidezza e 4× in resistenza)
- [2026-09-27] **La resistenza dei perni stampati non è mai stata il problema:** un Ø8 a sbalzo su 30 mm rompe a 84 N e ne porta 1, coefficiente 84×. Il motivo per irrigidirli è lo **scorrimento viscoso**: il PLA sotto carico permanente triplica la freccia in un anno, quindi 0,045 mm che si presentano da soli come gioco sul labbro
- [2026-09-27] **Il profilo a cappello dell'asse X non è riutilizzabile sull'asse Y** con la conformazione scelta: è una C con le punte che formano un pezzo esterno, e i cuscinetti non possono fare il giro necessario. Cade l'ipotesi di riusare lo stesso profilo su entrambi gli assi
- [2026-09-28] **Trave Y comprata e misurata sullo STEP.** Alluminio, sezione a T asimmetrica: ala **67,100 × 1,500** (bordi a Y −42,050 e +25,050, quindi 42 mm di ala da un lato dell'anima e 25 dall'altro), anima **3,100** centrata su Y 0, altezza totale **23,500**. Due bande asolate sull'ala, a Y 11,05…15,65 e −11,75…−7,15, larghe 4,6 e piene all'82%
- [2026-09-28] **La prima cosa da verificare su un profilo asolato è dove passano i cuscinetti.** Le piste scelte (Y 18,05…25,05 e −22,95…−15,95) cadono fuori dalle bande asolate con 2,40 e 4,20 mm di margine: un 608 che passa sopra un'asola non fa rumore, fa un salto di quota che la penna disegna. Verificato anche il contrario, che serve per l'anti-sollevamento: il piano Z 0,000 della trave misura 3106,2 mm² su 1002 di lunghezza, cioè **1002 × 3,100 esatti — il bordo inferiore dell'anima è continuo, nessuna asola lo interrompe**
- [2026-09-29] **Schema di guida del carrello Y:** quattro 608 che *appoggiano* sul piano superiore dell'ala (non pinzati), due 608 sotto l'ala in anti-sollevamento, quattro MR63 che pinzano l'anima lateralmente. Il carrello poggia, non è precaricato in verticale: è lo schema giusto, e il gioco sui due inferiori non è gioco di scorrimento ma corsa prima che l'anti-sollevamento intervenga. Passo perni 60,000 come sull'asse X, carreggiata 41,000
- [2026-09-29] **Il perno superiore lungo non era un problema.** Era la preoccupazione dell'utente: un Ø8 stampato da ~52 mm che porta due cuscinetti. In realtà è appoggiato sul tetto per 29 mm al centro e a sbalzo solo 6,45 e 5,55 alle estremità: **0,26 MPa** contro i ~50 del PLA, freccia 0,0004 mm. Cade l'idea del pezzo centrale avvitato fra i cuscinetti
- [2026-09-29] **Difetto che si ripete di versione in versione: i 608 del lato +Y sono a filo esatto del bordo dell'ala** (Y 18,050…25,050 contro bordo 25,050). Il carrello trasla di 0,10 per lato col gioco dei MR63, quindi in esercizio il cuscinetto sborda e rotola sullo spigolo, che su un estruso è anche la zona con raggio di bordo e bave. La fascia pulita è larga 9,40: la posizione giusta è **17,05…24,05**. Segnalato su prova 1, 2, 3 e 4, mai corretto
- [2026-09-29] **Motore sotto o sopra: il conto non era quello che sembrava.** L'argomento dell'utente ("sopra devo alzare la penna di 5 cm") non regge sui numeri — l'attacco del modulo penna è più basso di soli 13 mm col motore sotto, ma col motore sotto tutta la macchina deve stare 19 mm più alta sulla carta, quindi il modulo penna col motore sopra viene **6 mm più corto**, non 50 più lungo. L'argomento che regge davvero è un altro, ed è migliore: **il corpo del motore è alluminio**, quindi quei millimetri di discesa non li fa un braccio stampato in PLA
- [2026-09-30] **La gabbia a cavallo dell'anima forza la quota del motore.** Interno Y da −6,750 a +17,250: il corpo può vivere solo sotto il bordo inferiore dell'anima, e il bordo superiore del motore finisce 0,35 mm sotto il piatto del carrello. Non c'è margine verso l'alto. Da qui discende tutto il resto: la cremagliera non arriva all'ala e deve scendere fino al pignone
- [2026-09-30] **Pattern dei fori motore sbagliato per tre versioni di fila:** 31,000 × **32,000** su prova 2 e 3, poi 31,000 × **31,450** su prova 4. Il NEMA 17 è 31,000 × 31,000. Con Ø4 su M3 ci sono 0,5 di raggio libero, quindi entra comunque — ma l'errore se lo mangia tutto, e proprio su **Z, che è l'asse dell'interasse**: resti senza registrazione del gioco. Su prova 4 la fila inferiore è esatta (−51,500 = −36,000 − 15,500) e **solo la fila superiore è alta di 0,450**. Il carrello X, controllato per confronto, ha 31,000 esatti: la copia non aveva portato la quota perché il carrello X ha una sola fila di due viti, non quattro
- [2026-09-30] **La doppia elica di cremagliera e pignone aveva mano opposta: non potevano ingranare.** Inseguendo un singolo dente al variare di Y, la cremagliera dà pendenza dX/dY **+0,60** e il pignone **−0,576**: stesso angolo di 30°, ma le punte delle V guardano da parti opposte e le tracce dei denti si incrociano a 60° invece di essere parallele nel punto di contatto. Verifica: 74,1 mm³ di compenetrazione che non scende sotto ~35 né ruotando il pignone, né facendolo rotolare davvero (rotazione + traslazione r·θ), né aumentando l'interasse di 0,8 mm. **La correzione è gratuita: si gira la ruota sottosopra** (essendo simmetrica rispetto al piano di mezzeria, ribaltarla inverte la mano senza spostare nient'altro) e la compenetrazione va a **0,000** con una finestra di gioco di ~0,26 mm. Ruotarla sul mozzo, che era l'istinto, cambia la fase e non la mano
- [2026-09-30] **Correzione di un errore mio sul segno dello 0,1 di aria fra cremagliera e ala.** Avevo scritto che stringendo la cremagliera contro l'ala l'ingranamento si stringe: è il contrario. Denti in giù e pignone sotto, quindi tirandola su l'interasse **aumenta** e il gioco cresce. Il margine di montaggio voluto dall'utente va nel verso giusto
- [2026-09-30] **La cremagliera Y non raggiunge le asole della trave:** il suo piano superiore va da Y −13,822 a −42,278, le asole sono a −11,750…−7,150. Vanno forati fori nuovi nell'ala, e la zona giusta è **Y −28…−38** per tre motivi che coincidono: è oltre Y −24,950 dove finisce il carrello (teste delle viti di qualsiasi altezza, montaggio dall'alto), è oltre Y −22,950 quindi fuori dalla pista dei cuscinetti 5 e 6, ed è oltre Y −27,678 dove finisce la gamba verticale, quindi le cave esagonali per i dadi sono accessibili da sotto
- [2026-10-01] **Prima stampa del carrello Y: si solleva.** Premendo con un dito sul retro del motore il carrello si alzava. Causa: niente teneva giù il lato −Y (i 608 lì sopra solo appoggiano), e qualsiasi forza applicata molto sotto l'ala genera un momento enorme. I numeri dicono che non era una sensazione: a stallo il momento di beccheggio è 324 N·mm, cioè **±5,4 N** contro 2,15 N di peso per stazione — una stazione si stacca. In marcia normale non solleva ma **dimezza il carico** su una stazione, ed è quella cedevolezza che poi esce come isteresi nel disegno
- [2026-10-01] **La nuova catena di tolleranze è il rischio principale aperto.** Il progetto dipende ormai da **tre quote della trave comprata, nessuna misurata**: spessore ala 1,500 (presa 608, gioco progettato 0,100), spessore anima 3,100 (MR63 laterali, 0,050), e la novità, **altezza totale 23,500** (MR63 anti-sollevamento, **gioco 0,000**). Prima la presa verticale lavorava su 1,5 mm di lamiera, adesso lavora su tutta l'altezza del profilo — che per di più è fatto di due profili a L imbullonati, quindi ha anche il problema che **le due gambe devono finire a filo sul bordo inferiore**, altrimenti un cuscinetto da 2,5 corre mezzo a sbalzo su un bordo da 3,1
- [2026-10-01] **Altezza della cremagliera Y fissata dalla catena:** con asse pignone a Z −36,000 e raggio operativo 15,119 (il Ø testa 33,237 dà x = +0,079), la primitiva va a **Z −20,881**, la punta del dente a **−22,381**, il dorso contro l'ala a 22,000 → **altezza totale 44,381**. Cresciuta di 7,3 mm rispetto al layout precedente, conseguenza diretta dell'aver abbassato il motore. La lama alta non preoccupa: la forza tangenziale è nel piano della lama e la doppia elica non genera spinta assiale, quindi non c'è niente che la fletta in Y
- [2026-10-01] **Lunghezza della cremagliera Y: due pezzi identici da 37 passi** (174,359 ciascuno, 348,718 totali, ~322 di corsa utile). In un pezzo solo non ci sta: servirebbero ~343 di dentatura, il piatto a 45° con larghezza 28,456 ne ammette 333,6 teorici e ~300 pratici. A 174 mm si stampano **dritti sul piatto**, niente rotazione a 45°. Due regole per il giunto: ogni pezzo multiplo esatto di 4,7124 tagliato su un confine di dente, e **il passo attraverso il giunto lo deve dare l'incastro fra i due pezzi** (maschio/femmina, stesso file) e non la foratura dell'alluminio, che vale ±0,2
- [2026-10-01] **Gioco e scorrevolezza non sono la stessa cosa, e vanno curati con strumenti diversi.** Dopo la prima stampa i giochi nominali sono stati stretti (608 da 0,150 a 0,100, MR63 laterali da 0,100 a 0,050, anti-sollevamento a 0,000). Tarare su una stampa reale è il metodo giusto, ma 0,050 è **sotto la tolleranza di stampa**: la prossima stampa può uscire con 0,15 di gioco o con 0,05 di interferenza, e in nessuno dei due casi c'è modo di aggiustare. Il rimedio al gioco è la **regolazione** (asole), non un nominale più stretto — che aggiunge precarico su tutta la corsa e lo fa pagare dove il profilo è un po' più spesso, con un punto duro. L'attrito è stato il vincolo del progetto per settimane: non va buttato via per una sensazione al tatto

**Problemi aperti su questo step (al 2026-10-01):**
- ~~Profondità del vano motore da verificare~~ — **risolto il 2026-09-26.** Il motore è un **pancake da 23,1 mm**, non un NEMA 17 standard da 34÷48: il vano da 23,5 mm è quindi corretto e le viti posteriori arrivano eccome al coperchio. Resta però **0,4 mm di gioco assiale**, che è più dei ±0,17 mm di tolleranza di centraggio della doppia elica: il motore va messo **in battuta sulla parete anteriore** (che è il riferimento della quota del pignone) con **due rondelle piane M3 da 0,5 mm** fra coperchio posteriore e parete posteriore, così le viti di dietro lo spingono in avanti invece di tirarlo indietro. Resta da verificare la profondità dei fori filettati posteriori (spesso 4÷5 mm) per scegliere la lunghezza delle viti
- ~~Test della calamita sui profili~~ — **risolto il 2026-09-27: alluminio**, 452 g/m. Toglie di mezzo il rischio peggiore sul peso del portale
- ~~Attrito del carrello completo da caratterizzare~~ — **risolto il 2026-09-27.** 10° di distacco, 15° di moto costante, e la causa è attrito interno dei cuscinetti (costante), non strisciamento (proporzionale). Il carrello è accettabile così com'è
- ~~Profilo della trave Y da comprare~~ — **risolto il 2026-09-28.** Trave a T in alluminio, ala 67,100 × 1,500, anima 3,100, altezza 23,500, due bande asolate. Le piste dei cuscinetti cadono fuori dalle asole con 2,40 e 4,20 mm di margine
- ~~Layout del carrello Y da chiudere~~ — **chiuso nella sostanza il 2026-10-01** attraverso quattro iterazioni (prova 1→4): guida, motore sotto a cavallo dell'anima, anti-sollevamento sull'anima, cremagliera sotto l'ala su base alta. Resta aperto il solo **modulo penna**
- **PRIORITÀ: misurare le tre quote della trave Y prima della prossima stampa.** Spessore ala (serve 1,500), spessore anima (3,100) e **altezza totale** (23,500), cinque punti ciascuna col calibro. È mezz'ora e risparmia due ristampe: il gioco progettato sull'anti-sollevamento è **0,000**, quindi o si pianta o esce con tre decimi di gioco, e non c'è modo di saperlo prima. Verificare anche che **le due gambe del profilo finiscano a filo sul bordo inferiore**: se una sporge di 0,2 il MR63 da 2,5 ci corre sopra da solo, mezzo a sbalzo su un bordo da 3,1
- **Fori motore, fila superiore: da Z −20,050 a −20,500.** Unica quota ancora sbagliata del pattern. Finché resta così il centro del pattern cade 0,225 sopra l'asse del pignone, cioè −0,164 di gioco d'ingranamento, e la registrazione via Ø4 è esaurita
- **Verificare la mano dell'elica anche sull'asse X.** Se la cremagliera Y è stata ricavata da quella X ribaltandola per portare i denti in giù, le due hanno mano opposta e **la stessa ruota va montata girata in un verso sull'asse X e nell'altro sull'asse Y**. Metodo: metterle in mesh nello stesso assieme e guardarle dall'alto — le V devono puntare nello stesso verso di quella che stanno ingranando
- **Asole di registrazione, le due che mancano.** (a) I due MR63 laterali di un lato con asola 3,2 × 4,5 in Y, per azzerare il gioco laterale con un cacciavite invece che con una ristampa; (b) i due MR63 anti-sollevamento con asola 3,2 × 5 in Z, che è **l'unico punto della macchina dove si può registrare la presa verticale**. Senza queste due, ogni taratura è una ristampa
- **I 608 del lato +Y vanno portati da Y 18,050…25,050 a 17,05…24,05.** Si perde 1 mm di carreggiata e si smette di rotolare sullo spigolo dell'ala
- **Cremagliera Y da disegnare**, ora che le quote ci sono: altezza totale **44,381** (dorso contro l'ala a 22,000, punta del dente a −22,381), due pezzi identici da **37 passi = 174,359**, incastro maschio/femmina al giunto, fori nuovi nell'ala a **Y −28…−38** con cave esagonali per i dadi. Piano superiore 5 mm con M3 oppure 7 mm con M4 (la forza separatrice col pancake è solo 2,66 N, M3 basta)
- **Modulo penna da progettare**, ultimo pezzo del carrello Y. Attacco previsto sulla schiena del motore: **verificare che il pancake abbia davvero i 4 filetti posteriori**, perché su molti non ci sono e tutto il piano dipende da quelli. Vincolo di precisione: l'imbardata residua è 0,050/60 = 0,000833 rad, quindi la penna a Y −60 costa 0,050 mm di isteresi — col pancake il bilanciamento **non** è più un vincolo (baricentro a Y +6 con il motore montato, resta dentro la carreggiata anche con 80 g a Y −60)
- **Interferenza geometrica fra cremagliera e fianco del carrello, asse X.** Vincolo: `X asse pignone ≤ −46,8`, calcolato su una cremagliera larga 15,588 centrata sull'apice. **Da ricalcolare sulla v3**, che è larga 38,66 con la flangia tutta da un lato
- **Misurare il legno prima di stampare la cremagliera X definitiva.** Il listello grande e i listelli da 8 vanno misurati col calibro in cinque punti, poi si imposta `D = t_binario + 59,150 − t_cremagliera` e si stampa. **La cremagliera si stampa dopo che il legno è in casa, non prima.** Se la dispersione fra i cinque punti supera 0,27 mm il riporto è imbarcato e va spianato, oppure si passa al piano B
- **Piano B se il legno esce storto:** buttare il riporto e appendere la cremagliera a **staffe stampate fissate al binario**. Una nervatura da 10 mm che sale 36 mm si flette di 0,002 mm sotto la forza di separazione. Vantaggio concettuale: la cremagliera verrebbe riferita al binario, cioè allo stesso pezzo su cui corre il pignone, invece che al tavolo
- **Parallelismo cremagliera-binario entro 0,17 mm su tutta la corsa.** Conseguenza della doppia elica: se la cremagliera non è parallela al binario nella direzione della larghezza, il centraggio del pignone sull'apice deriva mentre il carrello avanza. Su 400 mm di corsa significa 0,024°. Va allineata **al binario**, non al bordo della base: far scorrere il carrello e controllare la distanza pignone-apice in quattro o cinque punti prima di fissare. Sintomo caratteristico di un errore: scorre bene in mezzo e si irrigidisce verso le estremità
- **Gioco della pinza con il carrello X completo**: se si irrigidisce in un punto della corsa non è la stampa ma la complanarità dei due labbri, e la cura è portare l'interasse dei 608 da 23,40 a 23,50
- **Quanto alzare il piano del foglio**, che resta bloccato sul modulo penna: serve la quota della punta a riposo

## Storico sessioni

### [2026-10-01 18:00] (meccanica) Trave Y comprata, carrello Y dalla prova 1 alla 4, e la catena di quote che si sposta tutta sulla trave

**Riepilogo:** comprata e misurata la trave a T dell'asse Y, progettato il carrello Y in quattro iterazioni fino alla prima stampa, trovati e corretti tre errori geometrici (dentature con elica di mano opposta, pattern fori motore, cremagliera che non raggiunge le asole), e deciso di mettere il motore sotto la trave a cavallo dell'anima — scelta che obbliga la cremagliera a scendere su una base alta 44 mm e sposta le tolleranze critiche su tre quote di un profilo comprato e non ancora misurato.

**Cosa è stato fatto:**

- **Trave Y acquistata e rilevata dallo STEP**: alluminio, T asimmetrica, ala 67,100 × 1,500, anima 3,100, altezza totale 23,500, due bande asolate sull'ala. Verificato che le piste dei cuscinetti cadono fuori dalle asole (2,40 e 4,20 mm di margine) e che il bordo inferiore dell'anima è continuo per tutta la lunghezza — condizione che ha poi reso possibile l'anti-sollevamento
- **Carrello Y progettato in quattro versioni** (prova 1 → 4), con controllo booleano di compenetrazione a ogni passo: dall'ultima versione tutte le coppie (ruota, mozzo, dieci cuscinetti, carrello, trave) danno zero
- **Catena verticale chiusa**: asse pignone a Z −36,000, primitiva cremagliera a −20,881, altezza cremagliera 44,381
- **Lunghezza della cremagliera Y decisa**: due pezzi identici da 37 passi, stampati dritti sul piatto

**Bug: le due dentature avevano l'elica di mano opposta**

**Sintomo:** 74,1 mm³ di compenetrazione booleana fra pignone e cremagliera Y nell'assieme. Non scendeva sotto ~35 mm³ né ruotando il pignone sul suo asse, né facendolo rotolare davvero (rotazione + traslazione r·θ), né aumentando l'interasse fino a 0,8 mm.
**Causa:** inseguendo un singolo dente al variare di Y, la cremagliera dà pendenza dX/dY **+0,60** e il pignone **−0,576**. Stesso angolo d'elica (30°), ma le punte delle V guardano da parti opposte: le tracce dei denti si incrociano a 60° invece di essere parallele nel punto di contatto. Probabile origine: la cremagliera Y è stata ricavata da quella X ribaltandola per portare i denti in giù, e quel ribaltamento inverte la mano.
**Fix applicato:** si gira la ruota sottosopra. Essendo simmetrica rispetto al piano di mezzeria, ribaltarla inverte la mano senza spostare nient'altro. Verificato: compenetrazione **0,000 mm³** con finestra di gioco ~0,26 mm. **Non** si risolve ruotandola sul mozzo, che era l'istinto iniziale: quella cambia la fase, non la mano. Da verificare la stessa cosa sull'asse X, dove potrebbe servire l'orientamento opposto.

**Bug: il carrello Y si solleva sotto una forza applicata al motore**

**Sintomo:** alla prima stampa, premendo con un dito sul retro del motore il carrello si alzava dalla trave.
**Causa:** niente teneva giù il lato −Y (i 608 di quel lato solo appoggiano sull'ala) e il motore sta 59 mm sotto il piano di appoggio, quindi qualsiasi forza lì genera un momento grande. Confermato dai numeri: la forza tangenziale agisce sul punto primitivo a Z −20,881, l'attrito che la contrasta sui 608 a Z 23,500, braccio 44,381 mm scaricato sui 60 mm di passo perni → a stallo ±5,4 N contro 2,15 N di peso per stazione.
**Fix applicato:** due MR63 ad asse Y che spingono verso l'alto contro il bordo inferiore dell'anima. Verificato in CAD, da verificare sul pezzo stampato. In marcia normale il momento non solleva ma dimezza il carico su una stazione, ed è quella cedevolezza che si traduce in isteresi nel disegno.

**Bug: pattern dei fori motore sbagliato per tre versioni**

**Sintomo:** interasse fori NEMA 17 misurato 31,000 × 32,000 su prova 2 e 3, 31,000 × 31,450 su prova 4. Il valore corretto è 31,000 × 31,000.
**Causa:** l'utente aveva riprodotto i fori del carrello X, che però ha **una sola fila di due viti** (31,000 in una direzione sola, perché la coppia superiore cadrebbe fuori dal pezzo): la seconda quota non c'era da copiare ed è stata disegnata a mano.
**Fix applicato:** parziale. Su prova 4 la fila inferiore è esatta (−51,500 = −36,000 − 15,500), **la superiore è ancora alta di 0,450** e va portata a −20,500. Non impedisce il montaggio (Ø4 su M3 dà 0,5 di raggio libero) ma consuma tutta la registrazione proprio su Z, che è l'asse dell'interasse.

**Decisioni prese:**

- *Contesto:* dove mettere il motore dell'asse Y. **Decisione:** sotto la trave, con la gabbia centrata sul piano dell'anima. *Alternative scartate:* **motore sopra il carrello** — verificato che l'argomento dei "5 cm di penna in più" non regge (l'attacco del modulo è più basso di soli 13 mm col motore sotto, ma la macchina deve stare 19 mm più alta, quindi il modulo verrebbe 6 mm più corto col motore sopra); ha deciso l'argomento vero, cioè che **il corpo del motore è alluminio** e quei millimetri di discesa non li fa un braccio stampato in PLA. **Motore tutto fuori sul lato −Y** con la cremagliera direttamente sotto l'ala — geometricamente funziona, ma porta il baricentro a Y ≈ −37, fuori dalla carreggiata, e l'ala finisce a −42,05 quindi non c'è dove allargare l'appoggio. *Da rivedere se:* il modulo penna risultasse scomodo da attaccare dietro al motore, o se il franco verticale sotto il portale diventasse critico
- *Contesto:* come impedire al carrello di sollevarsi. **Decisione:** due MR63 ad asse Y sotto il bordo inferiore dell'anima. *Alternative scartate:* una seconda coppia di 608 sotto l'ala, che avrebbe leva migliore (Y −19,45 invece di Y 0) ma cade esattamente dove scende la cremagliera. *Da rivedere se:* l'altezza reale della trave si rivelasse troppo variabile per un gioco di 0,000 — nel qual caso serve comunque l'asola in Z
- *Contesto:* lunghezza della cremagliera Y, servendo ~343 mm di dentatura. **Decisione:** due pezzi identici da 37 passi (174,359), incastro maschio/femmina al giunto, stampati dritti sul piatto. *Alternative scartate:* pezzo unico — il piatto a 45° con larghezza 28,456 ammette 333,6 mm teorici e ~300 pratici, non bastano. *Da rivedere se:* la corsa Y richiesta cambiasse
- *Contesto:* il carrello stampato scorreva con troppo gioco. **Decisione (dell'utente):** stringere i nominali — 608 da 0,150 a 0,100, MR63 laterali da 0,100 a 0,050, anti-sollevamento a 0,000. *Riserva registrata:* 0,050 è sotto la tolleranza di stampa, quindi la prossima stampa può uscire con 0,15 di gioco o 0,05 di interferenza senza modo di aggiustare. Il rimedio strutturale resta l'**asola**, non il nominale più stretto, che aggiunge precarico su tutta la corsa e lo fa pagare dove il profilo è più spesso

**Correzioni a quanto detto in sessione:**
- Avevo scritto che stringendo la cremagliera contro l'ala l'ingranamento si stringe. È il contrario: denti in giù e pignone sotto, quindi tirandola su l'interasse **aumenta**. Il margine di montaggio voluto dall'utente andava nel verso giusto
- Avevo concluso che la coppia pignone-cremagliera "non può ingranare" e che andava specchiata la cremagliera. La diagnosi (mano opposta) era giusta ma la correzione no: basta **girare la ruota sottosopra**, cioè si risolve in montaggio come aveva intuito l'utente, solo con un movimento diverso da quello che pensava

**File consegnati/modificati:**
- `documento-sessione-cnc-2d.md` — versione 16
- STEP analizzati (non in repo): `asse_Y_-_staffa_carrello_prova_1`, `asse_Y_prova_2`, `asse_Y_prova_3`, `asse_Y__prova_4`, più il confronto con `tot_prova_3` (carrello X) e `ingranaggio_e_cremagliera_prova_1`

**Impatto su Vision/Pipeline:** Step 5 — aggiunte due decisioni progettuali (motore Y sotto la trave a cavallo dell'anima; anti-sollevamento sul bordo inferiore dell'anima), 17 note di cronologia, e riscritto il blocco dei problemi aperti: chiusi il profilo della trave Y e il layout del carrello Y, aperta come priorità la misura delle tre quote della trave.

---

### [2026-09-27 17:32] (meccanica) Carrello completo misurato, falso allarme sullo strisciamento, cremagliera chiusa e apertura dell'asse Y

**Riepilogo:** Il carrello X completo scorre e l'attrito che sembrava strisciamento si è rivelato attrito interno dei cuscinetti — cioè costante, quindi destinato a migliorare sotto carico — e con la massa che smette di essere un vincolo il formato sale ad A4 orizzontale; la cremagliera arriva a una versione geometricamente corretta dopo tre difetti trovati sullo STEP, e si apre l'asse Y con due principi che hanno già riscritto lo sketch.

**Cosa è stato fatto:**

- **Carrello X completo stampato e assemblato.** 290 g, distacco ~10°, moto costante ~15°. In forza sono 0,50 e 0,76 N contro 4,42 disponibili: margine 5,8×
- **Test della calamita**: profili non magnetici, quindi alluminio a 452 g/m invece dei 1314 dell'acciaio
- **Controllo dello STEP della cremagliera v2**, tre difetti trovati, e **verifica della v3** che li ha chiusi tutti: 59 denti tutti a 2,1562 sulla primitiva, tutti i passi a 4,7124, chevron simmetrico a ogni altezza di fascia fino all'ultima cifra
- **Ricostruita la catena verticale** dal carrello: asse pignone Z 74,150, primitiva Z 59,150, e l'equazione `D = t_binario + 59,150 − t_cremagliera` che lega le due quote di legno allo spessore stampato
- **Analisi delle tolleranze della catena verticale**, che ha ribaltato una raccomandazione data poche ore prima (vedi sotto)
- **Aperta la progettazione dell'asse Y**: quattro sketch discussi (1, 1.1, 1.2, 2), scritta la specifica del profilo da comprare, e calcolate frequenze proprie (145 Hz verticale, 388 orizzontale) e contributi d'errore del carrello

**Bug: l'attrito del carrello scambiato per strisciamento**

**Sintomo:** angolo di distacco 10° e moto costante 15° su un carrello da 290 g, cioè μ apparente 0,18÷0,27 contro lo 0,01÷0,02 che darebbero dei cuscinetti a sfere. Un test con 300 g aggiunti non ha fatto scendere l'angolo, il che indicava attrito **proporzionale al carico**, cioè radente.
**Causa:** nessun meccanismo di strisciamento esisteva davvero. La verifica diretta — far girare a dito tutti e 12 i cuscinetti e cercare contatti del PLA sul binario — li ha trovati tutti liberi e nessun contatto. Il drag è **attrito interno dei cuscinetti**: otto 608ZZ con coppia di spunto 0,3÷1,0 mN·m su raggio 11 danno 0,25÷0,83 N, che copre esattamente i 0,50÷0,76 misurati. Il test con i 300 g era l'unica misura fatta a occhio su una differenza sottile (15° contro ~8°) ed è stato battuto da un'osservazione binaria.
**Fix applicato:** nessuna modifica al pezzo — il carrello va bene com'è. Cambia l'interpretazione: essendo il drag **costante e non proporzionale**, caricando il portale il μ effettivo **scende** (da 0,27 a ~0,082 a 1,1 kg totali) invece di restare. Metodo migliore per una prossima verifica: tirare il carrello in piano con una bilancia da valigia e leggere i grammi, invece di stimare angoli a occhio.

**Bug: due fori passanti distruggevano la dentatura della cremagliera**

**Sintomo:** sullo STEP della v2, scansione sulla primitiva a Y=0: fra il dente a X 6,586 e il successivo a 18,137 c'erano 11,551 mm invece di 4,712, e un secondo salto di 9,425 a X≈256.
**Causa:** due viti di battuta erano state messe al centro della cremagliera, con fori Ø8 passanti **dall'alto**, che salivano fino a Z 3,375 tagliando i denti fino in testa. Il danno era un intaglio a V centrato sull'apice: a |Y|≥4,5 i denti erano interi, a Y=0 ne mancavano due. Restavano anche due schegge di dente spesse 0,030 mm, sotto la larghezza dell'ugello.
**Fix applicato:** fori rimossi in v3, verificato sullo STEP. Verificato anche che le sole 11 asole laterali bastano: la forza di separazione dell'ingranamento (1,60 N) spinge la cremagliera **verso il legno**, mai in su, e il rollio residuo attorno alla fila di viti vale 0,008 mm, il 4% del gioco.

**Decisioni prese:**

- Contesto: la massa mobile sembrava il vincolo che teneva la macchina piccola. Una volta stabilito che il drag è costante, il tetto è salito da 1283 a 3410 g contro ~1150 stimati, e fra il formato più piccolo e il più grande ballavano 84 g.
  Decisione: **formato A4 in orizzontale**, corsa X ~230 con la cremagliera da 59 denti, corsa Y ~317.
  Alternative scartate: A4 verticale e formato quadro 300×300 — richiedono 317÷320 mm di corsa X, quindi una cremagliera giuntata; e l'A4 verticale non serve, perché per disegnarlo basta ruotare il file di 90° nel software. A5, che non aveva più nessuna ragione di essere.
  Da rivedere se: il portale finito pesa molto più dei ~1150 g stimati.

- Contesto: allungare la corsa X richiederebbe due segmenti di cremagliera.
  Decisione: **un pezzo solo, 282,743 mm (60 passi esatti)**, che resta giuntabile in futuro senza ridisegnare nulla.
  Alternative scartate: segmentare subito. Il rischio al giunto è asimmetrico — largo di 0,10 è un neo estetico in un punto, **stretto di 0,20 inceppa** — ed è un difetto permanente che si scopre dopo aver forato il listello.
  Da rivedere se: serve un formato oltre l'A4.

- Contesto: la catena delle quote verticali non chiude sui nominali. Sensibilità 0,728 mm di gioco per mm di altezza, budget ±0,275, tolleranze reali ±0,60 col multistrato.
  Decisione: **la cremagliera si stampa dopo aver misurato il legno**, impostando D sulle misure reali; il residuo si registra ai fori Ø4 del motore (±0,5 mm = ±0,364 di gioco) con il metodo della carta da 0,10 mm nel punto più stretto.
  **Supera la raccomandazione data poche ore prima nella stessa sessione** («lascia il file com'è e taglia il legno a 55,3»), che valeva solo finché il legno lo tagliava l'utente. Comprandolo già dimensionato, l'unico pezzo controllabile diventa quello stampato.
  Alternative scartate: **stare stretti e comprimere legno o PLA con le viti**. Il PLA a 15 MPa resta elastico e cede 0,02 mm, inutile; il legno di costa a quella pressione si impronta in modo permanente e continua a scorrere per settimane, di una quantità dello stesso ordine dell'intero budget. E stretto è il lato pericoloso: largo si shimma in dieci minuti, stretto va piallato un listello già forato e montato.
  Da rivedere se: il listello arriva imbarcato oltre 0,27 mm su 284 — allora si passa alle staffe stampate sul binario.

- Contesto: legno massello o multistrato per il riporto della cremagliera.
  Decisione: **multistrato, ricavato di costa da un pannello.**
  Alternative scartate: massello, che trasversalmente alla fibra si muove dell'1,2% su un'escursione stagionale — **0,66 mm su 55**, più del doppio dell'intero budget, con la macchina che si inceppa d'inverno e balla d'estate. Il multistrato nel piano si muove 0,05 mm.

- Contesto: scelta del riempimento della cremagliera fra gyroid, nido d'ape e triangoli.
  Decisione: **gyroid 25%, 4 perimetri, layer 0,16 mm.** Con 4 perimetri i denti sono massicci ovunque, quindi il pattern conta solo per la stabilità dimensionale: il gyroid non ha tratti rettilinei lunghi (nessuna direzione di ritiro privilegiata su 284 mm) e non si incrocia mai con se stesso.
  Alternative scartate: nido d'ape, che ripassa ogni cella più volte per layer sprecando tempo per una resistenza non necessaria; griglia (l'impostazione di partenza), che fa passare l'ugello sopra linee già depositate a ogni intersezione.

- Contesto: dove attaccare la trave dell'asse Y. Il piano iniziale erano le 4 viti superiori del motore, lasciate libere apposta.
  Decisione: **la trave Y si attacca al corpo del carrello.** Rafforza il punto già aperto il 2026-09-26 con una ragione nuova e decisiva: il motore sta a 58÷74 °C e il PLA ha la Tg a 60 — una staffa imbullonata sulla sua faccia sarebbe il pezzo di plastica **più caldo e più caricato della macchina**, in condizioni da manuale di scorrimento viscoso, mentre tutto il resto lavora a temperatura ambiente. Accettata dall'utente.
  Alternative scartate: attacco sulle viti del motore. Oltre al termico: rende i due estremi del portale diversi per costruzione (il motore ce l'ha un lato solo), e cambierebbe da entrambe le parti se si aggiunge il secondo motore.

- Contesto: dove mettere penna, motore e cremagliera sul carrello Y.
  Decisione: **tutti sul piano di simmetria della sezione**, con penna al centro dell'impronta dei cuscinetti e motore sfalsato **lungo la corsa** invece che di fianco. Azzera due errori su tre: l'imbardata non sposta ciò che sta sull'asse, e una forza sul piano di simmetria non genera torsione. Corollario: **la sezione deve avere un piano di simmetria**, quindi T e non L.
  Alternative scartate: penna all'estremità di un carrello a C (sketch 2) — 0,250 mm di imbardata più 0,209 di torsione, contro uno spessore di tratto di 0,3÷0,5.
  Da rivedere se: non si trova una T e nemmeno due angolari accoppiabili.

- Contesto: la cremagliera dell'asse Y su una seconda trave dedicata.
  Decisione: **cremagliera e guida sullo stesso corpo rigido**, con la cremagliera su una staffa stampata della trave che porta i cuscinetti.
  Alternative scartate: seconda trave. Farebbe dipendere la profondità d'ingranamento da quanto due pezzi distinti restano paralleli su 437 mm. Una staffa stampata si flette di 0,002 mm: per tenere una cremagliera non serve una trave.

- Contesto: il tratto di perno stampato fra due cuscinetti superiori coassiali, ~30 mm all'aria.
  Decisione: **due tronchetti contrapposti**, uno per cuscinetto, radicati ciascuno nella propria parete — la stessa soluzione già in uso sull'asse X, che mantiene l'accoppiamento Ø8,1/collare Ø10,1 già validato.
  Alternative scartate: **ingrossare il perno in mezzo** (suggerimento sbagliato, corretto dall'utente: il cuscinetto interno si infila dalla punta e non passerebbe oltre un tratto più grosso). Scartato anche il perno d'acciaio, che introdurrebbe un accoppiamento nuovo da mettere a punto per un guadagno che non serve — la resistenza ha già un coefficiente 84×, e il motivo per irrigidire è solo lo scorrimento viscoso.

**File consegnati/modificati:**
- `documento-sessione-cnc-2d.md` — v14 → v15
- Controllati (non modificati da me): `cremagliera_prova_2.step`, `cremagliera_prova_3.step`, `tot_prova_3.step`
- Nessun file di progetto toccato: la sessione è stata di analisi, misura e progettazione

**Impatto su Vision/Pipeline:** Step 5 — l'obiettivo passa da **A5 a A4 in orizzontale**, e la decisione sul formato del 2026-09-21 viene superata (testo precedente conservato). Aggiornato il blocco dei problemi aperti: chiusi il test della calamita, la caratterizzazione dell'attrito e la profondità del vano motore; aperti la misura del legno prima della stampa, il piano B con staffe sul binario, la specifica del profilo Y e il layout del carrello Y.

---

### [2026-09-26 17:48] Carrello validato in stampa, cremagliera verificata, e bilancio di coppia del motore

**Riepilogo:** il carrello a U rovesciata passa tutte le verifiche strutturali sullo STEP e scorre perfettamente in stampa con mezzo set di cuscinetti; la cremagliera modulo 1,5 con gioco 0,2 è disegnata, verificata al centesimo e provata con il pignone a doppia elica; intercettati prima della stampa quattro errori (interasse fori NEMA sbagliato di 2 mm, due perni fuori quota di 5 centesimi, cremagliera asimmetrica rispetto all'apice, dato di coppia del motore sbagliato di 3,3× nell'annuncio); e chiuso il bilancio di coppia, che con un motore pancake da 130 mN·m rende l'angolo di distacco del carrello un passa/non passa sotto i 7°.

**Cosa è stato fatto:**
- **Messa a punto della lettura degli STEP** come strumento di verifica: estrazione dei nomi dei componenti, ingombri, volumi, tutti i cilindri con diametro/asse/direzione, distanze e compenetrazioni fra solidi, sezioni ASCII e sonde puntuali per la continuità del materiale. Su questa base sono state fatte tutte le verifiche sotto
- **Carrello (`tot_prova_3`) verificato e approvato.** Corpo unico, fianchi da 3 mm continui su tutti gli 80 mm fra Z 10,8 e Z 34,2, perno alto annegato con 18 mm di materiale sopra (non a sbalzo come temuto), ponte a Z 52 che lega i fianchi al centro, **0,4 mm di aria dal binario** su tutta la corsa, 12 cuscinetti tangenti senza compenetrazione, 7,95 mm dal legno
- **Prova di stampa del carrello riuscita**: con i cuscinetti di sinistra e quelli centrali, inclinando il binario di ~30° il carrello scorre su tutta la lunghezza e torna indietro senza impuntarsi
- **Cremagliera disegnata, quotata e validata.** Consegnato il disegno con la cella unitaria da 6 punti per il pattern, la tabella nominale/con gioco e il posizionamento rispetto all'asse motore. Verificata sullo STEP dell'utente: passo 4,7124, spessore in testa 1,065, vano al piede 1,191, altezza 3,375 — tutto entro il mezzo centesimo
- **Stampa di prova della cremagliera riuscita e ingranamento con il pignone perfetto**, il che conferma anche che i due V della doppia elica sono concordi
- **Bilancio di coppia chiuso** sui dati reali dell'etichetta del motore (130 mN·m, 1,0 A, ~4,0 Ω, 204 g, pancake da 23 mm): punto di lavoro a 0,6 A con culla alleggerita → 4,42 N alla cremagliera, e target di attrito sotto i 7° di angolo di distacco. Trovato e quantificato il legame **corrente → calore → coppia**: alleggerire la culla del motore vale 0,1 A, cioè il 13% di coppia
- **Verifica del vano motore chiusa**, e il problema aperto segnalato in questa stessa voce risulta inesistente: il motore è un pancake da 23,1 mm, quindi il vano da 23,5 mm va bene. Emersi però 0,4 mm di gioco assiale, che superano i ±0,17 mm di centraggio della doppia elica
- **Montaggio del motore a 2 viti anteriori + 2 posteriori validato coi numeri**: le quattro viti formano un rettangolo di 31 × 40 mm nel piano orizzontale, e i carichi dell'ingranamento si traducono in 1,5 N e 4,1 N per coppia di viti. Confermata anche la scelta di non appendere l'asse Y al motore
- **Trovata un'interferenza geometrica** fra la cremagliera e il fianco del carrello: con il pignone a 1 mm dal fianco, il bordo interno della cremagliera cade 0,8 mm dentro il carrello. Vincolo ricavato: asse pignone a X ≤ −46,8
- **Alimentatore cambiato**: non più il 24 V della striscia LED ma un DVE DSA-36W-12 da **12 V / 3 A / 36 W**. Verificato che tre motori a 0,6 A assorbono 0,85 A (28% della portata) e che la tensione non incide né su coppia né su temperatura, perché l'A4988 regola corrente e non tensione

**Bug: interasse dei fori di fissaggio del motore sbagliato di 2 mm**

**Sintomo:** nessuno ancora — intercettato nel file `tot_prova_2` mentre la stampa di prova era in corso
**Causa:** i fori erano a 33,0 mm di interasse; il NEMA 17 li ha normati a 31,0. Ogni foro fuori di 1 mm, contro i ±0,5 che perdona un foro Ø4 attraversato da una vite M3
**Fix applicato:** corretto a 31,0 in `tot_prova_3`, verificato. Resta uno scarto di 0,62 mm fra il centro del pattern e l'asse del foro Ø37 (0,37 in Y, 0,50 in Z), che però è innocuo perché la cremagliera verrà posizionata a partire dal motore già montato

**Bug: due perni alti fuori quota di 5 centesimi**

**Sintomo:** nessuno — trovato confrontando tutti i cilindri dello STEP fra loro
**Causa:** i due perni della stazione a Y 0 erano Ø8,05 con collare Ø10,05, gli altri sei Ø8,1 / Ø10,1. Uno sketch modificato su una stazione e non sull'altra. Cinque centesimi stanno dentro il campo in cui la tolleranza dei perni era stata tarata, quindi quei due cuscinetti sarebbero venuti più lenti — e sono i perni alti, quelli che portano il peso
**Fix applicato:** uniformati tutti e otto a Ø8,1 — confermato dall'utente

**Bug: dato di coppia contraddittorio nell'annuncio del motore**

**Sintomo:** la descrizione dichiara "coppia elevata (130 mN·m (60 oz.in))", ma i due valori non sono la stessa cosa — 60 oz·in valgono 424 mN·m, cioè 3,3 volte 130
**Causa:** copia-incolla dalla variante da 40 mm, venduta sulla stessa pagina dal medesimo venditore
**Fix applicato:** assunto **130 mN·m**, verificato per coerenza fisica — la coppia scala col volume del rotore, e un 40 mm a 1,7 A dà 40 N·cm, quindi dimezzando il pacco e scendendo a 1,0 A si arriva esattamente a 12÷13 N·cm. Usando il dato sbagliato il bilancio di coppia sarebbe stato sovrastimato di tre volte, con il rischio di scoprirlo solo a macchina montata

**Bug: cremagliera asimmetrica rispetto all'apice del chevron**

**Sintomo:** nessuno in funzionamento, ma margine del pignone ridotto a 0,93 mm su un lato contro 2,66 sull'altro
**Causa:** nel trimmare la larghezza da 17,321 a 15,588 il taglio è stato fatto su un solo lato, lasciando l'apice del V a 8,660 da un bordo e 6,928 dall'altro. Con ±0,17 mm di tolleranza di centraggio, 0,93 è sottile — e soprattutto impedisce di riferire la cremagliera al proprio bordo in fase di montaggio
**Fix applicato:** resa simmetrica — confermato dall'utente

**Decisioni prese:**

- Contesto: la staffa del binario va rialzata di ~5 mm, e con l'asse motore a Z 74,65 la cremagliera cade a 53 mm sopra il piano del legno, servendo un supporto molto alto
- Decisione: **rialzo in legno, e motore lasciato alla quota attuale.** Un listello alto 53 mm avvitato alla base è più rigido di qualunque cosa stampata di quell'altezza, costa due euro e si fa in venti minuti
- Alternative scartate: (a) abbassare il motore infilandolo fra le due stazioni di cuscinetti, che avrebbe portato il supporto a 30 mm ma richiedeva di allargare l'interasse ruote da 60 a 70 mm perché il corpo da 42,3 entrasse nella finestra da 38; (b) motore ad asse verticale con cremagliera a denti laterali, supporto a 15 mm, scartata perché il pignone sarebbe finito in fondo a 24 mm di albero a sbalzo
- Da rivedere se: il listello di legno si muove con l'umidità in modo da alterare l'ingranamento lungo la corsa

- Contesto: serviva decidere se l'alloggiamento del motore fosse un pezzo separato bullonato o integrato nel carrello, e come garantire la regolazione dell'ingranamento in assenza di asole
- Decisione: **alloggiamento integrato nel carrello, e la cremagliera viene posizionata a partire dal motore già montato.** È il motore a dire dove va la cremagliera, non il contrario
- Alternative scartate: alloggiamento bullonato con asole verticali da ±2 mm, che era la raccomandazione iniziale. Diventa superfluo se l'elemento regolabile è la cremagliera
- Conseguenza operativa: la regolazione deve comunque esistere, e si sposta tutta sulla cremagliera — spessori sotto per la profondità di ingranamento, fori Ø5 nel listello per viti M3 per il centraggio laterale della doppia elica, e fori pilota della cremagliera forati **dopo** aver trovato la posizione. Ordine di montaggio: binario → carrello → motore → cremagliera
- Da rivedere se: si rende necessario smontare il motore dopo aver fissato la cremagliera, perché a quel punto non resta margine di recupero

- Contesto: il motore ha quattro fori di fissaggio anche sul coperchio posteriore, e l'utente voleva riservare i fori superiori all'attacco dell'asse Y
- Decisione: **due viti sulla flangia anteriore e due sul coperchio posteriore.** Verificato coi numeri: le quattro viti formano un rettangolo di 31 × 40 mm nel piano orizzontale, e i carichi dell'ingranamento si traducono in 1,5 N e 4,1 N per coppia di viti. Il motore è pienamente vincolato
- Alternative scartate: quattro viti sulla sola flangia anteriore, che avrebbe richiesto di portare la flangia fino a Z 90 occupando lo spazio destinato all'asse Y

- Contesto: il motore è un pancake da 23 mm con 130 mN·m, un terzo di un NEMA 17 da 40, e serviva capire quanto margine di coppia resta
- Decisione: **punto di lavoro a 0,6 A con la culla del motore alleggerita** (4,42 N alla cremagliera, carcassa a 58 °C), e **l'angolo di distacco del carrello diventa un passa/non passa: target sotto i 7°**, non più solo una metrica di qualità
- Alternative tenute in riserva, in ordine: accelerazione a 400 mm/s², pignone z17 (+18% forza *e* risoluzione migliore, stessa cremagliera), secondo motore, motore da 40 mm
- Da rivedere se: l'angolo misurato sul carrello completo supera i 10°

- Contesto: l'utente ha 5 motori e ha proposto di motorizzare anche il lato passivo per raddoppiare la forza sull'asse
- Decisione: **tenuta come riserva, non adottata adesso.** Raddoppia davvero la forza (4,42 → 8,84 N) e porta l'angolo tranquillo da 7° a 17°, al costo di 214 g di massa in più
- Realizzazione preferita se si adotta: **due motori in serie su un solo driver** — zero GPIO in più, zero driver in più, lockstep elettrico perfetto. A 12 V però la caduta resistiva si mangia 6,0 V dei 12 contro i 3,0 di un motore per driver, quindi con un A4988 di scorta conviene la strada dei due driver separati (`motor0`/`motor1` sullo stesso asse in FluidNC). In entrambi i casi i due lati vanno messi in fase meccanicamente al montaggio, facendo scorrere la seconda cremagliera con i motori alimentati
- Avvertenza registrata: **il secondo motore non riduce l'attrito, dà solo la forza per vincerlo.** Se il carrello parte a 20° e si aggiunge il motore, si è nascosto un difetto che continuerà a consumare alluminio e a fare rumore. Misurare prima l'angolo, decidere dopo

**File consegnati/modificati:**
- Disegno quotato della cremagliera m1,5 con gioco 0,2 — pubblicato come artefatto, aggiornato una volta (orientamento di stampa e metodo di fissaggio)
- `documento-sessione-cnc-2d.md` — questa voce, correzioni alle decisioni di Step 5 superate, nuove note di Step 5

**Impatto su Vision/Pipeline:** Step 1 — registrato il cambio di alimentatore (12 V / 3 A), che supera il dato del 2026-09-19. Step 5 — due decisioni progettuali superate (cuscinetti 623ZZ → 608 + MR63; precarico a molla → gioco sul dente più registrazione dell'ingranamento), quindici note nuove di cronologia, e la sezione dei problemi aperti rivista: il vano motore risulta risolto, mentre si aggiungono il parallelismo cremagliera-binario entro 0,17 mm e l'interferenza geometrica fra cremagliera e fianco. Nessun cambio alla Vision.

---

### [2026-09-25 12:17] Materiali acquistati: il progetto si adatta a quello che esiste in ferramenta

**Riepilogo:** scelto il cuscinetto giusto con un calcolo di pressione di contatto (623ZZ, Ø10, foro M3), acquistati i materiali reali, e scoperto che il profilo a U trovato in ferramenta ha esattamente le misure che servono per il canale progettato.

**Cosa è stato fatto:**
- Calcolata la pressione di contatto per nove misure di cuscinetto, da cui la scelta del 623ZZ
- Prodotto un modello 3D navigabile del carrello, con guide semitrasparenti per mostrare le ruote dentro il canale
- Acquistati i materiali: base, viti, angolare forato, profilo a U
- Verificate le corse ottenibili sulla base reale contro quelle necessarie per un A5
- Riviste tre proposte successive di carrello dell'utente, con le correzioni caso per caso

**Decisioni prese:**

- Contesto: i 608 acquistati risultavano troppo grandi (Ø22, metà della larghezza del motore) e gli MR62 disponibili in casa troppo piccoli
- Decisione: **623ZZ, 3 × 10 × 4**. A 3 N di carico preme sull'alluminio con 53 MPa, contro i 150 a cui cede — tre volte di margine. Canale interno 12 mm invece dei 24 richiesti dai 608
- Il motivo che ha pesato di più non è la pressione ma il foro: **3 mm significa M3, la stessa vite di tutto il resto della macchina.** Una sola misura di viteria su tutto il progetto
- Alternative scartate: 624ZZ (Ø13, più margine ma introduce le M4 come seconda misura); restare sugli MR62 (margine risicato e M2 scomode); restare sui 608 (canale da 24 mm, carrello sproporzionato)
- Supera la decisione del 2026-09-24 sui 608

- Contesto: l'utente proponeva un cuscinetto grande montato perpendicolare, in mezzo agli altri, per premere sulla guida
- Decisione: **scartata**
- Motivo: un cuscinetto rotola solo se il suo asse è perpendicolare alla direzione di marcia; con l'asse parallelo alla corsa striscia e si appiattisce in un punto. E un Ø32 in mezzo a quattro Ø22 sporgerebbe di 5 mm, diventando l'unico a toccare e sollevando gli altri
- È stata però recuperata l'intuizione sotto: un cuscinetto centrale su molla è un'alternativa valida all'eccentrico, scartata perché sulla guida serve rigidezza, non cedevolezza

- Contesto: l'utente proponeva superfici di rotolamento in gomma per chiudere il gioco da sole
- Decisione: **scartata sulle guide**
- Motivo: la gomma non elimina il gioco, lo trasforma in cedevolezza. Il carico cambia a ogni accelerazione e inversione, quindi il carrello si sposterebbe in modo variabile lungo tutto il disegno — peggio del gioco, che almeno è costante e compensabile. In più l'attrito di rotolamento si impenna e la gomma prende la forma se la macchina resta ferma
- Dove la cedevolezza serve è già prevista: la molla di precarico del pignone

- Contesto: come fissare i cuscinetti senza annegare nel calcolo degli ingombri di dadi e bulloni
- Decisione: **perni stampati Ø3 per le ruote fisse, viti M3 con tratto liscio per quelle regolabili**, e **asole al posto degli eccentrici**
- Motivo: il perno stampato regge (5,7 MPa contro i 50 del PLA) ma non si regola; l'asola dà tre millimetri di corsa dove un eccentrico ne dà uno. Una vite M8 con dado richiederebbe 30 mm di spazio contro i 7 del cuscinetto
- Supera l'indicazione sull'eccentrico data il 2026-09-24

- Contesto: l'utente proponeva di accorciare il carrello avvicinando le ruote, per guadagnare foglio
- Decisione: **non accorciare**, tenere le ruote a 80-100 mm lungo la corsa
- Motivo: con un decimo di gioco e la penna 60 mm sotto il piano delle ruote, 80 mm di interasse danno 0,075 mm di errore sulla punta e 40 mm ne danno 0,15 — al limite del visibile. E la corsa guadagnata non serve: sulla base reale ne avanzano già 65 mm
- Da rivedere se: la struttura definitiva riducesse la corsa disponibile sotto i 260 mm in Y

**File consegnati/modificati:**
- Pagina pubblicata: modello 3D navigabile del carrello — https://claude.ai/artifact/CTT88x6q9LKyBt84rNhPLA
- Pagina aggiornata: disposizione dei cuscinetti, con la tavola sulla disposizione in larghezza — https://claude.ai/artifact/KyA5zQpxbfWVwYeRFV9S6S

**Impatto su Vision/Pipeline:** Step 5 aggiornato nelle decisioni su cuscinetti, fissaggio, regolazione e base. Aggiunta la decisione del profilo unico a U, che rende superflua sull'asse X la regola della guida maestra (mantenuta per l'asse Y, dove le guide restano due pezzi distinti). Sostituita l'indicazione della base in MDF con il compensato effettivamente acquistato. Aggiunto il principio che lunghezza e larghezza del carrello hanno costi diversi.

---

### [2026-09-24 18:44] Dall'SVG al G-code nell'interfaccia, e progetto meccanico con modulo 1,5

**Riepilogo:** costruita da zero l'interfaccia web che porta un SVG fino al G-code ottimizzato con stima del tempo, e impostato il progetto meccanico — modulo 1,5 stampato, primo pignone provato, carrello su cuscinetti radiali 608 al posto dei cuscinetti lineari previsti.

**Cosa è stato fatto:**
- Servo validato al banco: cablaggio ripassato, prova partendo dal centro della corsa verso gli estremi, nessun ronzio di fondo corsa. Prodotto il G-code di prova con i movimenti della penna (Z-3 alzata, Z-7 abbassata, valori provvisori al centro della corsa)
- Costruita l'interfaccia web in `webui/`: impalcatura, piano macchina fisso con reticolo, foglio trascinabile, importazione SVG, ottimizzazione del percorso, generazione del G-code, stima del tempo
- Consegnate tre pagine pubblicate: anteprima dell'interfaccia, disegno quotato della cremagliera, disposizione dei cuscinetti sul carrello
- Stampato e provato il primo pignone, in versione dritta e a doppia elica
- Definite tutte le quote della cremagliera, i segmenti di stampa e lo schema di fissaggio

**Bug: quota della cremagliera etichettata male**

**Sintomo:** nel primo elenco di quote consegnato, la voce "larghezza del vano al fondo" riportava 3,721 mm.
**Causa:** 3,721 mm è lo spessore del *dente* misurato sulla linea di fondo. Il vano fra due denti è 0,992 mm. Errore di etichetta mio, non di calcolo.
**Fix applicato:** corretto e segnalato all'utente prima che disegnasse lo sketch. Nel disegno quotato definitivo le quote ambigue sono sostituite dalle **coordinate dei sei punti del profilo**, che non si prestano a interpretazioni.

**Bug: orientamento di stampa della cremagliera consigliato male**

**Sintomo:** avevo indicato di stampare la cremagliera coricata su un fianco, per avere il profilo del dente tracciato dall'ugello invece che dalla sovrapposizione degli strati.
**Causa:** il consiglio ignorava la flangia di fissaggio — che in quella posizione resta in aria — e sopravvalutava la debolezza fra gli strati.
**Fix applicato:** rifatto il conto. La forza sul dente è al massimo 9 N, che sulla sezione di base dà 0,08 MPa contro i 20÷30 MPa a cui cede l'adesione fra strati: trecento volte di margine. Consiglio cambiato in **denti in su, flangia sul piatto**, con strati da 0,10 mm perché a 20° di fianco ogni layer arretra di 0,036 mm.

**Decisioni prese:**

- Contesto: il modulo 1 previsto è al limite di quanto una FDM stampa in modo affidabile
- Decisione: **modulo 1,5 con 20 denti**, 33,95 passi/mm, risoluzione 0,029 mm
- Alternative scartate: modulo 1 (denti troppo fragili) e modulo 2 (risoluzione 0,039 mm e dente più alto, che rende l'allineamento col precarico più critico)
- Supera la decisione del 2026-09-21 — la risoluzione non è il fattore limitante, visto che l'errore reale sarà dieci volte maggiore, quindi il margine si spende in robustezza

- Contesto: avevo consigliato di comprare cremagliera e pignone in POM per contenere il gioco
- Decisione: **stampare tutto**, comprese le dentature
- Alternative scartate: acquisto della cremagliera — l'utente ha scelto la stampa, e la prova sul primo pignone ha confermato che le tolleranze reggono
- Da rivedere se: il provino da 25 denti mostrasse un ingranamento inaccettabile

- Contesto: erano stati consigliati cuscinetti lineari LM8UU su barre rettificate, ma l'utente ha acquistato cuscinetti radiali 608
- Decisione: **carrello con 608 che rotolano su guide in alluminio**, 11 per carrello, con una guida maestra e una secondaria libera lateralmente e un eccentrico per il contrasto laterale
- Alternative scartate: cuscinetto grande montato con l'asse parallelo alla corsa — non rotolerebbe, striscerebbe, appiattendosi in un punto; e un Ø32 in mezzo a quattro Ø22 diventerebbe l'unico a toccare, sollevando gli altri
- Supera la decisione del 2026-09-21 sulle guide a barre tonde

- Contesto: il pignone sembrava troppo grande perché i cuscinetti non trovavano spazio
- Decisione: **bande laterali separate** per motore, pignone con cremagliera e cuscinetti
- Alternative scartate: rimpicciolire il pignone — non risolverebbe, perché il vincolo viene dal corpo del motore che resta 42 × 42 comunque

**File consegnati/modificati:**
- `webui/` — nuova: `index.html`, tre fogli di stile, dieci moduli JavaScript, `tools/build.py`, `tools/make-sample.py`, `README.md`
- `gcode/test/quadrato-e-cerchio-penna.gcode` — nuovo, con i movimenti del servo
- `gcode/test/README.md` — aggiornato con le quote provvisorie della penna
- Pagina pubblicata: anteprima dell'interfaccia — https://claude.ai/artifact/Sq9d4MVjUBujRCgK682BXA
- Pagina pubblicata: disegno quotato della cremagliera — https://claude.ai/artifact/5TmiPjPf3sXv4orV8ntco7
- Pagina pubblicata: carrello su cuscinetti 608 — https://claude.ai/artifact/KyA5zQpxbfWVwYeRFV9S6S

**Impatto su Vision/Pipeline:** Step 4 passa da "da fare" a **in corso**, con cinque decisioni progettuali aggiunte e la cronologia dei progressi. Step 5 aggiornato nelle decisioni su guide, pignone e corse: le voci su LM8UU, modulo 1 e corsa Y da 260 mm sono state sostituite, e la sostituzione è motivata qui sopra.

---

### [2026-09-21 11:18] FluidNC installato, configurato e validato: la macchina si muove sotto G-code

**Riepilogo:** FluidNC 4.1.0 installato e messo in rete, configurazione accettata senza modifiche dallo schema 4.x, microstepping 1/16 verificato su entrambi gli assi e primi movimenti coordinati eseguiti — ma solo dopo aver trovato un errore di polarità che avevo introdotto io nella configurazione dell'ENABLE condiviso.

**Cosa è stato fatto:**
- Installazione di FluidNC v4.1.0 dal web installer, variante esp32-wifi, fresh-install con WebUI-2
- Configurazione WiFi in modalità client: hostname `cnc2d`, IP 192.168.1.11, mDNS attivo
- Caricamento e attivazione di `cnc2d-config.yaml`, con verifica riga per riga dell'output del validatore
- Verifica funzionale dei tre LED di stato via `M62`/`M63`
- Realizzazione dei ponticelli di microstepping (MS1/MS2/MS3 a VDD) su entrambi i driver, e verifica a 3,3 V con il multimetro prima di dare corrente
- Esecuzione del quadrato da 40 mm e del cerchio da raggio 20 mm: **Step 3 completato**
- Creati due file di G-code di prova nel repository, con il relativo README

**Bug: i motori non si muovono nonostante gli impulsi siano corretti**

**Sintomo:** FluidNC eseguiva i comandi di movimento in modo apparentemente perfetto — `ok` in risposta, coordinate che avanzavano da 0 a 40.000 alla velocità giusta, stato che passava da `Jog` a `Idle`, nessun errore da nessuna parte — ma i motori restavano completamente immobili e gli alberi molli al tatto. Ripetibile su entrambi gli assi.

**Causa:** nella configurazione avevo scritto `shared_stepper_disable_pin: gpio.27:low`. Il `:low` è sbagliato, e il motivo è controintuitivo: il pin dell'A4988 si chiama ENABLE ed è attivo basso, il che suggerisce appunto `:low`. Ma il campo di FluidNC si chiama *disable*, e ci viene scritto "vero" quando i motori vanno spenti; sull'A4988 si spegne portando il pin ALTO. Visto da FluidNC è quindi un disable **attivo alto**, cioè senza modificatore. Con `:low` la logica si ribalta e FluidNC, volendo tenere i motori sempre energizzati per via di `idle_ms: 255`, teneva il pin alto — cioè li teneva spenti.

**Fix applicato:** rimosso il `:low` dalla configurazione, file ricaricato sull'ESP32 e riavviato. Verificato: i motori ora sono energizzati fin dall'avvio e rispondono ai comandi. La prova diagnostica che ha isolato la causa è stata staccare il filo ENABLE dal lato del driver: l'A4988 ha un pull-down interno su quel pin, quindi scollegato si abilita da solo, e vedere il motore girare in quelle condizioni ha dimostrato che tutto il resto (VMOT, STEP, DIR, taratura del Vref, cablaggio) era a posto e che il guasto era nella polarità in configurazione.

**Bug: due errori di battitura diagnosticati come guasti hardware**

**Sintomo:** in due occasioni distinte, comportamenti che sembravano guasti seri. Primo caso: FluidNC continuava a caricare la configurazione di default ignorando la nostra. Secondo caso: nessun movimento dei motori accompagnato da una disconnessione dell'interfaccia.

**Causa:** nel primo caso il comando inviato era `$Config/Filename=cnc2d-config.yalm` — estensione `.yalm` invece di `.yaml`, con le due lettere centrali invertite. FluidNC cercava un file inesistente e ricadeva sul default, dicendolo chiaramente con `Cannot open configuration file`. Nel secondo caso il comando era `$J=G91 X40 F500v`, con una `v` in fondo: incollando con Ctrl+V, il campo comandi della WebUI in alcuni browser intercetta la scorciatoia a metà e scrive la lettera insieme al testo. Il comando veniva rifiutato con `error:2 Bad GCode number format` e non veniva eseguito nulla.

**Fix applicato:** comandi reinviati corretti. Nessuna modifica al sistema. Registrato nel README dei file di prova, perché la `v` fantasma è un comportamento del browser che si ripresenterà.

**Decisioni prese:**
- Contesto: i file di G-code di prova andavano eseguiti con il servo non ancora tarato
- Decisione: **nessun comando sull'asse Z nei file di prova**. Si muovono solo X e Y
- Alternative scartate: includere un alza/abbassa penna per provare anche il servo. Scartata perché gli estremi `min_pulse_us`/`max_pulse_us` in configurazione sono valori di partenza non verificati, e mandare un SG90 a una posizione arbitraria rischia di portarlo in battuta meccanica, dove assorbe molto e forza gli ingranaggi
- Da rivedere se: dopo la taratura del servo, i file di prova possono essere estesi con i movimenti di penna

**File consegnati/modificati:**
- `firmware/fluidnc/cnc2d-config.yaml` — corretta la polarità di `shared_stepper_disable_pin`, con commento esteso sul perché
- `firmware/fluidnc/README.md` — aggiunta la trappola della polarità dell'ENABLE e la prova diagnostica
- `gcode/test/quadrato-40mm.gcode` — nuovo
- `gcode/test/cerchio-r20.gcode` — nuovo
- `gcode/test/README.md` — nuovo

**Impatto su Vision/Pipeline:** Step 3 passa da "in corso" a **completato**. Aggiunte alle sue note la conferma che lo schema 4.1.0 accetta la configurazione scritta sul 3.x (il dubbio sollevato nella voce delle 09:57 è risolto), la procedura di attivazione effettivamente funzionante e l'esito delle verifiche su LED e microstepping.

---

### [2026-09-21 09:57] Progetto della struttura meccanica e installazione di FluidNC

**Riepilogo:** Progettata la meccanica — portale a cremagliera e pignone stampati in 3D su base di legno, formato A5 — e avviata l'installazione di FluidNC sull'ESP32 con le scelte di versione e interfaccia.

**Cosa è stato fatto:**

- Definita l'architettura meccanica completa e aggiunto lo **Step 5** alla pipeline, che finora non prevedeva nessuno step per la struttura fisica
- Individuato il rischio principale della cremagliera — il gioco fra i denti stampati — e la contromisura: motore su piastrina oscillante con precarico a molla contro la cremagliera
- Chiarito che il vincolo di ritegno del carrello e la libertà di flottare del motore non sono in conflitto, perché agiscono su corpi diversi
- Definito il metodo di taratura in loco di `steps_per_mm` e la misura del gioco
- Avviata l'installazione di FluidNC dal web installer, con le scelte guidate passo per passo

**Decisioni prese:**

- Contesto: come trasmettere il moto ai due assi
- Decisione: **cremagliera e pignone stampati in 3D**, con precarico a molla del pignone
- Alternative scartate: cinghia GT2 con motori fissi, che avrebbe gioco quasi nullo e meno massa mobile, ma richiede componenti acquistati. Resta la via di ripiego, sostituibile senza rifare la struttura
- Da rivedere se: il gioco misurato restasse sopra 0,3 mm anche dopo aver messo a punto il precarico
- Supera l'assunzione implicita del 2026-09-21 09:15, dove `steps_per_mm` era stato ipotizzato a 80 partendo da una trasmissione a cinghia

- Contesto: il formato massimo di lavoro, che fissa le corse e la luce del portale
- Decisione: **A5**, con corse di circa 200 × 260 mm
- Alternative scartate: A4, rimandato — un portale più largo accentua la tendenza del ponte a mettersi di traverso, essendo spinto da un lato solo

- Contesto: quale interfaccia web installare fra WebUI-2, WebUI-3 e FigUI
- Decisione: **WebUI-2**, perché è quella su cui è costruita la documentazione del wiki e nei prossimi giorni servirà leggerla
- Alternative scartate: le altre due, non per demerito ma perché se ne può installare una sola e occupano lo stesso spazio; tenerne una lascia anche memoria libera per la pagina che costruiremo

**File consegnati/modificati:**

- `documento-sessione-cnc-2d.md` — aggiornato alla versione 6, aggiunto lo Step 5

**Impatto su Vision/Pipeline:** Aggiunto **Step 5 — Struttura meccanica**, che copre una parte del progetto finora assente dalla pipeline. Aggiornata la nota sul microstepping dello Step 3, che assumeva una trasmissione a cinghia.

---

### [2026-09-21 09:15] Secondo asse, pen-lift e scelta di FluidNC come firmware definitivo

**Riepilogo:** Completato lo Step 2 con il secondo motore e il controllo del servo, poi presa la decisione architetturale del progetto — FluidNC al posto del firmware custom — e progettata l'interfaccia web che farà da slicer e da pannello di controllo.

**Cosa è stato fatto:**

- Cablato e collaudato l'asse Y (STEP D25, DIR D33, ENABLE condiviso su D27); firmware esteso a due assi con velocità e rampa indipendenti per asse, regolabili da slider
- Aggiunta l'inversione di direzione per asse, salvata in memoria non volatile: serve a rimediare a un asse montato al contrario senza toccare cavi né codice
- Aggiunto il controllo del servo pen-lift su D32 con due posizioni calibrabili, pilotato dal periferico LEDC nativo senza librerie esterne. All'accensione il servo parte rilasciato: uno che va in battuta contro un vincolo meccanico appena riceve corrente assorbe moltissimo
- Scritta la configurazione FluidNC di partenza e riqualificato lo sketch esistente come strumento di collaudo

**Bug: il motore Y gira da solo appena alimentato**

**Sintomo:** al primo collegamento del secondo driver, il motore Y ruotava in continuo verso destra senza nessun comando.
**Causa:** D25 e D33 non erano ancora configurati come uscite nel firmware. Restando ingressi ad alta impedenza raccoglievano il ronzio di rete e il rumore di commutazione, che il driver leggeva come impulsi di STEP.
**Fix applicato:** i pin STEP vengono portati a livello basso come prima istruzione di `setup()`, prima di qualsiasi altra inizializzazione. **Confermato.**

**Decisioni prese:**

- Contesto: per disegnare servono interpretazione del G-code, movimento coordinato dei due assi e pianificazione dell'accelerazione fra segmenti consecutivi — la parte dove si sbaglia in modi difficili da diagnosticare
- Decisione: **FluidNC diventa il firmware definitivo**; lo sketch custom retrocede a banco di collaudo hardware e resta nel repo, perché è lo strumento che ha permesso di trovare un driver con l'ingresso STEP in corto
- Alternative scartate: completare il firmware custom, stimato in settimane di lavoro per riprodurre funzionalità già collaudate su migliaia di macchine
- Nota: il lavoro sul generatore di G-code non dipende da questa scelta, perché produce G-code standard in entrambi i casi
- Da rivedere se: FluidNC si rivelasse troppo rigido per qualche esigenza specifica del pen-lift

- Contesto: come passare da un'immagine al G-code
- Decisione: tutta l'elaborazione nel browser con algoritmi di tracciamento deterministici, partendo dal solo SVG
- Alternative scartate: delegare la vettorializzazione a un modello generativo esterno con prompt da copiare e incollare — un modello linguistico non ricalca un'immagine, inventa percorsi plausibili e non ripetibili; e l'elaborazione a bordo dell'ESP32, fuori portata per la sua RAM

- Contesto: come conoscere l'area di lavoro senza finecorsa di riferimento
- Decisione: **azzeramento manuale** — le dimensioni del foglio si calibrano una volta sola e si salvano, l'origine si ristabilisce a inizio sessione portando il carrello nell'angolo. Due pulsanti nell'angolo in basso a sinistra restano come finecorsa rigidi di emergenza
- Alternative scartate: finecorsa di riferimento su entrambi gli assi, rimandabili in qualsiasi momento perché automatizzerebbero semplicemente la procedura manuale
- Conseguenza: i driver non vanno mai disabilitati, altrimenti un asse spostato a mano farebbe perdere la posizione. Annulla l'idea di un auto-spegnimento dopo inattività, discussa nella stessa sessione

- Contesto: l'alimentazione, dopo aver valutato di alimentare tutto dai 12 V con un regolatore
- Decisione: **si resta sulla batteria 18650** per la logica e il servo, con i motori sui 12 V. Consumo medio ~140 mA a 5 V, che su una cella da 3000 mAh dà una decina di ore — ben oltre le 5-6 richieste
- Alternative scartate: step-down 12 V→5 V per alimentare tutto dalla presa, e regolatore lineare 7805, scartato perché dissiperebbe 5 W con il servo in movimento e andrebbe in protezione termica facendo riavviare l'ESP32

**File consegnati/modificati:**

- `firmware/fluidnc/cnc2d-config.yaml` — creato
- `firmware/fluidnc/README.md` — creato
- `firmware/cnc2d_dashboard/cnc2d_dashboard.ino` — modificato (due assi, inversione, servo)
- `firmware/cnc2d_dashboard/README.md` — modificato (riqualificato come strumento di collaudo)

**Impatto su Vision/Pipeline:** Step 2 completato. Step 3 riscritto attorno a FluidNC e passato a "in corso". Step 4 riscritto da "software slicer" a "interfaccia web e generazione G-code", con le decisioni di architettura prese in questa sessione.

---

### [2026-09-20 18:18] Motore X funzionante: la causa era l'alimentatore, non il cablaggio

**Riepilogo:** Dopo una lunga diagnosi sistematica si è scoperto che l'alimentatore "24V" riciclato era in realtà un controller per strisce LED RGB con uscite in PWM — causa di tutti i sintomi della sessione e della distruzione di due driver A4988; sostituito con un alimentatore DC 12V/3A e con il terzo modulo, il motore X gira correttamente.

**Cosa è stato fatto:**

- Revisione completa del firmware, con tre difetti reali corretti: il movimento a giro completo bloccava per 3 secondi dentro l'handler HTTP affamando web server e OTA; lo stepping in `loop()` subiva il blocco di `server.handleClient()` producendo cadenze irregolari; DIR veniva riscritto a ogni impulso invece che una volta a inizio movimento
- Aggiunta una tab "Motore" con intervallo dei passi, numero di passi, rampa di accelerazione, abilitazione diretta del driver e forzatura dei singoli pin per la misura col multimetro; ogni movimento registra nel log i passi effettivamente emessi
- Scartata l'ipotesi delle resistenze di shunt sbagliate: lette `R100` sul modulo, quindi il Vref di 0,55 V per 0,69 A/fase era corretto fin dall'inizio
- Diagnosi condotta misurando uno per uno tutti i pin del driver (VDD, MOT, RST, SLP, EN, STEP, DIR) invece di procedere per ipotesi

**Bug: alimentatore di potenza inutilizzabile**

**Sintomo:** VMOT oscillante tra 1 V e 20 V invece che ferma; coppia di tenuta debolissima; passi singoli a volte corretti, rotazione continua mai; comportamento che cambiava toccando i fili.
**Causa:** l'alimentatore riciclato è un controller per strisce LED RGB. Il bianco è il +24V comune, rosso/verde/blu sono uscite di canale pilotate in PWM. Usare il rosso come massa significava alimentare il driver attraverso un interruttore che si apriva e chiudeva continuamente. Confermato misurando sotto carico su tutti e tre i canali.
**Fix applicato:** sostituito con alimentatore switching DC 12V/3A, misurato stabile a 12,2 V. **Confermato.**

**Bug: secondo driver A4988 con ingresso STEP in corto**

**Sintomo:** con l'alimentazione risolta l'albero era finalmente bloccato e il test ENABLE rispondeva, ma nessun comando muoveva il motore e il pin STEP sul driver leggeva sempre 0 V pur avendo continuità verso D4.
**Causa:** ingresso STEP in corto verso massa dentro il chip. Localizzato dividendo il nodo in due metà: lato ESP32 muto, lato driver 0,2 Ω, e sfilando il modulo la breadboard tornava muta. Danno provocato dall'alimentatore RGB.
**Fix applicato:** terzo modulo A4988, collaudato prima dell'installazione. **Confermato.**

**Bug: il motore gira solo con passi singoli (aperto dalla sessione precedente)**

**Sintomo:** vedi voce del 2026-09-20 09:45.
**Causa:** l'alimentatore RGB. L'ipotesi precedente sui pin GPIO16/17 riservati al PSRAM era **sbagliata**.
**Fix applicato:** risolto dalla sostituzione dell'alimentatore. **Confermato** — il motore X ora gira su comando dalla dashboard.

**Decisioni prese:**

- Contesto: l'alimentatore riciclato dalla striscia LED si è rivelato un controller PWM e aveva già distrutto due driver A4988
- Decisione: passare a un alimentatore switching DC **12V/3A**, e rendere obbligatorio un condensatore elettrolitico da 100 µF / 50 V tra VMOT e GND del driver
- Supera la decisione del 2026-09-20 09:45 sull'uso dell'alimentatore 24V riciclato, e corregge il dato allora registrato secondo cui "il filo bianco è il vero 24V+ e il rosso è il GND": il rosso non è una massa, è un'uscita di canale
- Alternative scartate: aprire l'alimentatore per bypassare il controller (pericoloso, è collegato alla rete); impostare il controller su un colore fisso al massimo (il MOSFET di canale non è dimensionato per un carico induttivo e qualsiasi cambio di modalità fermerebbe il motore)
- Da rivedere se: servisse più coppia ad alta velocità, caso in cui si potrebbe passare a 24V con un alimentatore DC vero

- Contesto: due moduli A4988 persi senza accorgersene subito, con ore di diagnosi spese a cercare il guasto altrove
- Decisione: collaudare ogni nuovo modulo prima di alimentarlo, verificando che la continuità tra STEP, DIR, EN e GND sia muta su tutti e tre
- Da rivedere se: si adottasse un driver diverso con pull-down interni sugli ingressi, che renderebbero il test non significativo

**File consegnati/modificati:**

- `firmware/cnc2d_dashboard/cnc2d_dashboard.ino` — modificato
- `firmware/cnc2d_dashboard/README.md` — modificato (formula del Vref e dipendenza dalle resistenze di shunt, condensatore di bulk obbligatorio, documentazione della tab Motore)
- `documento-sessione-cnc-2d.md` — aggiornato alla versione 4

**Impatto su Vision/Pipeline:** Step 2 resta "in corso" ma il primo motore è validato; restano il secondo asse e la verifica della velocità di lavoro. Aggiornata la decisione sull'alimentazione di potenza e aggiunto il collaudo di accettazione dei driver.

---

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
