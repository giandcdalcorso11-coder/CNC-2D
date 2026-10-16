# Rinomina automatica dei voice over

Script che ascolta la cartella dove scarichi i voice over dal telefono, trascrive
ogni audio (in italiano) e **rinomina il file sul posto** col testo di quello
che dici dentro — così non devi più ricordarti a memoria cosa c'è in "REC0412.m4a".

Gira **sul tuo PC**, non in questa sessione cloud: la trascrizione è locale
(modello Whisper via `faster-whisper`), quindi non carichi niente online, è
gratis e funziona anche offline dopo il primo avvio.

## Setup (una volta sola)

1. Installa Python 3.10+ da [python.org](https://www.python.org/downloads/) —
   durante l'installazione spunta **"Add python.exe to PATH"**.
2. Apri il Prompt dei comandi (cmd) o PowerShell e lancia:
   ```
   pip install faster-whisper
   ```
   (la prima volta che lo usi, lo script scarica anche il modello di
   trascrizione — qualche centinaio di MB, una tantum, poi resta in cache)

## Uso

**Il modo più comodo: doppio click.** Metti `000-rinomina_voiceover.py` e
`000-rinomina voice over.bat` nella stessa cartella dove scarichi i voice
over (es. `0-voice over`), poi fai doppio click su
**`000-rinomina voice over.bat`**: si apre da sola una finestra, parte in
ascolto su quella cartella, e da quel momento ogni nuovo file audio che ci
metti dentro viene trascritto e rinominato entro pochi secondi, senza
aprire tu stesso nessun terminale. Lasciala aperta (anche ridotta a icona)
mentre lavori; chiuderla ferma l'ascolto.

Il prefisso `000-` serve solo a tenere questi file (script, log, storico)
sempre in cima alla cartella in ordine alfabetico, separati dai voice over
numerati `001-`, `002-`, ... — così restano facili da trovare e cliccare.

**In alternativa, da riga di comando** (stessa cartella dei voice over):
```
python 000-rinomina_voiceover.py .            rem elabora una volta i file nuovi
python 000-rinomina_voiceover.py . --watch    rem resta in ascolto come il .bat
```

## Ordine cronologico

Il nome del file testuale non segue l'ordine di registrazione, quindi lo
script antepone un **numero progressivo a 3 cifre** (es.
`006-oggi-montiamo-il-carrello-y.mp4`). Il numero successivo lo ricava dal
più alto già scritto in `000-elaborati.json` (lo storico di tutte le
rinomine fatte finora) — **continua a salire anche se sposti o archivi
altrove i file già rinominati** (es. in una sottocartella per un episodio
finito), perché non dipende da cosa resta fisicamente nella cartella.

**Non cancellare `000-elaborati.json`** tra una sessione e l'altra: è la
memoria dello script. Se lo cancelli, la volta dopo riparte scansionando
solo quello che trova nella cartella in quel momento (vedi sotto) e può
sbagliare la numerazione se nel frattempo hai spostato via dei file.

Se rinomini a mano i primi file (come nel caso di averne già 5) **prima
ancora che esista `000-elaborati.json`**, usa lo stesso schema — `NNN-`
davanti, il resto come preferisci — es.:
```
001-idea-del-progetto.mp4
002-primo-led.mp4
003-circuito-start.mp4
004-ota.mp4
005-dashboard-log.mp4
```
Alla prima run, lo script guarda sia lo storico sia la cartella, vede `005`
come numero più alto e il prossimo file nuovo diventa `006-...`; da lì in
poi lo storico in `000-elaborati.json` tiene il conto anche per i file che
sposti altrove.

## Cosa produce

I file audio vengono **rinominati sul posto** (es. `WhatsApp Audio
2026-10-15 at 22.50.54.mp4` → `006-oggi-montiamo-il-carrello-y.mp4`), non
copiati altrove. Per ognuno trovi anche:
- un file `.txt` con la **trascrizione integrale**, comodo da incollare
  direttamente in `content/instagram-series.md` come bozza di voice over
- `000-elaborati.json`, lo **storico delle rinomine** (nome originale →
  nuovo nome, data/ora), che serve anche allo script per non ritrascrivere
  due volte lo stesso file
- `000-log-trascrizioni.csv`, lo stesso storico in formato apribile con
  Excel, con in più il testo trascritto per ogni voce

## Organizzare i voice over finiti in sottocartelle

Lo script guarda solo il livello principale della cartella che gli indichi,
non entra nelle sottocartelle. Quindi puoi tranquillamente creare una
sottocartella tipo `episodio-1/` e spostarci dentro i voice over già
rinominati e sistemati: lo script li ignora da quel momento in poi (non li
ritocca) e, grazie allo storico in `000-elaborati.json`, continua a
numerare i nuovi file da dove era arrivato.

## Limiti da sapere

- La trascrizione automatica su audio registrato col microfono del telefono
  può avere qualche imprecisione (rumore di fondo, parole tecniche come nomi
  di pin o componenti) — il nome file e il `.txt` sono un punto di partenza
  da ricontrollare al volo, non un testo definitivo pronto per il montaggio.
- Il nome file usa solo le prime ~10 parole (i filename troppo lunghi danno
  problemi su Windows); il testo completo è sempre nel `.txt` accanto.
