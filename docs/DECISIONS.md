# Decisioni architetturali

## Decisioni esistenti

### Decisione: iPlug2 come framework del plug-in
**Stato:** Esistente.
**Motivazione:** il progetto usa le classi iPlug2 Plugin e iGraphics e dispone di progetti/workflow iPlug2.
**Implicazioni:** l'integrazione della piattaforma dipende dal framework e richiede codice iPlug2 esterno.

### Decisione: Divisione dinamica dentro IntelliSplit
**Stato:** Esistente.
**Motivazione:** `IntelliSplit.cpp` possiede l'array da 128 note e i calcoli della divisione.
**Implicazioni:** il motore non è verificabile indipendentemente.

### Decisione: Due canali in uscita configurabili
**Stato:** Esistente.
**Motivazione:** parametri e vettori sono definiti per sinistra e destra.
**Implicazioni:** la modalità Mix richiede l'invio e il tracciamento della stessa nota su entrambi i canali.

### Decisione: Nessun state chunk
**Stato:** Esistente. `PLUG_DOES_STATE_CHUNKS 0`.
**Implicazioni:** la persistenza dipende dallo stato/preset dei parametri iPlug2, salvo aggiunta di gestione personalizzata dello stato.

### Decisione: Array MIDI fisso da 128 note
**Stato:** Esistente.
**Implicazioni:** la memoria è limitata e l'indice della divisione coincide direttamente con il numero della nota MIDI.

### Decisione: Pianificazione UI/decadimento a 60 FPS
**Stato:** Esistente.
**Implicazioni:** il decadimento è guidato da `PLUG_FPS` e il reset usa il conteggio dei campioni. Il comportamento può variare con sample rate e pianificazione dei blocchi.

## Decisioni proposte

### Proposta: Estrarre il motore MIDI
**Stato:** Non approvata.
**Modifica:** nuove classi di divisione e routing indipendenti da iPlug2.

### Proposta: Preservare l'identità degli eventi delle note
**Stato:** Non approvata.
**Modifica:** usare conteggi delle duplicazioni, ID delle note o record specifici per canale anziché vettori basati solo sul valore.

### Proposta: Memorizzare il routing per ogni Note On attiva
**Stato:** Non approvata.
**Modifica:** la Note Off leggerebbe il canale e la transposizione memorizzati anziché i valori correnti dei parametri.

### Proposta: Validare le combinazioni dei parametri
**Stato:** Non approvata.
**Modifica:** definire il comportamento per Min > Max e per la discordanza dell'intervallo di transposizione della UI.
