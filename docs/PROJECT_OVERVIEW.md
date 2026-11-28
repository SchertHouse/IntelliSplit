# Panoramica del progetto

## Scopo
IntelliSplit è un processore MIDI basato su iPlug2 che assegna dinamicamente le note MIDI in ingresso a una mano virtuale sinistra o destra e le instrada verso canali MIDI e transposizioni configurabili.

Il repository implementa una divisione dinamica anziché una divisione fissa. Mantiene un array da 128 elementi dell'attività della tastiera e ricava i candidati della divisione sinistra/destra dall'attività recente delle note. La qualità musicale dell'algoritmo non può essere confermata senza test dal vivo.

## Tecnologie
- C++ con iPlug2.
- iGraphics di iPlug2 per la GUI desktop.
- Target VST3 per Windows e macOS.
- Target AU per macOS.
- Visual Studio/MSBuild su Windows.
- Xcode/xcodebuild su macOS.
- Workflow GitHub Actions per le build Windows e macOS.

## iPlug2
Il repository non include iPlug2. La configurazione di build prevede una copia adiacente di `iPlug2` tramite `$(IPLUG2_ROOT)`/`../../iPlug2`.

I workflow di build Windows e macOS fissano la revisione iPlug2 `61c042e936d832b6c7d93c5dd8b1e09cf2518bb9`. Questa è la revisione del framework usata nelle build descritte dal repository; il sorgente iPlug2 installato è esterno al repository.

## Formati
I target presenti nel repository includono:
- VST3 per Windows.
- VST3 per macOS.
- Target progetto AUv2/AU v2 e AUv3 per macOS.
- Target iOS AUv3 e standalone, sebbene i formati principali richiesti siano AU macOS e VST3 macOS/Windows.

AUv3 non è lo stesso target di AUv2; lo stato di ogni build deve essere verificato separatamente.

## Struttura delle cartelle
- `IntelliSplit.cpp`: costruttore del plug-in, disposizione UI, callback DSP, routing MIDI e algoritmo della divisione.
- `IntelliSplit.h`: stato del plug-in, ID dei parametri e decadimento integrato della tastiera.
- `IBarGraphControl.h`: controllo personalizzato per visualizzare attività e divisione.
- `SmallSet.h`: set basato su bitset per tenere traccia delle Note On/Off.
- `ThreadSafeVector.h`: archivio protetto da mutex delle note assegnate alle mani.
- `Utils.h`: funzioni di smoothing e tempo.
- `config.h`: metadati iPlug2 del plug-in e costanti di rendering/UI.
- `projects/`: progetti Visual Studio e Xcode.
- `config/`: proprietà di build Windows e macOS.
- `scripts/`: script Windows per bundle e post-build.
- `.github/workflows/`: automazione delle build Windows e macOS.
- `resources/`: icone, font, xib/storyboard e metadati della piattaforma.
- `docs/`: documentazione tecnica e documenti di analisi.

## Componenti principali
- `IntelliSplit`: proprietario iPlug2 di UI, parametri, DSP e MIDI.
- `SmallSet<uint8_t,0,128>`: tiene traccia dei set di Note On/Off.
- `ThreadSafeVector<uint8_t>`: tiene traccia delle note assegnate a sinistra/destra e dello stato delle duplicazioni.
- `IBarGraphControl<N_KEY>`: mostra l'attività della tastiera e il marcatore della divisione.
- `Utils`: calcola un decadimento di tipo esponenziale.

## Stato del repository
L'analisi statica è stata completata sui file disponibili. Il worktree era pulito prima dell'aggiunta della documentazione. Nessun file sorgente, build o risorsa è stato modificato in questa fase di analisi. Nessuna build Windows o macOS viene dichiarata verificata.
