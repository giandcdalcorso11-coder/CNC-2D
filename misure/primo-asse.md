# Primo asse — scheda di misura

Procedura per misurare quanto si muove davvero il carrello, con il solo
calibro. Da compilare al banco: le tabelle vanno riempite con le letture
grezze, non con le medie né con le conclusioni.

## I numeri di riferimento

| Grandezza | Valore |
|---|---|
| Passo del dente `p = pi * m` | 4,712389 mm |
| Un giro di pignone = 20 denti | 94,247780 mm |
| Un microscatto a 1/16 | 0,029452 mm |
| Passi/mm nominali | 33,95305 |
| Pulsante "1 giro" | 3200 microscatti esatti |

Il pulsante "1 giro" (tab **Controllo**) chiama `stepsPerRev()` e manda
3200 microscatti senza arrotondamenti. Il campo "Distanza (mm)" (tab
**Motore**) calcola `lroundf(mm * 33,953)`, quindi il numero digitato non
corrisponde mai esattamente agli impulsi emessi: **il numero vero si legge
nella riga di stato dell'asse**, che dopo il movimento scrive `N passi · 0
mancanti`.

## Impostazioni prima di cominciare

- Microstep: **1/16** nel menu Trasmissione.
- Velocità: **10 mm/s**. Accelerazione: **3 mm**. Lente, perché a questa
  prova interessa la posizione, non la prestazione.
- "Inverti direzione": sistemalo una volta sola, prima di iniziare, in modo
  che la freccia corrisponda al verso reale. Cambiarlo a metà prova
  invalida tutte le misure fatte prima.
- Driver abilitati: spuntato (tab Motore, in fondo).

## Il controllo da fare prima di tutto: il testimone sul pignone

Con un pennarello a punta fine traccia **una riga sola che attraversa il
giunto** tra il mozzo del pignone e l'albero del motore. Se a fine prova le
due metà della riga non sono più allineate, il pignone ha slittato
sull'albero e ogni misura della giornata va buttata.

È il guasto più frequente e il più ingannevole: produce un ammanco di corsa
che sembra in tutto un errore di taratura.

## Prova 0 — contare i denti (nessuno strumento, errore zero)

Un giro di pignone sono venti denti, per costruzione. Questa prova verifica
il rapporto di trasmissione **e** i microstep insieme, senza misurare niente.

1. Scegli una cava della cremagliera (lo spazio fra due denti: è più facile
   da centrare a occhio di un dente) e segna sul carrello un riferimento
   allineato a quella cava.
2. Premi **"1 giro"**.
3. Conta quante cave ha percorso il riferimento.

La dashboard manda 3200 impulsi perché il menu dice 1/16. Se i ponticelli
sul driver fossero su un altro valore, ogni impulso varrebbe più strada e il
carrello andrebbe oltre, non meno:

| Esito | Significato |
|---|---|
| 20 cave | rapporto e microstep confermati, vai avanti |
| 40 cave | il driver è a 1/8, non a 1/16 |
| 10 cave | il driver è a 1/32 |
| arriva in fondo alla rotaia | il driver è a passo intero: 16 giri, 1508 mm |
| 19 o 21 | ricontrolla il conteggio, poi i denti del pignone |

Per questo la Prova 0 va fatta **con il carrello all'inizio della corsa e la
mano sullo STOP**: se i ponticelli non sono dove credi, il carrello parte per
un metro e mezzo.

Risultato: ............ cave

## Prova 1 — il passo vero della cremagliera stampata

È la misura che decide i passi/mm: il rapporto di trasmissione è esatto per
costruzione (Prova 0), quindi l'unica incognita è di quanto il PLA si è
ritirato. Si misura sulla cremagliera, senza bisogno che il carrello si muova.

### Servono due perni identici

Due pezzi di tondino dello stesso diametro, appoggiati nelle cave: il perno si
centra da solo contro i due fianchi, e il calibro lo tocca di tangente, su una
superficie cilindrica ben definita. Le punte nude del calibro invece appoggiano
ogni volta a un'altezza diversa dentro la cava, e la lettura cambia a ogni
tentativo.

Vanno benissimo **due punte da trapano uguali**: il gambo è rettificato, e
soprattutto le due sono identiche fra loro, che è l'unica cosa che conta.

Diametro ideale per modulo 1,5: `1,68 * m = 2,52 mm`, quindi **2,5 mm** è la
misura giusta. Da 2 a 3 mm va comunque bene: il perno deve appoggiare sui
fianchi, non toccare il fondo della cava.

### Il metodo

Si misura **sopra i due perni**, da esterno a esterno, con i becchi esterni
del calibro. La lettura vale `(numero di passi fra le due cave) * p + d`, dove
`d` è il diametro del perno.

1. Perno fisso nella cava 1. Secondo perno nella cava 11. Leggi `R10`.
2. Lascia il primo perno dov'è. Sposta il secondo nella cava 31. Leggi `R30`.
3. `p = (R30 - R10) / 20`.

Il diametro del perno entra identico nelle due letture e **si cancella nella
sottrazione**: non serve nemmeno saperlo. Per lo stesso motivo non serve che i
due perni siano esattamente 2,50 mm, né uguali fra loro al centesimo — serve
solo che restino gli stessi due perni per tutta la serie.

Venti passi invece di dieci dimezzano l'errore: `p` esce con ±0,002 mm, cioè
±0,015 sui passi/mm. E 30 passi sono 141 mm più il perno, quindi ci stanno
ancora in un calibro da 150.

Letture attese con perni da 2,5 mm, se il ritiro fosse nullo: `R10 = 49,62`,
`R30 = 143,87`, differenza `20 * p = 94,248`. Se la differenza viene vicino a
89,5 o a 99 hai sbagliato il conteggio delle cave di una posizione: l'errore di
conteggio vale un passo intero, 4,7 mm, quindi si vede subito e non si nasconde
mai dentro il risultato.

### Tre serie lungo la cremagliera

| Serie | Cava di partenza | R10 (mm) | R30 (mm) | p = (R30-R10)/20 |
|---|---|---|---|---|
| A (inizio) | 1 | | | |
| B (metà) | | | | |
| C (fine) | | | | |

Tre valori di `p` che differiscono di più di 0,005 mm vogliono dire che la
stampa non è uniforme lungo la corsa: in quel caso l'errore si accumula invece
di restare costante, e tararlo con un numero solo non lo elimina.

Se la cremagliera è in più pezzi affiancati, fai una serie dentro ciascun pezzo
e **una serie a cavallo della giunzione**: dice se il giunto si mangia o
aggiunge una frazione di passo, che è l'errore peggiore perché è localizzato e
una taratura globale non lo vede.

Numero totale di cave della cremagliera: ............

### Attesa, e il limite vero di questa misura

`p` un po' **minore** di 4,712389: il PLA si ritira, e il ritiro va sempre
verso l'ingranamento più largo, mai verso il grippaggio. Passi/mm reali =
`160 / p`.

Oltre il secondo decimale sui passi/mm non vale la pena andare, e il motivo non
è il calibro: è il coefficiente di dilatazione del PLA, circa 70 µm per metro
per grado. Su una campata di 20 passi, dieci gradi di differenza valgono
0,066 mm, cioè 0,024 sui passi/mm — più dell'errore di misura. Quindi:

- misura la cremagliera alla temperatura a cui lavorerà, non appena stampata;
- su un disegno di 300 mm, dieci gradi di escursione valgono 0,2 mm di errore
  accumulato, e nessuna taratura lo toglie.

## Prova 2 — la corsa vera del carrello

Verifica l'insieme: slittamenti, flessioni, passi persi, pignone lasco.

Serve un riscontro: un blocchetto con una faccia piana, bloccato alla
rotaia con un morsetto. **Non serve che sia perfettamente in squadra**,
perché si misurano differenze di due letture e l'errore di squadra è lo
stesso in entrambe e si cancella. Serve solo che non si muova.

1. Blocca il riscontro **dietro** al carrello, lasciando un gioco di circa
   15 mm (sotto i 15 mm i becchi interni del calibro non ci stanno).
2. Misura la luce fra riscontro e carrello: `g0`.
3. Premi **"1 giro"** nel verso che allontana il carrello dal riscontro.
   Controlla che la riga di stato dica `3200 passi · 0 mancanti`.
4. Misura di nuovo la luce: `g1`. Corsa = `g1 - g0`.
5. Sposta il riscontro a 15 mm dietro la nuova posizione e ripeti, per tre
   tratte in fila: coprono circa 283 mm dei 308 di corsa.

Il carrello non va mai portato in battuta contro il riscontro, e non si
inverte mai il verso durante la prova: così il gioco d'ingranamento non
entra in questa misura, che si occupa solo della scala.

Con il calibro: tienilo parallelo alla rotaia e, bilanciandolo appena, prendi
la lettura **più piccola** che riesci a ottenere — è quella perpendicolare,
le altre sono tutte più lunghe del vero. I becchi interni spingono le due
facce allontanandole, quindi leggi al primo contatto, senza forzare: il
motore tiene il carrello con qualche decina di newton, ma non è un morsetto.

Se i becchi interni non arrivano, variante equivalente: misura con i becchi
esterni dalla faccia esterna del riscontro alla faccia del carrello. Lo
spessore del riscontro entra in tutte le letture allo stesso modo e si
cancella nella differenza.

| Tratta | g0 (mm) | g1 (mm) | corsa = g1 - g0 |
|---|---|---|---|
| 1 | | | |
| 2 | | | |
| 3 | | | |

Attesa: ogni tratta pari a `20 * p` della Prova 1. Tre tratte uguali fra
loro ma diverse dal nominale = taratura. Tre tratte diverse fra loro =
problema meccanico locale, e la tratta dice dove.

## Prova 3 — il gioco d'ingranamento

Il gioco non si accumula: è una zona morta che si paga **una volta a ogni
inversione**. Per vederlo si confrontano due tratte identiche, la prima
delle quali inverte il verso e la seconda no.

1. Arriva in posizione muovendoti **avanti**, e fermati.
2. Blocca il riscontro **davanti** al carrello, nel verso in cui stai per
   tornare, con una luce di circa 40 mm. Misura `g0`.
3. Nel campo "Distanza (mm)" scrivi `11,8` e premi la freccia **indietro**:
   è l'inversione. Misura `g1`.
4. Premi **la stessa freccia una seconda volta**, stesso verso, nessuna
   inversione. Misura `g2`.
5. Gioco = `(g1 - g2) - (g0 - g1)`.

La seconda tratta è la corsa piena, la prima è la corsa piena meno il
gioco. La differenza è il gioco, e non richiede di sapere quanti millimetri
sono stati comandati: basta che le due tratte siano lo stesso comando.

| Ripetizione | g0 (mm) | g1 (mm) | g2 (mm) | gioco |
|---|---|---|---|---|
| 1 | | | |
| 2 | | | |
| 3 | | | |

Attesa: circa 0,2–0,3 mm. Fallo in tre punti diversi della cremagliera: un
gioco che cambia lungo la corsa vuol dire che l'interasse fra pignone e
cremagliera non è costante, cioè che la cremagliera non è parallela alla
rotaia.

## Prova 4 — l'uniformità dello sforzo

1. **Togli** la spunta a "Driver abilitati": il motore lascia l'albero
   libero e il carrello si spinge a mano.
2. Spingi lentamente da un capo all'altro, due volte per verso.
3. Segna con una bandierina di nastro ogni punto duro e scrivi a quanti
   millimetri dal capo della rotaia si trova.

Un punto duro che si ripete nello stesso posto in entrambi i versi è
geometria: cremagliera non parallela alla rotaia, o rotaia inarcata dal
fissaggio. Un punto duro che si sposta è sporco o un truciolo.

4. Riabilita i driver e percorri la corsa intera a 10 mm/s, poi a 40 mm/s,
   ascoltando: sotto carico il tono del motore cambia, e un passo perso si
   sente come una raspata.

| Punto duro | mm dal capo | si ripete nei due versi? |
|---|---|---|
| | | |
| | | |

## Cosa serve per chiudere

Le letture grezze di tutte e quattro le prove, più:

- l'esito del testimone sul pignone, controllato a fine giornata;
- quanti passi riportava la riga di stato a ogni movimento, se mai diversi
  da quelli attesi.
