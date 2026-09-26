# Costi materiali — CNC 2D Plotter

Elenco spese per la costruzione della macchina. Prezzi in euro, IVA/spedizione incluse dove indicato dallo scontrino/ordine.

## Legenda stato

- **confermato**: prezzo da scontrino/ordine
- **stima**: prezzo aggregato o ricordato a memoria, da confermare

## Elettronica

| Voce | Quantità | Prezzo | Stato | Note |
|------|----------|--------|-------|------|
| ESP32 NodeMCU (Diymore, USB-C, CH340) | 2 pezzi | 17,98 € | confermato | scheda di sviluppo, WiFi+BT |
| Breadboard Kit + jumper wire (AUKENIEN) | 4 breadboard (400+830) + cavi M/M e a U | 15,99 € | confermato | opzione "Breadboard+JW" |
| Micro servo 9G (ARCELI) | 2 pezzi | 5,99 € | confermato | per il pen-lift (asse Z) |
| Modulo step-up Type-C 5V 2A (DollaTek) | 5 pezzi | 8,99 € | confermato | boost/carica batteria 18650 |
| Motori NEMA17 17HS4023 (iMetrx) | 5 pezzi | 52,86 € | confermato | ne servono 2, 3 di scorta |
| Driver A4988 con dissipatore (ARCELI) | 5 pezzi | 8,99 € | confermato | coprono anche i moduli bruciati durante i test |
| Interruttori a scorrimento SPDT (RUNCCI-YUN) | 30 pezzi | 9,48 € | confermato | incluso quello sulla linea 5V+ verso VIN |
| Display LCD 1602 I2C (Freenove) | 2 pezzi | 12,95 € | confermato | da integrare verso fine progetto, solo 2 GPIO (SDA/SCL) |
| Starter kit breadboard Miuzei (Kit A) | 1 kit | 21,99 € | confermato | include condensatori, pulsanti, resistenze e cavetti già usati nel progetto — nessuna voce separata per questi |
| Batterie ricaricabili NiMH 3500 mAh (Generic) | 6 pezzi | 27,99 € | stima | acquisto originale non ritrovato, prezzo di un articolo analogo |
| Alloggiamento batterie (portabatterie) | 1 | ~2,50 € | stima | ordine non trovato, prezzo stimato — da confermare |
| Alimentatore switching 12V/3A (equivalente) | 1 | 15,19 € | stima | già posseduto/recuperato da materiale esistente, non riacquistato — prezzo di un articolo equivalente su Amazon.it |
| Modulo Nano-V3 + Nano I/O Shield (QIQIAZI) | 3 pezzi | 18,99 € | confermato | riadattata come morsettiera generica ("Nano Terminal Adapter") per i collegamenti di potenza |

## Meccanica

| Voce | Quantità | Prezzo | Stato | Note |
|------|----------|--------|-------|------|
| Tavola in legno (compensato 385×455×10) | 1 | 3,00 € | stima | acquisto Brico, ricordato a memoria |
| Viti M3 (conf. da 16, 10, 25 mm) | 3 confezioni | — | stima | incluso nel totale Brico, split da confermare |
| Barre di ferro (profilo a U + angolare forato) | 2 | — | stima | incluso nel totale Brico, split da confermare |
| **Totale scontrino Brico** | — | **30,60 €** | confermato | somma delle tre voci sopra |
| Cuscinetti 608RS 8x22x7mm | 20 pezzi | 11,99 € | confermato | scartati, troppo grandi — superati dalla scelta 623ZZ |
| Cuscinetti MR63ZZ 3x6x2,5mm | 10 pezzi | 9,95 € | confermato | alternativa provata, canale troppo stretto — superata dalla scelta 623ZZ |

## Materiale di stampa 3D

| Voce | Quantità | Prezzo | Stato | Note |
|------|----------|--------|-------|------|
| PLA+2.0 Sunlu nero (bobina) | 4 kg (4x1kg) | 52,99 € | confermato | acquisto della bobina; consumo effettivo ~150 g finora, in corso |

## Altro

| Voce | Quantità | Prezzo | Stato | Note |
|------|----------|--------|-------|------|
| Mini calamite MEALOS (100 pz, misti 3x1/4x2/5x2mm) | 100 pz | 14,65 € | confermato | ancoraggio del foglio sul piano (foglio magnetico o incassate nel legno) |

---

## Totale spese (parziale)

- Elettronica: **219,89 €** (di cui 45,68 € stimati: batterie, alloggiamento, alimentatore)
- Meccanica (incl. Brico): **52,54 €**
- Materiale di stampa 3D (bobina acquistata): **52,99 €**
- Altro: **14,65 €**

**Totale ad oggi: 340,07 €**

Note:
- Il totale Brico (30,60 €) è ancora un aggregato stimato (legno + viti + barre), da confermare con lo scontrino
- Il costo del PLA+ è quello della bobina intera da 4 kg; solo una piccola parte (~150 g) è stata effettivamente consumata finora
- I cuscinetti 608RS e MR63ZZ sono spese reali ma di tentativi scartati, non del progetto finale (623ZZ ancora da inserire in elenco quando arriva lo screenshot)
- L'alloggiamento batterie non ha ordine ritrovato: prezzo stimato, da correggere se recuperi lo scontrino
- L'alimentatore 12V/3A e il condensatore 100 µF/50V erano già disponibili in casa (recuperati, non riacquistati per il progetto): il condensatore rientra comunque nello starter kit Miuzei già conteggiato, mentre per l'alimentatore è stato stimato il prezzo di un articolo equivalente per completezza del totale
- La batteria NASTIMA 12V citata nelle verifiche hardware iniziali non è stata usata: è stata sostituita dall'alimentatore 12V/3A, quindi non compare come voce di costo
- Strumenti (multimetro etc.) esclusi volutamente dal conteggio
