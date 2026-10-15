# Rinomina automatica dei voice over

Script che ascolta la cartella dove scarichi i voice over dal telefono, trascrive
ogni audio (in italiano) e crea una copia rinominata col testo di quello che dici
dentro — così non devi più ricordarti a memoria cosa c'è in "REC0412.m4a".

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

Metti i voice over grezzi in una cartella qualsiasi, ad esempio
`C:\Users\<te>\Desktop\voiceover-grezzi`, poi:

**Elaborazione una tantum** (processa solo i file nuovi da quell'ultima volta):
```
python rinomina_voiceover.py "C:\Users\<te>\Desktop\voiceover-grezzi"
```

**Modalità automatica** (resta in ascolto, utile se tieni la cartella
sincronizzata col telefono — es. Google Drive, cavo, AirDrop):
```
python rinomina_voiceover.py "C:\Users\<te>\Desktop\voiceover-grezzi" --watch
```
Lascia la finestra aperta mentre lavori: ogni nuovo file che compare nella
cartella viene trascritto e rinominato entro pochi secondi, senza che tu
debba rilanciare nulla. Ctrl+C per fermarlo.

## Cosa produce

Dentro `voiceover-grezzi\rinominati\` trovi, per ogni audio:
- la **copia rinominata** col testo (es. `oggi-montiamo-il-carrello-y.m4a`) —
  l'originale nella cartella di partenza non viene toccato né spostato
- un file `.txt` con la **trascrizione integrale**, comodo da incollare
  direttamente in `content/instagram-series.md` come bozza di voice over
- `_log-trascrizioni.csv` con lo storico di tutte le rinomine fatte

## Limiti da sapere

- La trascrizione automatica su audio registrato col microfono del telefono
  può avere qualche imprecisione (rumore di fondo, parole tecniche come nomi
  di pin o componenti) — il nome file e il `.txt` sono un punto di partenza
  da ricontrollare al volo, non un testo definitivo pronto per il montaggio.
- Il nome file usa solo le prime ~10 parole (i filename troppo lunghi danno
  problemi su Windows); il testo completo è sempre nel `.txt` accanto.
