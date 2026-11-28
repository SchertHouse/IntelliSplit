# Compilazione e test

## Prerequisiti
- Checkout iPlug2 in `../../iPlug2` rispetto al repository, oppure `IPLUG2_ROOT` configurato correttamente.
- Dipendenze e SDK iPlug2.
- Windows: Visual Studio con MSVC v143 e Windows SDK 10.
- macOS: Xcode e runner macOS 15 per i workflow del repository.
- Python 3.12 per la configurazione dei workflow GitHub.

## Revisione iPlug2
Tutti e tre i workflow verificano la revisione iPlug2 `61c042e936d832b6c7d93c5dd8b7e09cf2518bb9` prima della configurazione della build.

## Windows
La soluzione è `IntelliSplit.sln`. Il progetto VST3 è `projects/IntelliSplit-vst3.vcxproj`.

Supporta configurazioni Debug, Release e Tracer per Win32 e x64. L'estensione del target è `.vst3`.

Comando MSBuild rappresentativo del workflow:

```powershell
msbuild "<IntelliSplit-vst3.vcxproj>" /m /t:Build /p:SolutionDir="<repository>/" /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=10.0 /verbosity:minimal
```

Il progetto Visual Studio scrive gli artefatti x64 in `build-win\vst3\x64\<Configuration>\`.

La proprietà post-build richiama `scripts\postbuild-win.bat` e può creare un bundle VST3 e copiarlo nei percorsi standard dei plug-in VST3.

Non viene dichiarato alcun percorso esatto locale per il plug-in Cantabile 4. L'utente deve determinare e verificare il percorso di scansione dell'host.

## Debug
Compilare Debug|x64 e collegare Visual Studio a Cantabile 4 oppure avviare Cantabile 4 sotto il debugger, se supportato. Impostare il percorso del plug-in VST3 sul bundle compilato localmente.

L'output PDB è configurato in `config\IntelliSplit-win.props` come `build-win\pdbs\IntelliSplit_x64.pdb`.

## Procedura di test Windows (futura)
1. Compilare Debug|x64.
2. Installare o esporre il bundle VST3 a Cantabile 4.
3. Caricare IntelliSplit e confermare che ingresso e uscita MIDI siano abilitati.
4. Inoltrare una tastiera o sorgente MIDI al plug-in.
5. Configurare due destinazioni e selezionarle per i canali sinistro/destro.
6. Testare canali, trasposizione, tutte le modalità di priorità, reset e automazione dei parametri.
7. Tenere e rilasciare note duplicate e testare il retrigger.
8. Cambiare canali/trasposizione mentre le note sono tenute.
9. Verificare il comportamento di All Notes Off.
10. Salvare e ripristinare rack/preset di Cantabile e stato del plug-in.

Tale test non è stato eseguito in questa fase di analisi.

## macOS
Il workflow macOS:
- controlla iPlug2;
- scarica SDK e dipendenze precompilate iPlug2;
- compila `projects/IntelliSplit-macOS.xcodeproj` con `-target VST3` o `-target AU`;
- richiede `arm64 x86_64`;
- disabilita il code signing.

Il workflow `IntelliSplit-macOS-AU.yml` attende il bundle AU in `$HOME/Library/Audio/Plug-Ins/Components/IntelliSplit.component`, quindi lo pacchettizza e lo carica come artefatto.

Nessuna build macOS è stata eseguita localmente. La disponibilità di macOS nelle GitHub Actions non costituisce prova di un test locale interattivo riuscito.

## Comandi di build macOS
VST3:

```bash
xcodebuild -project projects/IntelliSplit-macOS.xcodeproj -target VST3 -configuration Release -sdk macosx ARCHS="arm64 x86_64" ONLY_ACTIVE_ARCH=NO build
```

AU:

```bash
xcodebuild -project projects/IntelliSplit-macOS.xcodeproj -target AU -configuration Release -sdk macosx ARCHS="arm64 x86_64" ONLY_ACTIVE_ARCH=NO build
```

## Test
Non è presente alcun progetto unit test dedicato o runner automatizzato nell'inventario del repository. Il comportamento del motore deve essere estratto dietro interfacce indipendenti da iPlug2 prima di poter eseguire deterministicamente una suite di unit test.
