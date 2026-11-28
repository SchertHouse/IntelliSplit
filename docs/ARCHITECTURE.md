# Architettura

## Responsabilità dei componenti

### IntelliSplit
`IntelliSplit` è la classe iPlug2 centrale del plug-in. Possiede:
- registrazione dei parametri;
- disposizione dei controlli grafici;
- array dell'attività da 128 note;
- ingresso/uscita MIDI;
- stato della divisione dinamica;
- thread di aggiornamento della UI;
- vettori delle note attive;
- tempistica del reset.

La classe combina DSP, callback UI e stato del routing. Non espone un motore indipendente da iPlug2.

### IBarGraphControl
Il controllo riceve un riferimento a `keyboard` e ridisegna 128 barre più un marcatore della divisione. La sincronizzazione DSP/UI avviene indirettamente tramite iPlug2 e `SendControlValueFromDelegate`.

### SmallSet
`SmallSet` è una implementazione compatta basata su bitset e vettore. È usata da `noteOn` e `noteOff`. Usa un mutex e supporta la rimozione interna senza lock tramite `remove(..., false)`.

### ThreadSafeVector
`ThreadSafeVector` memorizza le assegnazioni attive alle mani e i marcatori delle duplicazioni (`left`, `right`, `leftD`, `rightD`). Ogni operazione è protetta da mutex.

### Utils
`Utils::ComputeSmoothingP` converte il tempo Smooth visibile in un coefficiente di decadimento per aggiornamento. `Utils::Smooth` applica un decadimento esponenziale lineare verso zero e quantizza i valori inferiori a 0.01.

## Flusso degli eventi
`Ingresso MIDI -> ProcessMidiMsg -> aggiornamento stato note -> Evoluzione tastiera -> ProcessingSplit -> assegnazione mano -> transposizione -> SendMidiMsg`

Flusso aggiuntivo:
- `ProcessBlock` richiama periodicamente `Evolve` per il decadimento.
- Una Note Off avvia la temporizzazione dell'inattività.
- ProcessBlock ripristina `splitMid` a `Init.Split` dopo l'assenza di note attive e trascorso il ritardo di reset.
- Gli eventi UI possono inviare All Notes Off e aggiornare lo stato condiviso dei parametri.

## Interazione DSP/UI/parametri
I parametri sono registrati nella base iPlug2 `Plugin` tramite `GetParam(...)->Init...`.
I controlli UI usano direttamente gli ID dei parametri. Le azioni dei pulsanti chiamano `SetNormalized` su `kButtonGroup`.
Il grafico non è un normale controllo parametro; è collegato con il tag `kBarGraphControl`.
`OnUIOpen` avvia un worker thread che aggiorna il grafico a `UPDATE_FREQ`; `OnUIClose` lo interrompe.

## Stato
Lo stato DSP include:
- `keyboard[128]`
- `splitMid`, `lSplit`, `rSplit`
- `noteOn`, `noteOff`
- `left`, `right`, `leftD`, `rightD`
- temporizzazione e stato attivo del reset
- mappatura dal canale di ingresso all'ultima nota

Lo stato UI include i valori visivi dei pulsanti e i vecchi valori dei canali usati per emettere All Notes Off.

## Decisioni architetturali
Decisioni confermate:
- iPlug2 possiede integrazione del plug-in e infrastruttura di parametri/preset.
- La divisione dinamica è implementata dentro `IntelliSplit`.
- Lo stato dell'assegnazione alle mani è mantenuto fuori dall'array delle attività.
- UI e routing MIDI condividono stato mutabile del plug-in.
- Le modifiche UI possono inviare direttamente MIDI all'host.
- I contenitori personalizzati sono locali al repository del plug-in.

## Critiche architetturali
- Il motore è strettamente accoppiato a iPlug2 e non può essere testato senza ambiente host/build iPlug2.
- Routing MIDI, logica della divisione, callback UI, threading e gestione parametri sono concentrati in `IntelliSplit.cpp`.
- Parte dello stato usa mutex mentre il resto dello stato DSP non ha un modello esplicito di sincronizzazione.
- Il worker thread della UI legge/modifica lo stato UI indipendentemente dalla proprietà del thread host.
- Le note duplicate sono rappresentate da vettori basati solo sul valore, perdendo identità ed ordine degli eventi.
- `fromUI` è un unico flag globale per All Notes Off proveniente dalla UI ed è vulnerabile ad ambiguità temporali.
