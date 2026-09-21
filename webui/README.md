# Interfaccia web CNC-2D

L'interfaccia di controllo e preparazione del disegno. Non sostituisce la WebUI
di FluidNC: la affianca, occupandosi di tutto ciò che sta *prima* del G-code —
il foglio, il disegno, l'anteprima del percorso — e usando FluidNC per eseguirlo.

Stato attuale: **impalcatura con segnaposto**. La struttura, la navigazione,
il foglio e l'anteprima del percorso funzionano; il caricamento dell'SVG e la
generazione del G-code non sono ancora implementati. Le parti non costruite
sono segnalate nell'interfaccia stessa con una nota in corsivo, per non
scambiare un segnaposto per una funzione rotta.

## Vincoli che hanno determinato la forma del codice

**Niente framework, niente librerie, niente processo di build.** La pagina
dovrà essere servita dalla memoria dell'ESP32, dove lo spazio è di qualche
centinaio di kilobyte e non c'è internet garantito. Tutto ciò che serve è nella
cartella, e si apre com'è.

**Script classici, non moduli ES.** Un modulo ES caricato da `file://` viene
bloccato dal browser, quindi la pagina non si potrebbe nemmeno guardare senza
un server. Con gli script normali si apre `index.html` con un doppio clic.

**L'interfaccia dovrà essere servita dall'ESP32 per funzionare davvero.**
FluidNC espone il suo HTTP senza intestazioni CORS: una pagina caricata da un
altro dominio invia le richieste ma il browser le rifiuta. Finché non è
caricata sulla macchina, si lavora in modalità finta.

## Modalità finta

Attiva per impostazione predefinita. L'interfaccia simula le risposte della
macchina: nessuna richiesta esce dal browser, le coordinate si aggiornano, il
log si riempie. Serve a poter lavorare sull'interfaccia senza accendere nulla,
che è la maggior parte del tempo.

Si disattiva dalla tab Impostazioni.

## I file

| File | Contenuto |
|---|---|
| `index.html` | struttura della pagina |
| `css/base.css` | variabili di colore, reset, tipografia |
| `css/layout.css` | barra, tab, le tre colonne |
| `css/components.css` | schede, pulsanti, spie, foglio, cursore, log |
| `js/state.js` | stato centrale e notifiche fra moduli |
| `js/api.js` | comunicazione con FluidNC e modalità finta |
| `js/logview.js` | la tab Log |
| `js/paper.js` | foglio, formati, percorso sul foglio |
| `js/preview.js` | il cursore verticale che fa scorrere la penna |
| `js/jog.js` | movimento manuale, origine, penna |
| `js/settings.js` | la tab Impostazioni |
| `js/app.js` | avvio, tab, barra di stato |

## Due dettagli che è facile sbagliare

**L'asse Y è ribaltato.** In SVG la Y cresce verso il basso, sulla macchina
cresce verso l'alto. Il gruppo `#flip` applica `translate(0,H) scale(1,-1)`
così le coordinate dentro l'SVG sono quelle della macchina, in millimetri.
Senza quel ribaltamento ogni disegno uscirebbe specchiato, ed è un errore che
si scopre tardi e a foglio rovinato.

**Gli spessori di tratto sono in millimetri.** Dentro l'SVG l'unità è il
millimetro per via della viewBox: uno `stroke-width: 1` sarebbe un tratto da
un millimetro, non da un pixel, e cambierebbe aspetto a ogni formato di foglio.

## Cosa manca

- Lettura dell'SVG e conversione in percorso (`js/paper.js`)
- Generazione del G-code dal percorso
- Invio del programma alla macchina e avanzamento in tempo reale
- Calibrazione dell'area dai due angoli del foglio
- Comandi della penna, dopo la taratura del servo
- Caricamento della pagina nella memoria dell'ESP32
