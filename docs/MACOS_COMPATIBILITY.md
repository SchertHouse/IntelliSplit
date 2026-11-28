# Analisi della compatibilità macOS

Per questa analisi non era disponibile alcun Mac e nessuna build macOS viene dichiarata riuscita o testata.

## Problemi identificati direttamente

### MAC-001: Dipendenze layout e workflow iPlug2
- **File:** `.github/workflows/*.yml`, `config/IntelliSplit-mac.xcconfig`.
- **Descrizione:** le build macOS attendono iPlug2 in `../../iPlug2`; i workflow scaricano le dipendenze e fissano una revisione specifica.
- **Causa:** il repository da solo non rappresenta le dipendenze relative.
- **Probabilità:** confermata dalla configurazione.
- **Severità:** Media.
- **Suggerimento:** aggiungere una validazione del layout delle dipendenze e mantenere documentata la revisione fissata.
- **Verifica su Mac futura:** eseguire `xcodebuild -project ... -target VST3 -showBuildSettings` e verificare `SRCROOT`, `IPLUG2_ROOT`, header e librerie.

### MAC-002: Flag della build universale
- **File:** `config/IntelliSplit-mac.xcconfig`, workflow macOS.
- **Descrizione:** la configurazione macOS è a 64 bit; i workflow aggiungono `ARCHS="arm64 x86_64"`.
- **Causa:** le impostazioni di architettura del target Xcode e della riga di comando possono interagire.
- **Probabilità:** Potenziale.
- **Severità:** Media.
- **Suggerimento:** verificare in Xcode le impostazioni del prodotto e della deployment target.
- **Verifica futura:** usare `lipo -archs` e `arch` su ambienti di test arm64 e x86_64.

### MAC-003: Ricerca del pacchetto AU
- **File:** `.github/workflows/build-macos-AU.yml`.
- **Descrizione:** la build AU assume che un componente esista nella directory Components standard dell'utente.
- **Causa:** il comportamento della directory di lavoro e l'installazione del bundle AU possono variare.
- **Probabilità:** Potenziale.
- **Severità:** Media.
- **Suggerimento:** catturare i prodotti di build tramite `SYMROOT`/percorso artefatto anziché affidarsi solo alla directory standard della libreria.
- **Verifica futura:** eseguire il workflow e verificare che il componente esista e possa essere caricato.

## Problemi potenziali

### MAC-004: Codifica dei sorgenti
- **File:** `IntelliSplit.cpp`, `SmallSet.h`, commenti sorgente.
- **Descrizione:** i commenti sorgente contengono testo non ASCII. Alcuni file possono usare codifiche diverse o codifiche Windows.
- **Causa:** Clang normalmente accetta commenti UTF-8, ma una codifica diversa può causare problemi in configurazioni Xcode/locale insolite.
- **Probabilità:** Bassa.
- **Severità:** Bassa.
- **Suggerimento:** mantenere i sorgenti C++ in UTF-8 ed evitare caratteri accentati negli identificatori.
- **Verifica futura:** build pulita dei target macOS.

### MAC-005: Assunzioni solo Windows
- **File:** `IntelliSplit.cpp`, `IBarGraphControl.h`, `Utils.h`, script `.bat`.
- **Descrizione:** nei file C++ principali non è stata rilevata alcuna chiamata diretta all'API Windows. Gli script Windows sono separati.
- **Causa:** header e macro iPlug2 dovrebbero astrarre le differenze tra piattaforme.
- **Probabilità:** Bassa.
- **Severità:** Media.
- **Suggerimento:** preservare i condizionali iPlug2 ed evitare API Win32 nel core DSP/UI.
- **Verifica futura:** compilare i target VST3 e AU.

### MAC-006: Rendering OpenGL/Metal
- **File:** `config/IntelliSplit-mac.xcconfig`, risorse iGraphics.
- **Descrizione:** macOS usa IGRAPHICS_METAL, mentre Windows usa IGRAPHICS_NANOVG/GL2.
- **Causa:** iPlug2 gestisce normalmente le differenze dei backend, ma il codice personalizzato può usare API non completamente neutrali.
- **Probabilità:** Bassa.
- **Severità:** Media.
- **Suggerimento:** mantenere `IBarGraphControl` sulle primitive di disegno iPlug2.
- **Verifica futura:** aprire la UI su Mac Intel e Apple Silicon e confrontare il rendering.

### MAC-007: Dichiarazioni VST3 e AU
- **File:** `config.h`, progetto Xcode.
- **Descrizione:** `PLUG_TYPE=2`, `PLUG_DOES_MIDI_IN/OUT`, MPE e impostazioni UI devono essere accettati da entrambi i formati.
- **Causa:** i wrapper iPlug2 dei formati aggiungono comportamenti ABI specifici della piattaforma.
- **Probabilità:** Potenziale.
- **Severità:** Media.
- **Suggerimento:** verificare separatamente registrazione dell'host e routing MIDI per VST3 e AU.
- **Verifica futura:** caricare entrambi negli host macOS e controllare le capacità MIDI.

## Ipotesi da verificare
- Se Xcode esponga un target `AU` come assunto dal workflow.
- Se il packaging AUv2 abbia successo con il runner macOS-15.
- Se risorse ed entitlements AUv3 siano completi.
- Se il linking universale abbia successo per tutte le librerie iPlug2 richieste.
- Se build con code signing disabilitato siano la modalità corretta di packaging.
- Se il comportamento dei mutex personalizzati e del worker UI funzioni con le restrizioni dello scheduler macOS.

## Elementi dipendenti dall'ambiente
- Certificati di firma e notarizzazione non sono rappresentati.
- Il comportamento su host Apple Silicon non è testato.
- Il comportamento reale negli host AU, il supporto del routing MIDI e la persistenza dei parametri non possono essere inferiti staticamente.
