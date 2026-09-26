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

| Ep. | Titolo | Step reale | Stato riprese | Stato montaggio | Pubblicato |
|-----|--------|-----------|----------------|------------------|------------|
| 0 | L'idea | — | da girare | da fare | no |
| 1 | Il primo LED | Step 1 | da girare | da fare | no |
| 2 | La batteria | Step 1 | da girare | da fare | no |
| 3 | La dashboard nel browser | Step 1.1 | da girare | da fare | no |
| 4 | OTA e i log | Step 1.1 | da scrivere | da fare | no |
| 5 | Le prime frecce | Step 1.1 | da scrivere | da fare | no |
| 6 | Cablaggio motore X e il primo driver morto | Step 2 | da scrivere | da fare | no |
| 7 | La caccia al Vref | Step 2 | da scrivere | da fare | no |
| 8 | Il mistero del wiggle | Step 2 | da scrivere | da fare | no |
| 9 | La causa vera: l'alimentatore RGB | Step 2 | da scrivere | da fare | no |
| 10 | Motore X vivo, poi il Y | Step 2 | da scrivere | da fare | no |
| 11 | Perché arrendersi a un firmware pronto | Step 3 | da scrivere | da fare | no |
| 12 | Installazione alla cieca | Step 3 | da scrivere | da fare | no |
| 13 | La penna come asse | Step 3 | da scrivere | da fare | no |
| 14 | Il primo quadrato | Step 3 | da scrivere | da fare | no |
| 15 | Il calcolo resta nel browser | Step 4 | da scrivere | da fare | no |
| 16 | Dall'SVG al foglio | Step 4 | da scrivere | da fare | no |
| 17 | Il G-code ottimizzato | Step 4 | da scrivere | da fare | no |
| 18 | Cremagliera o cinghia | Step 5 | da scrivere | da fare | no |
| 19 | Il cuscinetto giusto | Step 5 | da scrivere | da fare | no |
| 20 | Il primo pignone stampato | Step 5 | da scrivere | da fare | no |
| 21 | Il carrello e le due guide | Step 5 | da scrivere | da fare | no |
| 22 | Taratura e primo disegno vero | Step 5 | da scrivere | da fare | no |

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

**Girato:** schermo del browser che raggiunge la dashboard, provisioning Wi-Fi dalla tab dedicata, pulsante che accende/spegne il LED, eventuale ripresa di `cnc2d.local` digitato nella barra degli indirizzi

**Voice over (bozza):**

> "Un ESP32 che lampeggia da solo non serve a niente: deve poter ricevere comandi. La soluzione più comoda è una pagina web ospitata direttamente sulla scheda — niente app, niente programmi da installare, solo un browser.
> Ho usato la libreria web server già inclusa nell'ESP32, per non dipendere da librerie esterne. E ho aggiunto un nome invece di un IP da ricordare: cnc2d.local.
> Le credenziali Wi-Fi si salvano da qui, dalla dashboard stessa — e se la rete di casa non si trova, la scheda apre un proprio punto d'accesso di emergenza, così la pagina resta sempre raggiungibile per correggerle.
> [click sul pulsante centrale] Il LED si accende dal browser. Da telefono, da PC, da chiunque sia sulla stessa rete.
> Le frecce che vedete già in pagina non fanno ancora nulla — sono lì in previsione dei motori. Prossimo episodio: farle funzionare per davvero."

**Testo overlay suggerito:** "Step 1 — Una pagina, nessun cavo"

---

## Note di produzione generali

- Mantenere sempre la stessa intro/sigla breve (2-3 secondi) per rendere riconoscibile la serie
- Le riprese degli step già completati (1-5) vanno ricostruite ora con il materiale/hardware ancora disponibile: non serve che il "fallimento" avvenga davvero in quel momento, la voce e il montaggio raccontano cosa è successo
- Aggiornare questo file ad ogni episodio scritto/girato/montato, spuntando la colonna corrispondente nel registro
