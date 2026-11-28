# UI e parametri

| Parametro | UI | Intervallo/predefinito iPlug2 | Uso |
|---|---|---|---|
| kParamPSmooth | Manopola Smooth | 0-10000 ms, 500 ms | Tempo di decadimento per singola nota |
| kParamRange | Manopola Range | 0-24, 12 | Intervallo del candidato della divisione |
| kOutputMin | Casella Min | 0-127, 24 | Limite minimo della divisione |
| kOutputMax | Casella Max | 0-127, 108 | Limite massimo della divisione |
| kSplit | Casella Init.Split | 0-127, 63 | Divisione iniziale e post-reset |
| kTimeReset | Manopola Reset | 0-10000 ms, 3000 ms | Ritardo di inattività prima del reset |
| kOutputChannelSx | Casella Canale sinistro | 1-16, 1 | Canale MIDI sinistro |
| kOutputChannelDx | Casella Canale destro | 1-16, 2 | Canale MIDI destro |
| kOutputTrasSx | Casella Trasposizione sinistra | iPlug2 -128..127, 0 internamente | Trasposizione della mano sinistra |
| kOutputTrasDx | Casella Trasposizione destra | iPlug2 -128..127, 0 internamente | Trasposizione della mano destra |
| kButtonGroup | Pulsanti SX/Mix/DX | interno -1..1, normalizzato 0..1 | Priorità del routing |

## Smooth
Registrato in millisecondi ed controllato visivamente da `IVKnobControl`.

## Range
Registrato con un limite superiore di 24 unità di semitoni e utilizzato nei calcoli dei candidati e delle sovrapposizioni.

## Min/Max
Sono due caselle indipendenti. Nessuna validazione impedisce visivamente `Min > Max`.

## Init.Split
Collegato direttamente a `kSplit`; l'azione UI imposta `reset=false`.

## Reset
Collegato direttamente a `kTimeReset`.

## Canali in uscita
I parametri dei canali contengono valori da 1 a 16. Il DSP li converte in 0-15 subito prima di costruire i messaggi MIDI iPlug2.

I gestori UI inviano All Notes Off sul vecchio canale e aggiornano `oldCh1`/`oldCh2`.

## Trasposizione
L'intervallo del parametro iPlug2 è -128..127, ma le caselle UI sono attualmente configurate per valori soltanto positivi:
- Sinistra: predefinito 24, minimo 1, massimo 128
- Destra: predefinito 108, minimo 1, massimo 128

Questa incongruenza impedisce alla UI di esporre correttamente l'intervallo negativo e nullo e non mostra chiaramente il significato MIDI in semitoni da -128 a +127.

## Modalità dei pulsanti
`SX`, `Mix` e `DX` sono pulsanti ordinari. Le relative azioni aggiornano `kButtonGroup` e impostano esplicitamente i valori visivi degli altri pulsanti.

Il parametro `kButtonGroup` ha intervallo intero -1..1 e viene normalizzato a 0..1 per la UI. Le costanti normalizzate sono 0.0, 0.5 e 1.0.

## Sincronizzazione
I parametri sono posseduti da iPlug2 e possono essere automatizzati. I controlli UI chiamano `SetValueFromDelegate` durante l'inizializzazione del layout.

Il marcatore del grafico viene aggiornato con `SendControlValueFromDelegate(kBarGraphControl, splitMid)`.

## Persistenza
Il repository abilita l'infrastruttura iPlug2 degli stati con `PLUG_DOES_STATE_CHUNKS 0`. Preset e persistenza dei parametri dipendono quindi dalla serializzazione e dall'automazione dei parametri iPlug2 anziché da codice personalizzato di stato.
