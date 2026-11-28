# Roadmap

## Stato attuale
- Implementato: shell iPlug2 del plug-in, GUI, euristica della divisione dinamica, routing sinistra/destra, transposizione, canali, reset e grafico dell'attività.
- Documentato: architettura statica e rischi tecnici.
- Non verificato: comportamento reale nell'host, build Debug Windows, routing in Cantabile 4, build macOS e caricamento AU.

## Priorità 1: Correttezza
1. Interpretare le Note On con velocity zero come Note Off.
2. Introdurre un'identità per nota o un contatore esplicito delle duplicazioni.
3. Conservare il canale in uscita originale e la transposizione per ogni evento di nota attiva.
4. Rifiutare le note MIDI trasposte fuori dall'intervallo 0-127.
5. Aggiungere uno stato esplicito di routing `sinistra/destra` che conservi i conteggi delle duplicazioni.
6. Aggiungere la validazione Min <= Max.
7. Proteggere il caso Smooth=0.

## Priorità 2: Verificabilità
1. Estrarre il calcolo della divisione e la gestione dello stato MIDI da iPlug2 in un motore indipendente dalla piattaforma.
2. Aggiungere test unitari deterministici per formule della divisione e ciclo di vita delle note.
3. Aggiungere test di integrazione dell'host o sequenze MIDI controllate.
4. Aggiungere casi di test per note bloccate e cambi di canale.

## Priorità 3: Tempo reale e concorrenza
1. Rimuovere mutex non necessari dal codice già monothread.
2. Sostituire i dati condivisi bloccati tra UI e DSP con snapshot espliciti compatibili con iPlug2/UI.
3. Rimuovere, ove possibile, allocazioni dinamiche dai percorsi critici di elaborazione delle note.
4. Sostituire o limitare il worker thread della UI.
5. Definire la proprietà DSP/UI di `keyboard`, vettori e stato della divisione.

## Priorità 4: Algoritmo
1. Definire e testare gli invarianti di Min/Max/Range.
2. Testare e calibrare i rami dei candidati della divisione.
3. Rendere indipendente il comportamento delle sovrapposizioni e delle duplicazioni dai valori `leftD/rightD`.
4. Rendere il comportamento del reset deterministico e verificabile.
5. Valutare uno smoothing di `splitMid` invece di assegnazioni forzate e improvvise.

## Priorità 5: UI
1. Allineare intervalli ed etichette dei parametri di trasposizione.
2. Aggiungere un'indicazione per Min/Max non valido.
3. Chiarire le unità di Reset e le modalità di priorità.
4. Migliorare la visualizzazione della divisione e dello stato delle mani.

## Priorità 6: macOS
1. Compilare VST3 e AU su un Mac reale.
2. Verificare gli output universali arm64/x86_64.
3. Verificare gli host AU e AUv3.
4. Risolvere esclusivamente gli errori di build/link riproducibili.

## Attività rinviate
- Nessuna implementazione sorgente in questa fase.
- Nessuna build o test dell'host in questa fase.
- Il lavoro macOS resta bloccato finché non è disponibile un Mac reale o un ambiente equivalente.

## Mantenimento della documentazione
Ogni modifica al comportamento, alle API, ai parametri, al build o alla compatibilità deve aggiornare i documenti italiani pertinenti nella stessa modifica. Le funzionalità devono essere indicate come complete solo dopo una verifica reale o una suite automatizzata pertinente.
