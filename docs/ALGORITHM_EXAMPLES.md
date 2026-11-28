# Esempi dell'algoritmo

Queste sono descrizioni teoriche di sequenze destinate a test futuri. I risultati contrassegnati come "non verificato" non sono stati osservati in un host.

## Passaggio ascendente
Esempio: Note On 60, 64, 67, 72, 76, tenute o rilasciate a seconda del test.

Obiettivo atteso: l'attività dovrebbe spostare o enfatizzare un candidato della mano destra mentre le note salgono.
Risultato effettivo: non verificato. Il codice usa attività relativa e candidati dinamici anziché una soglia fissa.

## Passaggio discendente
Esempio: 76, 72, 67, 64, 60.

Obiettivo atteso: l'attività dovrebbe enfatizzare un candidato della mano sinistra.
Risultato effettivo: non verificato.

## Accordo
Esempio: simultanee 60, 64, 67.

Comportamento previsto: le Note On ripetute impostano tutte le note al massimo; le scansioni sinistra/destra si fermano sulle note massima/minima e Range modifica la distanza dei candidati. In modalità Mix lo stesso accordo viene inviato a entrambi i canali.

## Registri alternati
Esempio: 60, 72, 62, 74, 64, 76.

Questa sequenza deve sottoporre a stress la stabilità della divisione e l'incrocio dei candidati sinistro/destro. Il codice può forzare il comportamento `doubleNote` nel ramo con gap negativo dei candidati. Risultato effettivo: non verificato.

## Prevalenza della mano sinistra
Esempio: sequenza tenuta 48-60 con occasionali 64.

Test futuro atteso: la divisione dovrebbe tendere a sinistra rispetto all'attività del lato destro. Confrontare Smooth basso/alto.
Risultato effettivo: non verificato.

## Prevalenza della mano destra
Esempio: sequenza 67-79 con occasionali 60.

Test futuro atteso: la divisione dovrebbe tendere a destra rispetto all'attività del lato sinistro.
Risultato effettivo: non verificato.

## Entrambe le mani
Esempio: note basse 48-55 e alte 72-79 alternate.

Il test futuro deve verificare se il punto medio `lSplit/rSplit` e i vincoli delle duplicazioni restano stabili. Risultato effettivo: non verificato.

## Effetti dei parametri
- Smooth basso: il decadimento raggiunge la soglia più rapidamente, ma Smooth=0 è attualmente un caso limite che richiede test e correzione.
- Smooth alto: i valori persistono e possono mantenere l'influenza più a lungo.
- Range 0/24: modifica le distanze dei candidati e la gestione delle sovrapposizioni duplicate.
- Min/Max: limita la divisione finale se configurato correttamente.
- Reset 0: dovrebbe ripristinare immediatamente la divisione una volta che nessuna nota è attiva.
- Reset 10000: mantiene la divisione dinamica più a lungo dopo l'ultima Note Off.
- SX: dovrebbe instradare una nota esclusivamente sul canale sinistro.
- Mix: dovrebbe instradare una nota su entrambi i canali.
- DX: dovrebbe instradare una nota esclusivamente sul canale destro.

## Sequenza di reset
1. Riprodurre una sequenza che modifica `splitMid`.
2. Rilasciare tutte le note.
3. Attendere il numero di millisecondi definito da Reset.
4. In un successivo ProcessBlock senza note attive, verificare che il marcatore della divisione sia pari a Init.Split.

Risultato effettivo: non verificato.

## Test di retrigger
Inviare più Note On dello stesso numero di nota prima del corrispondente numero di Note Off.

I vettori attuali memorizzano i valori delle note, non eventi indipendenti. Questo è un caso di test futuro per note bloccate o rimosse prematuramente.
