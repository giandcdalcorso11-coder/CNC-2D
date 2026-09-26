# Costi materiali — CNC 2D Plotter

Elenco spese per la costruzione della macchina. Prezzi in euro, IVA/spedizione incluse dove indicato dallo scontrino/ordine.

## Legenda stato

- **confermato**: prezzo da scontrino/ordine
- **stima**: prezzo aggregato, ricordato a memoria, o di un articolo equivalente (materiale già posseduto)

## Legenda colonna "prezzo effettivo"

Molti articoli sono venduti in confezioni multiple o in kit con componenti misti, usati solo in parte in questa macchina (il resto resta di scorta o serve ad altro). La colonna **prezzo effettivo** stima quanto di quel prezzo è davvero attribuibile alla CNC 2D, in proporzione alla quantità usata sulla quantità acquistata. Sono stime ragionate, non misurate — da correggere se conosci i numeri reali.

## Elettronica

| Voce | Acquistato | Prezzo acquisto | Usato nel progetto (stima) | Prezzo effettivo (stima) | Stato | Note |
|------|-----------|------------------|------------------------------|---------------------------|-------|------|
| ESP32 NodeMCU (Diymore, USB-C, CH340) | 2 pezzi | 17,98 € | 1 pezzo | 8,99 € | confermato | il secondo resta di scorta |
| Breadboard Kit + jumper wire (AUKENIEN) | 4 breadboard + cavi | 15,99 € | ~1 breadboard + parte cavi, in fase di test | 6,40 € | confermato | stima 40%, il resto è scorta/eccedenza kit |
| Micro servo 9G (ARCELI) | 2 pezzi | 5,99 € | 1 pezzo | 3,00 € | confermato | pen-lift asse Z, il secondo è scorta |
| Modulo step-up Type-C 5V 2A (DollaTek) | 5 pezzi | 8,99 € | 1 pezzo | 1,80 € | confermato | boost/carica batteria, 4 di scorta |
| Motori NEMA17 17HS4023 (iMetrx) | 5 pezzi | 52,86 € | 2 pezzi (X, Y) | 21,14 € | confermato | 3 di scorta |
| Driver A4988 con dissipatore (ARCELI) | 5 pezzi | 8,99 € | 4 pezzi (2 bruciati durante i test + 2 installati X/Y) | 7,19 € | confermato | i bruciati sono comunque consumo reale del progetto |
| Interruttori a scorrimento SPDT (RUNCCI-YUN) | 30 pezzi | 9,48 € | 1 pezzo | 0,32 € | confermato | linea 5V+ verso VIN |
| Display LCD 1602 I2C (Freenove) | 2 pezzi | 12,95 € | 1 pezzo | 6,48 € | confermato | da integrare verso fine progetto, il secondo è scorta |
| Starter kit breadboard Miuzei (Kit A) | 1 kit misto | 21,99 € | quota parte (resistenze, pulsanti, cavi, condensatore usati) | 6,60 € | confermato | stima 30% del kit, il resto è materiale generico non impiegato |
| Batterie ricaricabili NiMH 3500 mAh (Generic) | 6 pezzi | 27,99 € | 3 pezzi (serie per il modulo step-up) | 14,00 € | stima | acquisto originale non ritrovato, prezzo di un articolo analogo; 3 di scorta |
| Alloggiamento batterie (portabatterie) | 1 | 2,50 € | 1 | 2,50 € | stima | ordine non trovato, prezzo stimato |
| Alimentatore switching 12V/3A (equivalente) | 1 | 15,19 € | 1 (intero) | 15,19 € | stima | già posseduto/recuperato, prezzo di un articolo equivalente |
| Modulo Nano-V3 + Nano I/O Shield (QIQIAZI) | 3 set | 18,99 € | 1 shield (Nano non usato, solo la shield come morsettiera) | 0,33 € | confermato | ipotesi: ~6 € a scheda Nano (non usata), il resto (0,99 €) diviso tra le 3 shield |
| **Totale elettronica** | — | **219,89 €** | — | **93,94 €** | — | — |

## Meccanica

| Voce | Acquistato | Prezzo acquisto | Usato nel progetto (stima) | Prezzo effettivo (stima) | Stato | Note |
|------|-----------|------------------|------------------------------|---------------------------|-------|------|
| Tavola in legno (compensato 385×455×10) | 1 | 3,00 € | 1 (intera) | 3,00 € | stima | base della macchina, ricordato a memoria |
| Viti M3 (conf. da 16, 10, 25 mm) | 3 confezioni | 6,00 € | ~90% (M3 è la vite unica di tutto il progetto) | 5,40 € | stima | split ipotizzato dentro il totale Brico |
| Barre di ferro (profilo a U + angolare forato) | 2 | 21,60 € | 2 (intere) | 21,60 € | stima | split ipotizzato dentro il totale Brico; profilo Y e angolare, entrambi impiegati |
| **Totale scontrino Brico** | — | **30,60 €** | — | **30,00 €** | confermato | somma delle tre voci sopra (split stimato) |
| Cuscinetti 608RS 8x22x7mm | 20 pezzi | 11,99 € | ~12 pezzi (stima) | 7,19 € | confermato | in uso sul carrello, quantità reale da confermare |
| Cuscinetti MR63ZZ 3x6x2,5mm | 10 pezzi | 9,95 € | ~6 pezzi (stima) | 5,97 € | confermato | in uso sul carrello, quantità reale da confermare |
| **Totale meccanica** | — | **52,54 €** | — | **43,16 €** | — | — |

## Materiale di stampa 3D

| Voce | Acquistato | Prezzo acquisto | Usato nel progetto (stima) | Prezzo effettivo (stima) | Stato | Note |
|------|-----------|------------------|------------------------------|---------------------------|-------|------|
| PLA+2.0 Sunlu nero (bobina) | 4 kg (4x1kg) | 52,99 € | ~150 g (in corso) | 1,99 € | confermato | quantità e costo finali a fine lavorazione |

## Altro

| Voce | Acquistato | Prezzo acquisto | Usato nel progetto (stima) | Prezzo effettivo (stima) | Stato | Note |
|------|-----------|------------------|------------------------------|---------------------------|-------|------|
| Mini calamite MEALOS (100 pz, misti) | 100 pz | 14,65 € | ~15 pezzi (stima) | 2,20 € | confermato | ancoraggio foglio, quantità reale da confermare |

---

## Totale spese

| Categoria | Prezzo acquisto | Prezzo effettivo (stima) |
|-----------|------------------|---------------------------|
| Elettronica | 219,89 € | 93,94 € |
| Meccanica (incl. Brico) | 52,54 € | 43,16 € |
| Materiale di stampa 3D | 52,99 € | 1,99 € |
| Altro | 14,65 € | 2,20 € |
| **Totale** | **340,07 €** | **141,29 €** |

Il **prezzo acquisto** è quanto è uscito realmente dal portafoglio (comprese scorte, componenti bruciati durante i test e materiale in eccesso nei kit). Il **prezzo effettivo** è la stima di quanto costa davvero *questa macchina*, al netto di scorte e materiale non utilizzato — utile se un giorno vuoi calcolare "quanto costerebbe rifarla comprando solo il necessario".

Note generali:
- Bordi neri unico tipo di cuscinetto in uso: 608RS e MR63ZZ, nessun altro tipo (nessuna voce 623ZZ nel progetto reale)
- Il totale Brico (30,60 €) resta un aggregato reale confermato; lo split tra legno/viti/barre è una stima interna, da correggere con lo scontrino
- L'alimentatore 12V/3A e il condensatore 100 µF/50V erano già disponibili in casa (recuperati, non riacquistati per il progetto): il condensatore rientra comunque nello starter kit Miuzei già conteggiato, mentre per l'alimentatore è stato stimato il prezzo di un articolo equivalente per completezza del totale
- La batteria NASTIMA 12V citata nelle verifiche hardware iniziali non è stata usata: è stata sostituita dall'alimentatore 12V/3A, quindi non compare come voce di costo
- Strumenti (multimetro etc.) esclusi volutamente dal conteggio
