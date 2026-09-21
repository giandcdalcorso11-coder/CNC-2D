; CNC-2D - cerchio di prova, raggio 20 mm
;
; Serve a vedere l'interpolazione: in un arco i due motori girano
; insieme a velocita' che cambiano di continuo, ed e' esattamente la
; cosa che il firmware di collaudo non sapeva fare.
;
; La circonferenza e' 125,66 mm, cioe' poco piu' di tre giri d'albero
; distribuiti fra i due assi. A 500 mm/min il giro completo dura circa
; quindici secondi.

G21             ; unita' in millimetri
G90             ; coordinate assolute
G92 X0 Y0       ; l'origine e' dove si trova la macchina adesso

G0 X20 Y0       ; portati sul bordo del cerchio
G3 X20 Y0 I-20 J0 F500  ; cerchio completo antiorario attorno a X0 Y0
G0 X0 Y0        ; torna al centro
