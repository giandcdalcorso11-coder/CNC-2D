# CNC-2D Dashboard — ESP32 (strumento di collaudo)

> **Questo non è il firmware definitivo della macchina.** Il firmware finale è FluidNC,
> vedi `firmware/fluidnc/`. Questo sketch resta come **banco di collaudo dell'hardware**:
> serve a verificare il cablaggio, provare un driver nuovo, misurare i pin col multimetro
> e far girare i motori senza G-code. È lo strumento che ha permesso di individuare un
> driver con l'ingresso STEP in corto, e va tenuto per ogni volta che si tocca il cablaggio.

Dashboard web servita direttamente dall'ESP32, con controllo dei LED, configurazione Wi-Fi
da browser, log remoto, movimento manuale dei due assi e diagnostica dei pin.

## Hardware richiesto

- ESP32 alimentato via USB o batteria+interruttore (come da bring-up già completato)
- LED rosso + resistenza 220 ohm su D2 (GPIO2)
- LED verde + resistenza 220 ohm su D18 (GPIO18)
- LED giallo + resistenza 220 ohm su D19 (GPIO19)
- Driver A4988 motore X: STEP su D4, DIR su D26
- Driver A4988 motore Y: STEP su D25, DIR su D33
- ENABLE su D27, condiviso tra i due driver (come sulle schede CNC commerciali: non ha
  senso tenere un asse energizzato e l'altro no)
- **Condensatore elettrolitico da 100 µF / 35 V (minimo 47 µF) tra VMOT e GND del driver**,
  il più vicino possibile al modulo. Non è opzionale: senza di esso i picchi induttivi
  generati dalle bobine a ogni commutazione possono distruggere l'A4988.
- Servo per il pen-lift: segnale su D32, alimentazione 5 V **separata** dalla logica
  (un servo assorbe a strappi e può far cadere la tensione all'ESP32), massa in comune,
  più un condensatore elettrolitico sulla sua alimentazione: 100 µF basta per un SG90,
  un servo metal-gear da 15 kg ne vuole 470÷1000 µF e un alimentatore dedicato da 2 A.

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

Frecce sinistra/destra: pilotano il motore X. Frecce su/giù: pilotano il motore Y.
Un click singolo fa avanzare il motore di un singolo passo; tenendo premuto (dopo ~300ms)
il motore continua a girare finché non si rilascia il pulsante.

Pulsante centrale del joystick: alza e abbassa la penna. L'etichetta mostra lo stato
corrente (`PENNA` = alzata o rilasciata, `GIÙ` = appoggiata).

Pulsanti `360°`: disposti attorno alle frecce, ciascuno fa compiere un giro completo
(200 passi, cioè 1,8°/passo in full-step) all'asse e nel verso della freccia che affianca.
Il pulsante STOP ferma entrambi gli assi.

Pulsanti Rosso/Giallo/Verde: accendono/spengono rispettivamente i LED su D2, D19, D18,
per verificare che la dashboard comunichi correttamente con l'ESP32 e per avere indicatori
di stato distinti quando inizieremo a pilotare i motori.

Nota sulla direzione: se un asse risulta invertito rispetto a quanto ti aspetti, non serve
toccare i cavi né il codice — usa la spunta "Inverti direzione" nella tab Motore.

### Tab Motore (diagnostica)

Divisa in due colonne, una per asse, ciascuna con i propri parametri — X e Y avranno masse
diverse una volta montata la meccanica, quindi vogliono regolazioni indipendenti.

- **Velocità** (10–400 passi/s): 200 passi = un giro. Il limite superiore è volutamente
  conservativo: il motore è stato validato fino a 500 passi/s, qui ci si ferma a 400.
- **Accelerazione** (0–200 passi): su quanti passi si distribuisce la partenza. Il primo
  passo parte 4 volte più lento della velocità impostata e si accelera linearmente fino
  ad essa. A 0 la partenza è secca — utile per verificare fino a dove il motore aggancia
  da fermo, ma un motore fermo che non aggancia la frequenza di partenza si limita a vibrare.
- **Passi**: quanti passi eseguire col pulsante di quella colonna.
- **Inverti direzione**: ribalta il verso di quell'asse. Serve quando, montata la meccanica,
  un asse si muove al contrario rispetto alla freccia premuta: si risolve con una spunta
  invece di riaprire il cablaggio. L'impostazione è salvata in memoria non volatile e
  sopravvive ai riavvii.
- **Esegui i due assi insieme**: fa partire entrambi contemporaneamente, ciascuno con i
  propri parametri. Serve anche a verificare che l'alimentatore regga il consumo dei due
  motori in movimento.
- **Driver abilitati**: agisce direttamente sul pin ENABLE condiviso. Serve anche come verifica:
  togliendo la spunta i motori devono sbloccarsi (si girano a mano liberamente). Se restano
  bloccati, il segnale ENABLE non sta arrivando ai driver.
- **Test pin** (sezione richiudibile): forza STEP/DIR/EN a un livello fisso per misurarli
  col multimetro sul pin del driver.

La sezione **Penna (servo)** ha due slider per tarare l'angolo di penna alzata e penna
abbassata: rilasciando uno slider il servo si porta subito su quell'angolo, così si regola
guardando la penna invece che a tentativi. I due valori sono salvati in memoria non volatile.
"Rilascia" toglie il segnale PWM e il servo si ammorbidisce, smettendo di consumare e
scaldare. **All'accensione il servo parte rilasciato** e non si muove finché non glielo si
chiede: un servo che va in battuta contro un vincolo meccanico appena riceve corrente
assorbe moltissimo.

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
