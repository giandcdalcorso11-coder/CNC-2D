; CNC-2D - quadrato e cerchio, con i movimenti della penna
;
; Quote della penna PROVVISORIE: Z-3 alzata, Z-7 abbassata.
; Sono due valori al centro della corsa del servo, scelti perche' sicuri
; mentre gli estremi non sono ancora tarati. Se la penna si muove al
; contrario, basta scambiare i due numeri.
;
; Il G92 azzera solo X e Y: l'asse Z e' il servo, e azzerarlo sposterebbe
; il riferimento della penna su una posizione arbitraria.
;
; Le pause G4 danno al servo il tempo di arrivare e di smettere di
; oscillare prima che il carrello riparta. Senza, il primo tratto viene
; disegnato mentre la penna sta ancora scendendo.

G21                     ; millimetri
G90                     ; coordinate assolute
G92 X0 Y0               ; origine qui, solo sul piano

; ---------- quadrato da 40 mm ----------
G0 Z-3                  ; penna alzata
G0 X0 Y0 F2000
G1 Z-7 F600             ; penna abbassata
G4 P0.3
G1 X40 Y0  F1000        ; lato inferiore  - gira solo X
G1 X40 Y40              ; lato destro     - gira solo Y
G1 X0  Y40              ; lato superiore  - X al contrario
G1 X0  Y0               ; lato sinistro   - Y al contrario
G0 Z-3                  ; penna alzata
G4 P0.3

; ---------- cerchio di raggio 20 mm ----------
G0 X20 Y0 F2000         ; sul bordo del cerchio
G1 Z-7 F600
G4 P0.3
G3 X20 Y0 I-20 J0 F1000 ; cerchio completo antiorario attorno a X0 Y0
G0 Z-3
G4 P0.3

G0 X0 Y0 F2000          ; ritorno all'origine
M2                      ; fine programma
