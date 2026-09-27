# Archivio storico — cnc2d_dashboard

Snapshot del firmware `cnc2d_dashboard.ino` estratti dalla cronologia git, uno per ogni tappa
significativa. Utili per rigirare i test degli episodi passati con il codice esattamente
come era in quel momento, invece che con la versione finale.

Il file più recente `firmware/cnc2d_dashboard/cnc2d_dashboard.ino` resta quello aggiornato/attivo;
questi sono solo copie storiche di sola lettura.

| File | Data | Commit | Cosa fa in quella versione |
|------|------|--------|------------------------------|
| `01-dashboard-led-singolo.ino` | 2026-09-19 | `6e0329d` | Dashboard base, un solo LED controllato dal browser + tab Wi-Fi |
| `02-tre-led-tre-pulsanti.ino` | 2026-09-19 | `5ae1c0d` | Tre pulsanti dedicati Rosso/Giallo/Verde per i LED D2/D19/D18, joystick centrale disabilitato |
| `03-ota.ino` | 2026-09-19 | `604365a` | Aggiunto ArduinoOTA per aggiornare il firmware via Wi-Fi |
| `04-tab-log.ino` | 2026-09-19 | `bea62c6` | Aggiunta tab Log come sostituto del Serial Monitor durante OTA |
| `05-log-eventi-led.ino` | 2026-09-19 | `6a3d769` | I toggle dei LED vengono registrati nel log web |
| `06-frecce-motore-x.ino` | 2026-09-19 | `70a2bde` | Prime frecce collegate al motore X (STEP/DIR/ENABLE) + giro completo |
| `07-step-lento-diagnosi.ino` | 2026-09-19 | `28e6b94` | Timing dei passi rallentato per diagnosticare lo stallo all'avvio |
| `08-dir-enable-spostati.ino` | 2026-09-20 | `9dfe36f` | DIR/ENABLE spostati da D16/D17 per escludere problemi pin-specifici |
| `09-step-non-bloccante-diagnostica.ino` | 2026-09-20 | `f60d42b` | Step motore non bloccante + tab diagnostica motore |
| `10-test-pin-multimetro.ino` | 2026-09-20 | `57c85c2` | Endpoint per forzare i pin a livello fisso, per test col multimetro |
| `11-asse-y.ino` | 2026-09-20 | `55c0ff3` | Aggiunto asse Y, secondo driver A4988 su D25/D33 con ENABLE condiviso |
| `12-velocita-rampa-per-asse.ino` | 2026-09-20 | `c1dc9b9` | Velocità e rampa impostabili per ogni asse con slider dedicati |
| `13-inversione-direzione-nvs.ino` | 2026-09-20 | `31f8d99` | Inversione direzione per asse, salvata in NVS |
| `14-servo-pen-lift.ino` | 2026-09-21 | `df3703b` | Servo pen-lift su D32 con due posizioni calibrate |

## Nota sul primissimo test (Ep. 1 — il primo LED)

Il test iniziale con un solo LED lampeggiante su D2 (sketch Blink, prima ancora della dashboard)
non è mai stato committato su questo repository — è stato uno sketch throwaway scritto ed eseguito
direttamente dall'IDE Arduino, senza salvarlo nel progetto. Per l'episodio 1 va quindi
ricreato da zero (è comunque il test più semplice: `pinMode` + `digitalWrite` in loop con
`delay`), non recuperato da qui.

## Versione attuale (FluidNC)

Dallo Step 3 in poi la macchina non usa più questo sketch come firmware definitivo — è stato
sostituito da FluidNC (`firmware/fluidnc/cnc2d-config.yaml`). Questo sketch resta solo per i
test hardware documentati sopra (Step 1 e Step 2) e per le riprese degli episodi corrispondenti.
