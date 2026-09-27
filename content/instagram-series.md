# Serie Instagram/TikTok — Making of CNC 2D Plotter

Stile di riferimento: la serie di Giovanni Dapont sulla barchetta radiocomandata (TikTok/Instagram Reels).

## Format

- Durata massima: 2 minuti
- Musica di sottofondo + spezzoni video delle lavorazioni reali + voice over che spiega cosa si sta facendo
- Formato verticale 9:16
- Pubblicazione **non in tempo reale**: si racconta la lavorazione come se procedesse episodio per episodio dall'inizio, con margine di montaggio rispetto allo stato reale del progetto (vedi registro pubblicazione più sotto)

## Struttura di ogni episodio

1. Hook nei primi 2-3 secondi (mostra il problema o il risultato, non l'inizio)
2. Il tentativo
3. Il problema che emerge
4. La soluzione, il momento "funziona"
5. Chiusura con teaser dell'episodio successivo

## Registro pubblicazione

Il registro non è più un piano di idee scritto in anticipo: ogni episodio dal n. 3 in poi è ancorato a una
voce reale dello **storico sessioni** in `documento-sessione-cnc-2d.md` (colonna "Sessione reale") — cioè a
lavoro che è *già stato fatto e documentato*, non a una sceneggiatura immaginata prima. Quando emerge lavoro
reale nuovo (una nuova voce di storico), si aggiungono nuovi episodi qui; non si scrivono episodi per lavoro
non ancora avvenuto.

| Ep. | Titolo | Step reale | Sessione reale | Codice/file di riferimento | Stato riprese | Stato montaggio | Pubblicato |
|-----|--------|-----------|-----------------|------------------------------|----------------|------------------|------------|
| 0 | L'idea | — | — (introduttivo) | — | da girare | da fare | no |
| 1 | Il primo LED | Step 1 | [2026-09-19 10:03](../documento-sessione-cnc-2d.md#2026-09-19-1003-bring-up-hardware-esp32-alimentazione-usb-e-batteria-completati) | non recuperabile, [vedi nota](../firmware/archivio-storico/README.md#nota-sul-primissimo-test-ep-1--il-primo-led) | da girare | da fare | no |
| 2 | La batteria | Step 1 | [2026-09-19 10:03](../documento-sessione-cnc-2d.md#2026-09-19-1003-bring-up-hardware-esp32-alimentazione-usb-e-batteria-completati) | — (nessuna modifica firmware) | da girare | da fare | no |
| 3 | La dashboard nel browser | Step 1.1 | [2026-09-19 09:25](../documento-sessione-cnc-2d.md#2026-09-19-0925-dashboard-web-esp32-con-controllo-led-e-configurazione-wi-fi) | [`01-dashboard-led-singolo.ino`](../firmware/archivio-storico/01-dashboard-led-singolo.ino) | da girare | da fare | no |
| 4 | OTA e i log | Step 1.1 | [2026-09-20 09:45](../documento-sessione-cnc-2d.md#2026-09-20-0945-dashboard-estesa-led-ota-log-e-avvio-debug-driver-a4988motore-x) | [`03-ota.ino`](../firmware/archivio-storico/03-ota.ino), [`04-tab-log.ino`](../firmware/archivio-storico/04-tab-log.ino) | da girare | da fare | no |
| 5 | Le prime frecce | Step 1.1 | [2026-09-20 09:45](../documento-sessione-cnc-2d.md#2026-09-20-0945-dashboard-estesa-led-ota-log-e-avvio-debug-driver-a4988motore-x) | [`06-frecce-motore-x.ino`](../firmware/archivio-storico/06-frecce-motore-x.ino) | da girare | da fare | no |
| 6 | Cablaggio motore X e il primo driver morto | Step 2 | [2026-09-20 09:45](../documento-sessione-cnc-2d.md#2026-09-20-0945-dashboard-estesa-led-ota-log-e-avvio-debug-driver-a4988motore-x) | [`06-frecce-motore-x.ino`](../firmware/archivio-storico/06-frecce-motore-x.ino) | da girare | da fare | no |
| 7 | La caccia al Vref | Step 2 | [2026-09-20 09:45](../documento-sessione-cnc-2d.md#2026-09-20-0945-dashboard-estesa-led-ota-log-e-avvio-debug-driver-a4988motore-x) | [`07-step-lento-diagnosi.ino`](../firmware/archivio-storico/07-step-lento-diagnosi.ino) | da girare | da fare | no |
| 8 | Il mistero del wiggle | Step 2 | [2026-09-20 09:45](../documento-sessione-cnc-2d.md#2026-09-20-0945-dashboard-estesa-led-ota-log-e-avvio-debug-driver-a4988motore-x) | [`08-dir-enable-spostati.ino`](../firmware/archivio-storico/08-dir-enable-spostati.ino) | da scrivere | da fare | no |
| 9 | La causa vera: l'alimentatore RGB | Step 2 | [2026-09-20 18:18](../documento-sessione-cnc-2d.md#2026-09-20-1818-motore-x-funzionante-la-causa-era-lalimentatore-non-il-cablaggio) | [`10-test-pin-multimetro.ino`](../firmware/archivio-storico/10-test-pin-multimetro.ino) | da scrivere | da fare | no |
| 10 | Motore X vivo, poi il Y | Step 2 | [2026-09-20 18:18](../documento-sessione-cnc-2d.md#2026-09-20-1818-motore-x-funzionante-la-causa-era-lalimentatore-non-il-cablaggio) + [2026-09-21 09:15](../documento-sessione-cnc-2d.md#2026-09-21-0915-secondo-asse-pen-lift-e-scelta-di-fluidnc-come-firmware-definitivo) | [`11-asse-y.ino`](../firmware/archivio-storico/11-asse-y.ino) | da scrivere | da fare | no |
| 11 | Perché arrendersi a un firmware pronto | Step 3 | [2026-09-21 09:15](../documento-sessione-cnc-2d.md#2026-09-21-0915-secondo-asse-pen-lift-e-scelta-di-fluidnc-come-firmware-definitivo) | [`cnc2d-config.yaml`](../firmware/fluidnc/cnc2d-config.yaml) | da scrivere | da fare | no |
| 12 | Installazione alla cieca | Step 3 | [2026-09-21 09:57](../documento-sessione-cnc-2d.md#2026-09-21-0957-progetto-della-struttura-meccanica-e-installazione-di-fluidnc) | [`cnc2d-config.yaml`](../firmware/fluidnc/cnc2d-config.yaml) | da scrivere | da fare | no |
| 13 | La penna come asse | Step 3 | [2026-09-21 09:15](../documento-sessione-cnc-2d.md#2026-09-21-0915-secondo-asse-pen-lift-e-scelta-di-fluidnc-come-firmware-definitivo) | [`cnc2d-config.yaml`](../firmware/fluidnc/cnc2d-config.yaml) | da scrivere | da fare | no |
| 14 | Il primo quadrato | Step 3 | [2026-09-21 11:18](../documento-sessione-cnc-2d.md#2026-09-21-1118-fluidnc-installato-configurato-e-validato-la-macchina-si-muove-sotto-g-code) | [`gcode/test/`](../gcode/test/) | da scrivere | da fare | no |
| 15 | Il calcolo resta nel browser | Step 4 | [2026-09-24 18:44](../documento-sessione-cnc-2d.md#2026-09-24-1844-dallsvg-al-g-code-nellinterfaccia-e-progetto-meccanico-con-modulo-15) | [`webui/`](../webui/) | da scrivere | da fare | no |
| 16 | Dall'SVG al foglio | Step 4 | [2026-09-24 18:44](../documento-sessione-cnc-2d.md#2026-09-24-1844-dallsvg-al-g-code-nellinterfaccia-e-progetto-meccanico-con-modulo-15) | [`webui/js/svgimport.js`](../webui/js/svgimport.js), [`webui/js/paper.js`](../webui/js/paper.js) | da scrivere | da fare | no |
| 17 | Il G-code ottimizzato | Step 4 | [2026-09-24 18:44](../documento-sessione-cnc-2d.md#2026-09-24-1844-dallsvg-al-g-code-nellinterfaccia-e-progetto-meccanico-con-modulo-15) | [`webui/js/optimize.js`](../webui/js/optimize.js), [`webui/js/gcode.js`](../webui/js/gcode.js) | da scrivere | da fare | no |
| 18 | Il pignone e il modulo giusto | Step 5 | [2026-09-24 18:44](../documento-sessione-cnc-2d.md#2026-09-24-1844-dallsvg-al-g-code-nellinterfaccia-e-progetto-meccanico-con-modulo-15) | — (progettazione, primo pignone stampato modulo 1,5) | da scrivere | da fare | no |
| 19 | Tutto si stampa, anche gli ingranaggi | Step 5 | [2026-09-24 18:44](../documento-sessione-cnc-2d.md#2026-09-24-1844-dallsvg-al-g-code-nellinterfaccia-e-progetto-meccanico-con-modulo-15) | — (decisione: cremagliera stampata, non comprata in POM) | da scrivere | da fare | no |
| 20 | Il cuscinetto giusto | Step 5 | [2026-09-25 12:17](../documento-sessione-cnc-2d.md#2026-09-25-1217-materiali-acquistati-il-progetto-si-adatta-a-quello-che-esiste-in-ferramenta) | — (calcolo pressione di contatto, materiali acquistati) | da scrivere | da fare | no |
| 21 | Quattro errori prima di stampare | Step 5 | [2026-09-26 17:48](../documento-sessione-cnc-2d.md#2026-09-26-1748-carrello-validato-in-stampa-cremagliera-verificata-e-bilancio-di-coppia-del-motore) | — (interasse fori motore, perni fuori quota, cremagliera asimmetrica, dato di coppia sbagliato) | da scrivere | da fare | no |
| 22 | Il bilancio di coppia | Step 5 | [2026-09-26 17:48](../documento-sessione-cnc-2d.md#2026-09-26-1748-carrello-validato-in-stampa-cremagliera-verificata-e-bilancio-di-coppia-del-motore) | — (motore pancake 130 mN·m, angolo di distacco target <7°) | da scrivere | da fare | no |
| 23 | Il falso allarme dello strisciamento | Step 5 | [2026-09-27 17:32](../documento-sessione-cnc-2d.md#2026-09-27-1732-meccanica-carrello-completo-misurato-falso-allarme-sullo-strisciamento-cremagliera-chiusa-e-apertura-dellasse-y) | — (attrito interno dei cuscinetti scambiato per strisciamento) | da scrivere | da fare | no |
| 24 | Il formato cresce | Step 5 | [2026-09-27 17:32](../documento-sessione-cnc-2d.md#2026-09-27-1732-meccanica-carrello-completo-misurato-falso-allarme-sullo-strisciamento-cremagliera-chiusa-e-apertura-dellasse-y) | — (da A5 ad A4 orizzontale) | da scrivere | da fare | no |
| 25 | Si apre l'asse Y | Step 5 | [2026-09-27 17:32](../documento-sessione-cnc-2d.md#2026-09-27-1732-meccanica-carrello-completo-misurato-falso-allarme-sullo-strisciamento-cremagliera-chiusa-e-apertura-dellasse-y) | — (quattro sketch, profilo a T, principio di simmetria) | da scrivere | da fare | no |
| 26 | Taratura e primo disegno vero | Step 5 | *in attesa — non ancora accaduto* | [`14-servo-pen-lift.ino`](../firmware/archivio-storico/14-servo-pen-lift.ino) | non pianificabile ancora | — | no |

Nota: i link ai file puntano al repository (relativi a questa cartella `content/`) — cliccabili direttamente su
GitHub. I link "Sessione reale" puntano alla voce corrispondente nello storico di `documento-sessione-cnc-2d.md`.
L'episodio 26 resta un segnaposto: non riceve uno script finché la taratura reale non è stata fatta e
documentata nello storico sessioni.

---

## Ep. 0 — L'idea

**Step reale corrispondente:** nessuno (episodio introduttivo, girato prima di iniziare la finzione temporale)

**Girato:** B-roll dei componenti ancora nella scatola (ESP32, motori NEMA17, driver A4988), eventuale sketch/disegno a mano del plotter che disegna, primo piano dell'utente che parla o solo voce fuori campo

**Voice over (bozza):**

> "Voglio costruire una macchina che disegna da sola. Non un kit, non un plotter pronto — una CNC 2D fatta da zero: motori, elettronica, firmware, struttura, tutto.
> L'idea è semplice da dire e complicata da fare: due assi motorizzati che muovono una penna, una terza per alzarla e abbassarla, e un cervello — un ESP32 — che riceve i comandi via Wi-Fi, senza bisogno di un cavo USB collegato mentre disegna.
> Non ho ancora idea di quanti pezzi brucerò prima di arrivarci. Ma da qui parte tutto — un componente alla volta, un errore alla volta.
> Episodio 1: il primo pezzo che si accende."

**Testo overlay suggerito:** "Costruire una CNC 2D da zero — Episodio 0"

**Musica:** tono più lento/cinematografico rispetto agli episodi tecnici, per marcare che è il "pilot"

---

## Ep. 1 — Il primo LED

**Step reale corrispondente:** Step 1 (bring-up ESP32, test Blink su D2)

**Codice di riferimento:** non recuperabile da git, [vedi nota nell'archivio storico](../firmware/archivio-storico/README.md#nota-sul-primissimo-test-ep-1--il-primo-led) — sketch Blink da ricreare (pinMode + digitalWrite in loop)

**Girato:** ESP32 su breadboard, LED con resistenza 220 ohm, collegamento USB-C, caricamento sketch, primo lampeggio

**Voice over (bozza):**

> "Prima di muovere qualsiasi motore, devo essere sicuro che il cervello della macchina funzioni. Un ESP32: Wi-Fi integrato, abbastanza potenza per gestire più cose insieme.
> Il primo test è il più semplice che esista: un LED che lampeggia. Non serve a niente sulla macchina finita, ma se questo non funziona, non funziona nient'altro.
> [collegamento] Resistenza da 220 ohm per non friggere il LED, pin D2, sketch minimo.
> [upload/attesa]
> Eccolo. Funziona da USB. Il prossimo passo: farlo funzionare anche senza cavo, a batteria."

**Testo overlay suggerito:** "Step 1 — Il cervello si accende"

---

## Ep. 2 — La batteria

**Step reale corrispondente:** Step 1 (batteria 18650 + boost/carica 5V + interruttore su VIN)

**Girato:** modulo batteria 18650, collegamento boost 5V, interruttore SPST, ESP32 che si accende senza USB

**Voice over (bozza):**

> "Una macchina che deve stare in giro per la stanza senza un cavo attaccato al PC ha bisogno di una batteria. Non una qualsiasi: una che si possa ricaricare mentre è ancora in uso.
> Questo modulo fa da boost — trasforma la tensione della cella 18650 in 5V stabili — e permette di scaricare mentre carica. Utile: se l'interruttore fosse sulla batteria, spegnerla vorrebbe dire anche interrompere la ricarica.
> Quindi l'interruttore va solo sulla linea verso l'ESP32, non sulla batteria.
> [test] Stacco lo USB. Il LED lampeggia lo stesso. La macchina non dipende più da un cavo per stare accesa.
> Prossimo passo: dare a questo cervello un modo per parlare con me senza fili."

**Testo overlay suggerito:** "Step 1 — Vita senza cavo"

---

## Ep. 3 — La dashboard nel browser

**Step reale corrispondente:** Step 1.1 (dashboard web sull'ESP32, WebServer.h, mDNS, tab Wi-Fi)

**Codice di riferimento:** [`01-dashboard-led-singolo.ino`](../firmware/archivio-storico/01-dashboard-led-singolo.ino)

**Girato:** schermo del browser che raggiunge la dashboard, provisioning Wi-Fi dalla tab dedicata, pulsante che accende/spegne il LED, eventuale ripresa di `cnc2d.local` digitato nella barra degli indirizzi

**Voice over (bozza):**

> "Un ESP32 che lampeggia da solo non serve a niente: deve poter ricevere comandi. La soluzione più comoda è una pagina web ospitata direttamente sulla scheda — niente app, niente programmi da installare, solo un browser.
> Ho usato la libreria web server già inclusa nell'ESP32, per non dipendere da librerie esterne. E ho aggiunto un nome invece di un IP da ricordare: cnc2d.local.
> Le credenziali Wi-Fi si salvano da qui, dalla dashboard stessa — e se la rete di casa non si trova, la scheda apre un proprio punto d'accesso di emergenza, così la pagina resta sempre raggiungibile per correggerle.
> [click sul pulsante centrale] Il LED si accende dal browser. Da telefono, da PC, da chiunque sia sulla stessa rete.
> Le frecce che vedete già in pagina non fanno ancora nulla — sono lì in previsione dei motori. Prossimo episodio: farle funzionare per davvero."

**Testo overlay suggerito:** "Step 1 — Una pagina, nessun cavo"

---

## Ep. 4 — OTA e i log

**Step reale corrispondente:** Step 1.1 (ArduinoOTA, tab Log come sostituto del Serial Monitor)

**Codice di riferimento:** [`03-ota.ino`](../firmware/archivio-storico/03-ota.ino), [`04-tab-log.ino`](../firmware/archivio-storico/04-tab-log.ino)

**Girato:** upload via USB dell'ultima volta, poi ESP32 scollegato dal PC; una modifica al codice caricata da browser via OTA; tab "Log" della dashboard che scorre messaggi in tempo reale

**Voice over (bozza):**

> "Ogni volta che cambiavo anche una riga di codice dovevo staccare la macchina, portarla al PC, ricollegare il cavo USB. Scomodo, e destinato a diventare impossibile quando la macchina sarà montata e lontana dalla scrivania.
> La soluzione si chiama OTA — aggiornamento del firmware via Wi-Fi. Il primo caricamento resta via cavo, ma da lì in poi basta il browser.
> C'è un prezzo da pagare, però: senza cavo USB collegato, sparisce anche il Serial Monitor — il modo normale per vedere cosa succede dentro alla scheda mentre gira.
> [tab Log] Quindi ho aggiunto una tab nella dashboard che mostra in tempo reale gli stessi messaggi che prima vedevo solo da cavo.
> Ora la macchina è davvero autonoma: nessun cavo per aggiornarla, nessun cavo per capire cosa sta pensando. Prossimo passo: farla muovere sul serio."

**Testo overlay suggerito:** "Step 1 — Nessun cavo, neanche per guardarci dentro"

---

## Ep. 5 — Le prime frecce

**Step reale corrispondente:** Step 1.1 (motore X collegato alle frecce, coda di passi non bloccante)

**Codice di riferimento:** [`06-frecce-motore-x.ino`](../firmware/archivio-storico/06-frecce-motore-x.ino)

**Girato:** driver A4988 e motore X cablati sulla breadboard, click sulla freccia nella dashboard, motore che scatta di un passo; pressione prolungata sulla freccia con reazione incerta del motore

**Voice over (bozza):**

> "Le frecce erano già nella dashboard da un po' — decorazione, in attesa dei motori. Oggi le collego davvero: un driver A4988, un motore NEMA17, e un pin STEP che dice al motore quando muoversi.
> [click singolo] Un click, un passo pulito. Funziona.
> Ma il primo tentativo di farlo girare in modo continuo dentro la richiesta web blocca tutto il server per secondi — l'OTA, il log, tutto affamato mentre il motore gira.
> La soluzione è spostare il movimento fuori dalla richiesta: una coda di passi che viene svuotata nel ciclo principale, non dentro l'handler HTTP.
> [pressione prolungata] Click singolo perfetto. Pressione prolungata... qualcosa non torna ancora. E lì iniziano i problemi veri."

**Testo overlay suggerito:** "Step 1 — Le frecce prendono vita"

---

## Ep. 6 — Cablaggio motore X e il primo driver morto

**Step reale corrispondente:** Step 2 (primo A4988 non muove il motore)

**Codice di riferimento:** [`06-frecce-motore-x.ino`](../firmware/archivio-storico/06-frecce-motore-x.ino)

**Girato:** cablaggio completo driver+motore ripreso da vicino, pulsante "Giro completo" premuto, motore fermo con solo un leggero ronzio, albero bloccato al tatto; sostituzione del modulo A4988

**Voice over (bozza):**

> "Cablaggio finito, motore collegato, driver alimentato. Provo 'Giro completo' dalla dashboard.
> [pausa, ronzio] Niente. Solo un leggero ronzio, e l'albero bloccato con forza al tatto — che in teoria è normale per uno stepper abilitato, ma da solo non mi dice nulla.
> Controllo tutto: continuità su STEP, DIR, ENABLE, VDD, RESET, VMOT, GND. Tutto corretto sulla carta.
> L'unico sospetto che resta è il chip stesso — danneggiato, probabilmente durante le prove di cablaggio sotto tensione.
> [sostituzione modulo] Monto il secondo driver di scorta.
> [test] Si muove. Parzialmente. E quella parola, 'parzialmente', è esattamente il problema che mi porto nel prossimo episodio."

**Testo overlay suggerito:** "Step 2 — Il primo driver non ce l'ha fatta"

---

## Ep. 7 — La caccia al Vref

**Step reale corrispondente:** Step 2 (Vref bloccato a 0,20V, misura instabile col motore collegato)

**Codice di riferimento:** [`07-step-lento-diagnosi.ino`](../firmware/archivio-storico/07-step-lento-diagnosi.ino)

**Girato:** multimetro sul trimmer del driver, tentativo di regolazione, valore fermo a 0,20V; motore scollegato, nuova misura, taratura riuscita a 0,55V

**Voice over (bozza):**

> "Il driver ha un piccolo trimmer che regola quanta corrente manda al motore — il Vref. Per il mio motore il valore giusto è intorno a 0,55V.
> [misura] Il multimetro dice 0,20V. Giro il trimmer. Ancora 0,20V. Come se non facesse nulla.
> Il sospetto naturale è che il potenziometro sia rotto. Ma prima di sostituire un altro pezzo, provo una cosa: stacco il motore e misuro di nuovo, a vuoto.
> [misura a vuoto] 0,55V, esattamente il target. Il problema non era il trimmer — era il motore collegato che introduceva rumore sulla misura stessa.
> Vref tarato, motore ricollegato. Ma il movimento resta a scatti — Vref giusto non basta ancora."

**Testo overlay suggerito:** "Step 2 — Il trimmer 'rotto' che non lo era"

---

## Note di produzione generali

- Mantenere sempre la stessa intro/sigla breve (2-3 secondi) per rendere riconoscibile la serie
- Le riprese degli step già completati (1-5) vanno ricostruite ora con il materiale/hardware ancora disponibile: non serve che il "fallimento" avvenga davvero in quel momento, la voce e il montaggio raccontano cosa è successo
- Aggiornare questo file ad ogni episodio scritto/girato/montato, spuntando la colonna corrispondente nel registro
