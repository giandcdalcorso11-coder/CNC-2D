# GDC CNC 2D

Plotter a penna costruito da zero: struttura a portale in alluminio, trasmissione
a pignone e cremagliera stampata in PLA (modulo 1,5, 20 denti), due NEMA 17 con
driver A4988 e un ESP32 come controllore. Pen-lift a servo.

Lo stato del progetto, le decisioni prese e il perché di ognuna stanno in
[`documento-sessione-cnc-2d.md`](documento-sessione-cnc-2d.md). È il documento da
leggere per primo: la pipeline a step dice dove siamo, lo storico dice come ci
siamo arrivati.

## Le due applicazioni

Il progetto ne ha due, con ruoli diversi, e non si sostituiscono a vicenda.

### GDC CNC 2D SLICER

L'applicazione vera. Importa un disegno vettoriale, lo posiziona sul foglio,
ottimizza il percorso, genera il G-code e comanda la macchina. Tutta
l'elaborazione avviene nel browser; il microcontrollore esegue e basta.

- [`webui/`](webui/) — l'interfaccia. Nessun framework e nessun processo di
  build nel senso usuale: la pagina deve stare nella memoria dell'ESP32, e
  `tools/build.py` la impacchetta in un unico file
- [`firmware/fluidnc/`](firmware/fluidnc/) — la configurazione di FluidNC, cioè
  il lato macchina: assi, pin, cinematica, finecorsa

### Pannello di controllo manuale GDC CNC 2D

Lo strumento di collaudo e messa a punto: frecce per muovere gli assi un
microscatto alla volta, un giro di pignone, slider per il servo, prove
d'insieme, log e aggiornamento via Wi-Fi. Serve a montare e a diagnosticare, non
a disegnare.

- [`firmware/cnc2d_dashboard/`](firmware/cnc2d_dashboard/) — lo sketch Arduino.
  La cartella e il file conservano il nome tecnico `cnc2d_dashboard` perché
  l'IDE Arduino richiede che cartella e sketch si chiamino allo stesso modo e
  non accetta spazi nel nome
- [`firmware/archivio-storico/`](firmware/archivio-storico/) — una copia dello
  sketch per ogni tappa dello sviluppo, con il commit da cui viene

## Il resto

| Cartella | Contenuto |
|---|---|
| [`misure/`](misure/) | procedure di misura da banco e risultati |
| [`gcode/test/`](gcode/test/) | G-code minimi per verificare il moto |
| [`content/`](content/) | costi dei materiali e materiale divulgativo |

## Convenzione degli assi

**X** è la trave del portale, quella su cui corre il carrello che porta la
penna. **Y** sono le due rotaie di base, su cui scorre tutto il portale. È la
convenzione cartesiana vista dal foglio, adottata il 2026-10-15: documenti e
commit precedenti a quella data usano i due nomi scambiati.
