# AGENT.md

## Sintesi del progetto
IntelliSplit è un processore MIDI scritto in C++ con iPlug2 che assegna dinamicamente le note in ingresso alla mano sinistra o destra e le instrada verso canali MIDI e transposizioni configurabili.

## Obiettivo
Preservare e migliorare la divisione dinamica mantenendo la compatibilità VST3 con Windows/macOS e AU con macOS. Non considerare completa una funzionalità finché non viene testata in un host reale o mediante una build verificata.

## Architettura
- La classe centrale `IntelliSplit` possiede parametri, callback UI, stato della divisione, DSP e routing MIDI.
- `IBarGraphControl` visualizza i valori di attività delle 128 note e la posizione della divisione.
- `SmallSet` tiene traccia dei set di Note On/Off.
- `ThreadSafeVector` tiene traccia delle assegnazioni attive alle mani.
- `Utils` implementa lo smoothing.
- iPlug2 fornisce l'integrazione del plug-in e le infrastrutture per preset e parametri.

## Tecnologie
- C++
- iPlug2
- iGraphics
- VST3
- AU/AUv2/AUv3
- MSVC v143/MSBuild su Windows
- Xcode/Clang su macOS

## iPlug2
Il repository non include iPlug2. Occorre clonarlo come `../../iPlug2` o configurarlo tramite le proprietà del progetto.

I workflow di build fissano la revisione iPlug2 `61c042e936d832b6c7d93c5dd8b7e09cf2518bb9`.

## Struttura del repository
- `IntelliSplit.cpp`
- `IntelliSplit.h`
- `IBarGraphControl.h`
- `SmallSet.h`
- `ThreadSafeVector.h`
- `Utils.h`
- `config.h`
- `projects/`
- `config/`
- `scripts/`
- `.github/workflows/`
- `resources/`
- `docs/`

## Algoritmo
Ogni numero di nota MIDI possiede un valore di attività. La Note On lo imposta a 1. Lo stato tenuto/rilasciato viene aggiunto a `noteOff`, dopodiché `EvolveKeyboard` attenua il valore in base a Smooth. `computeSplit` genera i candidati sinistro e destro partendo dall'attività ponderata della tastiera. `ProcessingSplit` combina candidati, Range, stato delle ripetizioni e limiti Min/Max.

Il reset avviene dopo l'assenza di note attive e trascorso il tempo configurato in Reset, ripristinando `splitMid` al valore Init.Split.

Modalità:
- SX: solo sinistra
- Mix: entrambe
- DX: solo destra

## Parametri
- Smooth: 0-10000 ms
- Range: 0-24
- Min/Max: 0-127
- Init.Split: 0-127
- Reset: 0-10000 ms
- Canali sinistro/destro: 1-16
- Trasposizioni sinistra/destra: intervallo iPlug2 -128..127; la UI usa attualmente box orientati ai soli valori positivi
- Modalità canale: SX/Mix/DX

## Flusso MIDI
`ProcessMidiMsg` aggiorna i set delle note, modifica l'attività della tastiera, sceglie una mano, applica la transposizione se necessario e invia il MIDI in uscita.

I rischi noti includono la Note On con velocity pari a zero, le note duplicate, il routing che può cambiare alla Note Off, la mancanza di transposizione nella poly pressure e le transposizioni fuori intervallo.

## Formati del plug-in
- VST3 per Windows
- VST3 per macOS
- AU per macOS
- Sono presenti anche target iOS/AUv3 e standalone

L'analisi statica non garantisce alcuna build piattaforma.

## Windows
Usare Visual Studio/MSBuild v143. Il progetto VST3 è `projects/IntelliSplit-vst3.vcxproj`.

Percorso degli artefatti Debug/Release x64:
`build-win\vst3\x64\<Configuration>\`

Schema dei comandi GitHub Actions:
`msbuild <project> /m /t:Build /p:SolutionDir="<repo>/" /p:Configuration=Release /p:Platform=x64`

## Cantabile 4
Il test futuro deve usare Debug|x64, collegarsi o caricare il VST3, instradare l'ingresso verso due destinazioni MIDI, verificare canali e transposizioni, testare rilascio e blocco delle note e salvare/ripristinare lo stato del plug-in.

## macOS
I workflow supportano build universali VST3 e AU con `arm64 x86_64`. I file e i workflow macOS sono presenti, ma non è stata eseguita alcuna prova su Mac. Non dichiarare riuscito macOS senza una build Xcode reale e un test di caricamento nell'host.

## Problemi noti
Consultare `docs/CODE_REVIEW.md`, `docs/MIDI_ROUTING.md`, `docs/ALGORITHM.md` e `docs/MACOS_COMPATIBILITY.md`.

Rischi ad alta priorità:
- velocity 0 non è una Note Off;
- i conteggi delle note duplicate vengono persi;
- il routing della Note Off non viene conservato per singolo evento On;
- la UI e il routing delle transposizioni non coincidono;
- Smooth=0 può causare una divisione per zero;
- Min > Max è possibile.

## Documentazione
- `docs/PROJECT_OVERVIEW.md`
- `docs/ARCHITECTURE.md`
- `docs/ALGORITHM.md`
- `docs/ALGORITHM_EXAMPLES.md`
- `docs/MIDI_ROUTING.md`
- `docs/UI_PARAMETERS.md`
- `docs/BUILD_AND_TEST.md`
- `docs/MACOS_COMPATIBILITY.md`
- `docs/CODE_REVIEW.md`
- `docs/ROADMAP.md`
- `docs/DECISIONS.md`

## Regole per i futuri agenti AI
- Leggere integralmente questo file prima di iniziare.
- Consultare i documenti pertinenti in `docs/`.
- Analizzare il codice esistente prima di modificarlo.
- Non riscrivere componenti funzionanti senza una motivazione concreta.
- Non introdurre modifiche che compromettano la compatibilità Windows/macOS.
- Aggiornare sempre la documentazione quando cambia il comportamento del software.
- Aggiornare `CHANGELOG.md` se viene creato o già presente.
- Aggiornare `docs/ROADMAP.md` quando un intervento cambia lo stato delle funzionalità.
- Registrare le decisioni architetturali importanti in `docs/DECISIONS.md`.
- Non indicare una funzionalità come completa senza verifica.
- Non dichiarare riuscita una build finché non è stata realmente eseguita.
- Distinguere chiaramente fatti confermati, ipotesi e lavoro non verificato.
- Dare priorità alla validazione Windows/Cantabile 4 rispetto a macOS.
- Non dichiarare verificato macOS senza un Mac reale o l'esecuzione registrata della CI macOS.
- Conservare il contesto delle attività precedenti e fare riferimento a ID stabili di issue.
