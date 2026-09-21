# G-code di prova

Due file minimi per verificare il moto prima che esista la struttura, con i
motori appoggiati sul banco e un pezzetto di nastro sull'albero come indice.

| File | Cosa verifica |
|---|---|
| `quadrato-40mm.gcode` | movimento su un asse alla volta, e l'inversione di direzione |
| `cerchio-r20.gcode` | interpolazione: i due assi che si muovono insieme |

## Come eseguirli

Senza scheda SD, il modo più rapido è incollare le righe nel campo comandi
della WebUI una alla volta. I file si possono anche caricare nella memoria
interna dell'ESP32 dalla tab FluidNC e mandarli in esecuzione da lì.

Attenzione al campo comandi della WebUI: incollando con Ctrl+V alcune
versioni del browser aggiungono una `v` in fondo alla riga, e FluidNC
risponde `error:2 Bad GCode number format`. Controllare il campo prima di
inviare.

## Perché 40 mm

Con `steps_per_mm: 80` e microstep 1/16, 40 mm corrispondono a 3200
microscatti, cioè esattamente un giro dell'albero. Il numero è scelto per
questo: rende il test verificabile a occhio senza strumenti. Se un lato del
quadrato producesse sedici giri invece di uno, i ponticelli MS1/MS2/MS3 non
starebbero facendo effetto e il driver sarebbe rimasto a passo intero.

Il valore 80 resta comunque un segnaposto ereditato da un'ipotesi di
trasmissione a cinghia. Con cremagliera e pignone andrà ricalcolato e poi
tarato sulla macchina, e a quel punto i 40 mm non saranno più un giro esatto:
questi file restano validi come prova di movimento, non come riferimento
dimensionale.

## Niente comandi per la penna

I file non contengono movimenti sull'asse Z. Il servo non è ancora tarato, e
i valori `min_pulse_us` / `max_pulse_us` in configurazione sono di partenza:
mandare un SG90 a una posizione non verificata significa rischiare di
portarlo in battuta contro un fine corsa meccanico, dove assorbe molto e
forza gli ingranaggi. I comandi di penna si aggiungono dopo la taratura.
