# Revisione del codice

## Aspetti positivi
- Il plug-in registra i parametri iPlug2 in modo coerente.
- Il ciclo principale è relativamente compatto e opera su 128 posizioni di note.
- Il grafico a barre e l'array delle attività offrono feedback visivo.
- I contenitori personalizzati delle note attive usano protezione tramite mutex.
- I workflow di build fissano iPlug2 e dimostrano target Windows, VST3 macOS e AU macOS.
- Il progetto dichiara configurazioni Debug/Release/Tracer su Windows.

## Problemi confermati

### MIDI
- La velocity 0 non viene interpretata come Note Off.
- Il tracciamento delle note duplicate per numero non preserva retrigger/sovrapposizioni.
- Canale e transposizione della Note Off vengono ricalcolati anziché memorizzati.
- Poly Aftertouch non viene trasposto.
- `channelNote` non viene inizializzato.
- `computeTrans` può restituire `-1` e il valore non viene sempre rifiutato.
- Le variazioni di Program/Channel Aftertouch e Control Change vengono duplicate su entrambi i canali senza semantica documentata per mano.

### Algoritmo
- Min/Max possono essere configurati con valori non validi.
- Smooth=0 può causare una divisione per zero in `ComputeSmoothingP`.
- Il reset si basa sul conteggio dei campioni ed è a singolo passo.
- Il ramo delle sovrapposizioni dopo il punto medio del candidato ha raggiungibilità limitata.
- Il routing Mix e lo stato delle duplicazioni sinistra/destra usano vettori basati solo sul valore.

### UI
- Gli intervalli UI delle transposizioni sono in conflitto con quelli iPlug2 e non esprimono chiaramente i semitoni negativi.
- I cambi di canale emettono All Notes Off tramite lo stato UI, ma il flag globale `fromUI` è sensibile alla tempistica.
- Nessuna validazione UI impedisce Min > Max.

### Tempo reale
- Il worker thread della UI può essere attivo mentre lo stato DSP è in uso.
- `Evolve` acquisisce `mMutex` e i lock interni di SmallSet/vettori in percorsi annidati.
- Nei callback MIDI sono presenti allocazioni dinamiche e operazioni sui vettori.
- `SmallSet::contains`, pur leggendo il bitset, non è bloccato; l'accesso concorrente UI/DSP non è completamente sincronizzato.
- `IBarGraphControl` conserva un riferimento a `keyboard`; l'accesso UI/DSP non è sincronizzato esplicitamente.

## Debito tecnico
- Il motore della divisione è inseparabile da iPlug2.
- Una sola classe centrale possiede UI, DSP, routing e stato dell'algoritmo.
- I contenitori personalizzati espongono API bloccate e non bloccate.
- La modalità viene rappresentata in modo incoerente dalle costanti UI normalizzate e dai valori interi DSP.
- Non esiste alcun progetto di unit test.
- Macro di output Debug e worker `std::thread` della UI sono incorporati nel codice del plug-in.

## Valutazioni qualitative
- Architettura: **Da migliorare** - responsabilità concentrate e accoppiamento iPlug2.
- Qualità C++: **Sufficiente** - stato chiaro ma con proprietà mista e problemi di const/thread safety.
- Manutenibilità: **Da migliorare** - implementazione centralizzata di oltre 500 righe e contenitori personalizzati.
- Separazione delle responsabilità: **Da migliorare**.
- Correttezza MIDI: **Critica** - velocity 0, duplicazioni e identità del routing Off restano irrisolti.
- Affidabilità dell'algoritmo: **Da migliorare** - nessun invariante formale o verifica dal vivo.
- Prestazioni in tempo reale: **Sufficiente** - lavoro limitato sulla tastiera, ma mutex, allocazioni e thread UI richiedono revisione.
- Gestione dei parametri: **Sufficiente** - la registrazione iPlug2 è presente, ma restano intervalli non validi e discrepanza delle transposizioni.
- UI: **Buona** - controlli e grafico sono implementati.
- Compatibilità Windows: **Buona** - evidenze da progetto/workflow, ma build locale non verificata in questa sessione.
- Compatibilità macOS: **Non verificabile** - nessun Mac/build eseguito qui.
- Compatibilità VST3: **Non verificabile** - lo stato richiede esecuzione in un host.
- Compatibilità AU: **Non verificabile**.
- Verificabilità: **Critica** - nessun motore isolato né test.
- Documentazione: **Sufficiente** - il README è minimo; questa analisi fornisce ora documentazione tecnica.
