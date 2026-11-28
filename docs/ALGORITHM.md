# Algoritmo

## Modello implementato
Ogni numero di nota MIDI da 0 a 127 possiede un valore di attività in virgola mobile in `keyboard[note]`.

Il comportamento visivo previsto è una divisione dinamica influenzata dall'attività recente delle note. L'implementazione non mantiene mani separate come stimatori persistenti di tipo Kalman. Calcola due posizioni candidate della divisione dall'array delle attività e le combina con la posizione della nota e lo stato attivo delle duplicazioni.

## Attività della tastiera
- Una Note On richiama `Evolve(note)`, che imposta `keyboard[note] = 1`.
- `Evolve()` viene eseguito da `ProcessBlock` circa una volta per aggiornamento `PLUG_FPS`.
- Ogni nota in `noteOff` ha `keyboard[note]` attenuato verso zero.
- Quando un valore di attività scende sotto 0.5, la nota viene rimossa da `leftD` e `rightD`.
- `Utils::Smooth` porta esattamente a zero i valori inferiori a 0.01.

Lo stesso valore della tastiera viene azzerato per le note in decadimento e non viene inizializzato da un decadimento separato per nota tenuta.

## Formula dello smoothing
`UPDATE_FREQ = 1000 / PLUG_FPS`, in millisecondi.

Per il parametro Smooth `T`:

`iterations = T / UPDATE_FREQ`

`p = 1 - threshold^(1 / iterations)`

Se `T = 0`, `iterations = 0` e la formula divide per zero. Il parametro è registrato con intervallo UI valido 0-10000 ms, quindi zero è raggiungibile. Il comportamento C++ effettivo dipende da `std::pow` e dalla divisione per zero a runtime.

Decadimento:

`keyboard[note] = keyboard[note] * (1 - p)`

La funzione threshold quantizza a zero i valori finali inferiori a 0.01.

## Candidati della divisione sinistra/destra
`computeSplit(keyboard, fromLeft)` scansiona:
- da sinistra a destra per `lSplit`;
- da destra a sinistra per `rSplit`.

Per ogni valore non nullo accumula `weightedSum += value * key` e `weightTotal += value`.

Se un valore è uguale a `MAXF == 1`, la scansione si ferma su quella nota e `borderKey` assume il valore della nota.

Senza una nota massima:

`result = borderEdge + direction * (handRange - |weightedMean - borderEdge|)`

dove `handRange = Range + 1` nella direzione sinistra oppure `Range` nella direzione destra.

Senza peso significativo:

`result = borderEdge + direction * handRange`

L'implementazione effettiva differisce tra sinistra e destra e va considerata un'euristica locale personalizzata, non una media ponderata convenzionale.

## ProcessingSplit
Pseudo-codice:

```text
lSplit = computeSplit(activity, left)
rSplit = computeSplit(activity, right)

if note >= splitMid:
    splitMid = rSplit
else if note <= splitMid:
    splitMid = lSplit

if not reset:
    krange = Range
    if rSplit - lSplit >= 0:
        splitMid = midpoint(lSplit, rSplit)
    else if rSplit-lSplit > -Range and recent activity brackets the note:
        splitMid = note + 1
        doubleNote = true

if leftD has notes:
    splitMid = max(max(leftD), splitMid)
if rightD has notes:
    splitMid = min(min(rightD), splitMid)

clamp splitMid to [Min.Range, Max.Range]
```

Nel caso `note >= splitMid`, `rSplit` aggiorna il candidato; nel caso `note <= splitMid`, `lSplit` aggiorna il candidato.

## Range
`Range` è registrato da 0 a 24 ed è incorporato nell'offset dei candidati della divisione e nella soglia del ramo duplicazione/bracket.

Non è un limite assoluto minimo/massimo diretto per entrambe le mani. L'unità effettiva corrisponde alle unità del controller, quindi semitoni/note.

## Min/Max
I limiti dei parametri sono indipendenti e compresi tra 0 e 127.

Il codice applica concettualmente prima il limite Min e poi il limite Max. Con Min > Max, l'ordine dei rami può lasciare `splitMid == Min`, violando il limite superiore Max. Questa configurazione è attualmente possibile tramite i parametri numerici.

## Init.Split
`kSplit` è inizializzato a 63 (`127/2`) e registrato da 0 a 127.

L'assegnazione del reset è `splitMid = kSplit`. La UI viene aggiornata con `SendControlValueFromDelegate`.

Cambiare Init.Split dalla UI imposta `reset=false`, ma non inizializza direttamente `splitMid` finché in `ProcessBlock` non si verifica la condizione di reset.

## Reset
A ogni Note Off, `delayTime = mCurrentSample + Reset_ms * sampleRate`.

In `ProcessBlock`, se `reset` è falso, `noteOn.size()==0` e `delayTime <= blockEnd`, la divisione torna a Init.Split.

Il reset è quindi basato sull'attività nella timeline dei campioni attivi, non sull'orologio reale. È a singolo passo e non viene animato progressivamente.

## Priorità
Il parametro dei pulsanti `kButtonGroup` usa valori normalizzati:
- SX: 0.0
- Mix: 0.5
- DX: 1.0

Il valore intero interno è -1, 0 o 1:
- SX: solo sinistra
- Mix: entrambe le mani
- DX: solo destra

Nell'assegnazione delle mani il ramo `splitType` è coerente con una sola modalità corrente. In Mix una nota può essere inviata a entrambe le mani; alla Note Off successiva il codice invia Off a entrambe e rimuove il valore da ciascun vettore.

## Limitazioni
- Il parametro di modalità non è dichiarato esclusivo tramite parametro radio oltre il comportamento della UI.
- L'algoritmo combina vincoli dinamici e forzati della divisione senza un modello di stato formale.
- Smooth a 0 può raggiungere un'espressione con divisione per zero in `ComputeSmoothingP`.
- Il ramo `if (rSplit-lSplit >=0)` impedisce al ramo successivo di essere negativo, quindi il ramo delle sovrapposizioni inferiori non è raggiungibile quando la prima condizione è vera, pur potendo essere rilevante in caso contrario.
- Il ramo `doubleNote` invia una nota a entrambe le mani.
