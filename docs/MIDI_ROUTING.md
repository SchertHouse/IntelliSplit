# Routing MIDI

## Ingresso
`ProcessMidiMsg` gestisce Note On, Note Off, Poly Aftertouch, Control Change, Pitch Wheel, Channel Aftertouch e Program Change.

## Note On
Alla Note On il codice:
1. aggiunge la nota a `noteOn`;
2. memorizza `channelNote[inputChannel] = note`;
3. rimuove la nota da `noteOff`;
4. evolve l'attività della tastiera;
5. determina la mano e la invia a uno o entrambi i canali configurati.

Non esiste una mappa dell'identità delle note o dei conteggi delle duplicazioni.

## Note Off
Il codice invia una Note Off se `left.contains(note)` o `right.contains(note)`. Rimuove i valori corrispondenti dai vettori, aggiunge la nota a `noteOff` e la rimuove da `noteOn`.

Il routing tenta di preservare la mano originale, ma non conserva il canale o la transposizione per singolo evento. Entrambi possono cambiare prima della Note Off.

## Bug e rischi critici

**BUG-001: Note On con velocity zero**
La velocity zero non viene interpretata come Note Off. Viene elaborata come Note On con velocity effettiva pari a zero, lasciando potenzialmente incoerenti `noteOn` e i vettori delle mani.

**BUG-002: Note duplicate**
`left` e `right` memorizzano soltanto il numero della nota. Due Note On sovrapposte della stessa nota collassano semanticamente; la prima Note Off rimuove il valore per tutte le occorrenze duplicate. Le ritrasmissioni possono essere perse o interrotte anticipatamente.

**BUG-003: Routing della Off modificabile**
Canale e transposizione della Off sono ricalcolati dai parametri correnti, non memorizzati nell'evento On corrispondente. Cambiare canale o transposizione può quindi far usare alla Note Off un routing diverso dalla Note On.

**BUG-004: Nota trasposta fuori intervallo**
`computeTrans` restituisce `-1` per un risultato fuori da 0-127. Il chiamante non rifiuta sempre `-1` prima di costruire il messaggio MIDI.

**BUG-005: Poly Aftertouch senza transposizione**
Poly Aftertouch viene instradato in base alla mano, ma usa il numero della nota originale senza applicare la transposizione configurata.

**BUG-006: Mappatura del Pitch Wheel per canale**
Il canale di ingresso 0 indirizza a entrambe le uscite. Gli altri canali usano `channelNote[channel]`, ma l'array non viene inizializzato e il valore non viene cancellato alla Note Off.

**BUG-007: Gestione della Off in modalità Mix**
In Mix una Note On viene inviata a entrambe le mani e la nota viene memorizzata in entrambi i vettori. Una Note Off successiva invia Off ad entrambe e rimuove entrambi i valori.

## Control Change
Control Change viene inviato a entrambi i canali in uscita. Un unico flag globale `fromUI` intercetta gli eventi All Notes Off provenienti dai gestori UI di canale/transposizione e inoltra l'evento con il canale di ingresso, anziché usare il normale fan-out.

## Poly Aftertouch
Il fan-out segue l'appartenenza alle mani attive. Se la nota è presente in entrambe, viene inviata a entrambe. Per una nota già trasposta nel routing, l'evento di pressione non viene trasposto.

## Pitch Wheel
Il canale di ingresso 0 invia il bend a entrambe le uscite. Gli altri canali usano l'ultima nota registrata per il canale di ingresso per scegliere la mano in uscita. Non usano la nota attualmente tenuta nello stato delle mani con identità dell'evento.

## Channel Aftertouch
Viene inviato a entrambi i canali in uscita e non è specifico per il canale di ingresso.

## Program Change
Viene inviato a entrambi i canali in uscita.

## Altri messaggi
I messaggi MIDI System Common/System Real-Time, dati MPE oltre i casi supportati, SysEx e MTC seguono il ramo predefinito e non vengono inoltrati.

Il plug-in dichiara `PLUG_DOES_MPE 1` in `config.h`, ma `ProcessMidiMsg` non implementa una logica di routing MPE specifica.
