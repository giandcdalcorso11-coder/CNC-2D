; CNC-2D - quadrato di prova, lato 40 mm
;
; Da eseguire a struttura non montata, con i motori liberi sul banco.
; Con steps_per_mm = 80 e microstep 1/16, 40 mm sono esattamente 3200
; microscatti, cioe' un giro completo dell'albero: ogni lato del quadrato
; e' un giro esatto di uno dei due motori.
;
; Non contiene comandi per la penna: il servo non e' ancora tarato, e
; mandarlo a una posizione non verificata rischia di portarlo in battuta
; meccanica. I comandi Z si aggiungono dopo la taratura.

G21             ; unita' in millimetri
G90             ; coordinate assolute
G92 X0 Y0       ; l'origine e' dove si trova la macchina adesso

G1 X40 Y0  F500 ; lato inferiore  - gira solo X
G1 X40 Y40      ; lato destro     - gira solo Y
G1 X0  Y40      ; lato superiore  - gira solo X, al contrario
G1 X0  Y0       ; lato sinistro   - gira solo Y, al contrario
